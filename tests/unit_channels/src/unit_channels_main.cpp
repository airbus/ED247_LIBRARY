/******************************************************************************
 * The MIT Licence
 *
 * Copyright (c) 2021 Airbus Operations S.A.S
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *****************************************************************************/
// Set ED247_FRIEND_TEST to have access to class private members
#define TEST_CLASS_NAME(test_case_name, test_name) test_case_name##_##test_name##_Test
class TEST_CLASS_NAME(ChannelContext, MultiPushPop);
#define ED247_FRIEND_TEST() friend TEST_CLASS_NAME(ChannelContext, MultiPushPop);

#include "unitary_test.h"
#include "ed247_context.h"

std::string config_path = "../config";

class ChannelContext : public ::testing::TestWithParam<std::string>
{
    protected:

        ChannelContext()
        {
            SAY("ChannelContext::Ctor");
        }

        ~ChannelContext() override
        {
            SAY("ChannelContext::Dtor");
        }

        // Per-test-suite set-up
        // Called before the first test in this test suite
        // Can be omitted if not needed
        static void SetUpTestSuite()
        {
            SAY("ChannelContext::SetUpTestSuite");
        }

        // Per-test-suite tear-down
        // Called after the last test in this test suite
        // Can be omitted if not needed
        static void TearDownTestSuite()
        {
            SAY("ChannelContext::TearDownTestSuite");
        }

        // Per-test set-up logic
        virtual void SetUp()
        {
            SAY("ChannelContext::SetUp");
        }

        // Per-test tear-down logic
        virtual void TearDown()
        {
            SAY("ChannelContext::TearDown");
        }
};

TEST_P(ChannelContext, MultiPushPop)
{
    try{
        RecordProperty("description", strize() << "Load content of [" << GetParam() << "]");

        std::string filepath = GetParam();
        ed247::Context* context = ed247::Context::create_from_filepath(filepath);

        // Retrieve the set of channels
        auto channel_set = context->get_channel_set();
        ASSERT_EQ(channel_set.size(), (uint32_t)2);

        // Check finder for find all
        auto channels = channel_set.find(".*");
        ASSERT_EQ(channels.size(), (uint32_t)2);

        // Check finder for a single channel
        auto channels0 = channel_set.find("Channel0");
        ASSERT_EQ(channels0.size(), (uint32_t)1);
        auto channel0 = channels0[0];
        auto channels1 = channel_set.find("Channel1");
        ASSERT_EQ(channels1.size(), (uint32_t)1);
        auto channel1 = channels1[0];

        // Push stream samples
        for(auto stream : channel0->streams()){
            ed247::StreamSample sample(stream.second->get_sample_max_size_bytes());
            bool full;
            for(uint32_t i = 0 ; i < stream.second->get_sample_max_number() ; i++){
                std::string sample_str = strize() << std::setw(stream.second->get_sample_max_size_bytes()) << std::setfill('0') << i;
                sample.copy(sample_str.c_str(),stream.second->get_sample_max_size_bytes());
                malloc_count_start();
                ASSERT_EQ(stream.second->push_sample(sample.data(), sample.size(), NULL, &full), true);
                ASSERT_EQ(malloc_count_stop(), 0);
                if(i < (stream.second->get_sample_max_number()-1))
                    ASSERT_FALSE(full);
                else
                    ASSERT_TRUE(full);
            }
        }

        // Encode channel & check frame
        malloc_count_start();
        channel0->encode_and_send();
        ASSERT_EQ(malloc_count_stop(), 0);
        uint32_t frame_index = 0;
        if(channel0->_configuration->_header._enable == ED247_YESNO_YES){
            uint16_t pid = ntohs(*(uint16_t*)((char*)channel0->_buffer.data()+frame_index));
            frame_index += sizeof(uint16_t);
            ASSERT_EQ(pid, 0);
            uint16_t sn = ntohs(*(uint16_t*)((char*)channel0->_buffer.data()+frame_index));
            frame_index += sizeof(uint16_t);
            ASSERT_EQ(sn, channel0->_header.get_next_serial_number()-1);
            if(channel0->_configuration->_header._transport_timestamp == ED247_YESNO_YES){
                ed247_timestamp_t timestamp;
                timestamp.epoch_s = ntohl(*(uint32_t*)((char*)channel0->_buffer.data()+frame_index));
                frame_index += sizeof(uint32_t);
                timestamp.offset_ns = ntohl(*(uint32_t*)((char*)channel0->_buffer.data()+frame_index));
                frame_index += sizeof(uint32_t);
                static ed247_timestamp_t now;
                ed247_get_time(&now);
                int32_t delta = ((int32_t)now.epoch_s - (int32_t)timestamp.epoch_s);
                delta = delta >= 0 ? delta : -delta;
                ASSERT_TRUE(delta < 30); // Check that frame header timestamp is in a good interval of time
            }else{
                frame_index += sizeof(uint32_t);
                frame_index += sizeof(uint32_t);
            }
        }
        for(auto stream : channel0->streams()){
            auto sid = ntohs(*(ed247_uid_t*)((char*)channel0->_buffer.data()+frame_index));
            frame_index += sizeof(ed247_uid_t);
            ASSERT_EQ(sid, stream.first);
            auto sample_size = ntohs(*(uint16_t*)((char*)channel0->_buffer.data()+frame_index));
            frame_index += sizeof(uint16_t);
            if(stream.second->get_type() == ED247_STREAM_TYPE_VNAD){
                ASSERT_EQ(sample_size, (stream.second->get_sample_max_size_bytes()+sizeof(uint16_t))*stream.second->get_sample_max_number());
            }else if(stream.second->get_type() == ED247_STREAM_TYPE_A664){
                ASSERT_EQ(sample_size, (sizeof(uint16_t)+stream.second->get_sample_max_size_bytes())*stream.second->get_sample_max_number());
            }else if(stream.second->get_type() == ED247_STREAM_TYPE_A825){
                ASSERT_EQ(sample_size, (sizeof(uint8_t)+stream.second->get_sample_max_size_bytes())*stream.second->get_sample_max_number());
            }else if(stream.second->get_type() == ED247_STREAM_TYPE_SERIAL){
                ASSERT_EQ(sample_size, (sizeof(uint16_t)+stream.second->get_sample_max_size_bytes())*stream.second->get_sample_max_number());
            }else{
                ASSERT_EQ(sample_size, (stream.second->get_sample_max_size_bytes())*stream.second->get_sample_max_number());
            }
            frame_index += sample_size;
        }

        // Decode sample
        malloc_count_start();
        channel1->decode((const char*)channel0->_buffer.data(), channel0->_buffer.size());
        ASSERT_EQ(malloc_count_stop(), 0);

        // Check frame header
        if(channel0->_configuration->_header._enable == ED247_YESNO_YES){
            ASSERT_EQ(channel0->_header.get_next_serial_number()-1, channel1->_header.get_recv_frame_details().sequence_number);
        }

        // Pop sample & check samples
        bool empty;
        for(auto stream : channel1->streams()){
            for(uint32_t i = 0 ; i < stream.second->get_sample_max_number() ; i++){
                malloc_count_start();
                auto& sample = stream.second->pop_sample(&empty);
                ASSERT_EQ(malloc_count_stop(), 0);
                std::string str_sample = strize() << std::setw(stream.second->get_sample_max_size_bytes()) << std::setfill('0') << i;
                auto str_sample_recv = std::string((char*)sample.data(), stream.second->get_sample_max_size_bytes());
                ASSERT_EQ(str_sample, str_sample_recv);
                // Check header
                if(channel0->_configuration->_header._enable == ED247_YESNO_YES){
                    ASSERT_EQ(sample.frame_details().sequence_number, i ? (channel0->_header.get_next_serial_number()-1) : 0);
                }
                if(channel0->_configuration->_header._transport_timestamp == ED247_YESNO_YES){
                    ASSERT_NE(sample.frame_details().transport_timestamp.epoch_s, 0);
                    ASSERT_NE(sample.frame_details().transport_timestamp.offset_ns, 0);
                }
                if(i < (stream.second->get_sample_max_number()-1))
                    ASSERT_FALSE(empty);
                else
                    ASSERT_TRUE(empty);
            }
        }

        delete context;
    }
    catch(std::exception & e)
    {
        SAY("Failure: " << e.what());
        ASSERT_TRUE(false);
    }
    catch(...)
    {
        SAY("Failure");
        ASSERT_TRUE(false);
    }
}

std::vector<std::string> configuration_files;

INSTANTIATE_TEST_CASE_P(ChannelTests, ChannelContext,
    ::testing::ValuesIn(configuration_files));

/*************
 * Functions *
 *************/

int main(int argc, char **argv)
{
    if(argc >=1)
        config_path = argv[1];
    else
        config_path = "../config";

    SAY("Configuration path: " << config_path);

    configuration_files.push_back(config_path+"/ecic_unit_channels_channels0.xml");
    configuration_files.push_back(config_path+"/ecic_unit_channels_channels1.xml");

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
