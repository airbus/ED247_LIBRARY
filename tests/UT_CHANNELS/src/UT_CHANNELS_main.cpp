/******************************************************************************
 * The MIT Licence
 *
 * Copyright (c) 2020 Airbus Operations S.A.S
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

/************
 * Includes *
 ************/

#include <ed247_test.h>
#include <ed247_logs.h>

#include <stdio.h>
// #include <unistd.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ed247_context.h>
#include <ed247_channel.h>
#include <ed247_cominterface.h>

#include <memory>

/***********
 * Defines *
 ***********/

using namespace ed247;

class ChannelContext : public ::testing::TestWithParam<std::string>
{
    protected:

        ChannelContext()
        {
            LOG_DEBUG() << "ChannelContext::Ctor" << LOG_END;
        }

        ~ChannelContext() override
        {
            LOG_DEBUG() << "ChannelContext::Dtor" << LOG_END;
        }

        // Per-test-suite set-up
        // Called before the first test in this test suite
        // Can be omitted if not needed
        static void SetUpTestSuite()
        {
            LOG_DEBUG() << "ChannelContext::SetUpTestSuite" << LOG_END;
        }

        // Per-test-suite tear-down
        // Called after the last test in this test suite
        // Can be omitted if not needed
        static void TearDownTestSuite()
        {
            LOG_DEBUG() << "ChannelContext::TearDownTestSuite" << LOG_END;
        }

        // Per-test set-up logic
        virtual void SetUp()
        {
            LOG_DEBUG() << "ChannelContext::SetUp" << LOG_END;
        }

        // Per-test tear-down logic
        virtual void TearDown()
        {
            LOG_DEBUG() << "ChannelContext::TearDown" << LOG_END;
        }
};

TEST_P(ChannelContext, MultiPushPop)
{
    try{
        std::ostringstream oss;
        oss << "Load content of [" << GetParam() << "]";
        RecordProperty("description",oss.str());

        std::string filepath{CONFIG_PATH"/ut_channels/"};
        filepath += GetParam();
        Context * context = Context::Builder::create(filepath,libed247_configuration_t(LIBED247_CONFIGURATION_DEFAULT));
        Context::Builder::initialize(*context);

        // Retrieve the pool of channels
        auto pool_channels = context->getPoolChannels();
        ASSERT_EQ(pool_channels->size(), (size_t)2);

        // Check finder for find all
        auto channels = pool_channels->find(".*");
        ASSERT_EQ(channels.size(), (size_t)2);

        // Check finder for a single channel
        auto channels0 = pool_channels->find("Channel0");
        ASSERT_EQ(channels0.size(), (size_t)1);
        auto channel0 = channels0[0];
        auto channels1 = pool_channels->find("Channel1");
        ASSERT_EQ(channels1.size(), (size_t)1);
        auto channel1 = channels1[0];

        // Push stream samples
        for(auto stream : channel0->streams()){
            auto sample = stream.second.stream->allocate_sample();
            bool full;
            for(size_t i = 0 ; i < stream.second.stream->get_configuration()->info.sample_max_number ; i++){
                oss.str("");
                oss << std::setw(stream.second.stream->get_configuration()->info.sample_max_size_bytes) << std::setfill('0') << i;
                sample->copy(oss.str().c_str(),stream.second.stream->get_configuration()->info.sample_max_size_bytes);
                memhooks_section_start();
                stream.second.stream->push_sample(*sample,&full);
                ASSERT_TRUE(memhooks_section_stop());
                if(i < (stream.second.stream->get_configuration()->info.sample_max_number-1))
                    ASSERT_FALSE(full);
                else
                    ASSERT_TRUE(full);
            }
        }

        // Encode channel & check frame
        memhooks_section_start();
        // context->getPoolStreams()->encode();
        channel0->encode(0);
        ASSERT_TRUE(memhooks_section_stop());
        size_t frame_index = 0;
        if(channel0->get_configuration()->header.enable == ED247_YESNO_YES){
            uint16_t pid = ntohs(*(uint16_t*)((char*)channel0->buffer().data()+frame_index));
            frame_index += sizeof(uint16_t);
            ASSERT_EQ(pid, 0);
            uint16_t sn = ntohs(*(uint16_t*)((char*)channel0->buffer().data()+frame_index));
            frame_index += sizeof(uint16_t);
            ASSERT_EQ(sn, channel0->get_header()._send_header.sequence_number-1);
            if(channel0->get_configuration()->header.transport_timestamp == ED247_YESNO_YES){
                ed247_timestamp_t timestamp;
                timestamp.epoch_s = ntohl(*(uint32_t*)((char*)channel0->buffer().data()+frame_index));
                frame_index += sizeof(uint32_t);
                timestamp.offset_ns = ntohl(*(uint32_t*)((char*)channel0->buffer().data()+frame_index));
                frame_index += sizeof(uint32_t);
                static struct timeval tp;
                gettimeofday(&tp, NULL);
                int32_t delta = ((int32_t)tp.tv_sec - (int32_t)timestamp.epoch_s);
                delta = delta >= 0 ? delta : -delta;
                ASSERT_TRUE(delta < 30); // Check that frame header timestamp is in a good interval of time
            }else{
                frame_index += sizeof(uint32_t);
                frame_index += sizeof(uint32_t);
            }
        }
        for(auto stream : channel0->streams()){
            auto sid = ntohs(*(ed247_uid_t*)((char*)channel0->buffer().data()+frame_index));
            frame_index += sizeof(ed247_uid_t);
            ASSERT_EQ(sid, stream.first);
            auto sample_size = ntohs(*(uint16_t*)((char*)channel0->buffer().data()+frame_index));
            frame_index += sizeof(uint16_t);
            if(stream.second.stream->get_configuration()->info.type == ED247_STREAM_TYPE_VNAD){
                ASSERT_EQ(sample_size, (stream.second.stream->get_configuration()->info.sample_max_size_bytes+sizeof(uint16_t))*stream.second.stream->get_configuration()->info.sample_max_number);
            }else if(stream.second.stream->get_configuration()->info.type == ED247_STREAM_TYPE_A664){
                ASSERT_EQ(sample_size, (sizeof(uint16_t)+stream.second.stream->get_configuration()->info.sample_max_size_bytes)*stream.second.stream->get_configuration()->info.sample_max_number);
            }else if(stream.second.stream->get_configuration()->info.type == ED247_STREAM_TYPE_A825){
                ASSERT_EQ(sample_size, (sizeof(uint8_t)+stream.second.stream->get_configuration()->info.sample_max_size_bytes)*stream.second.stream->get_configuration()->info.sample_max_number);
            }else if(stream.second.stream->get_configuration()->info.type == ED247_STREAM_TYPE_SERIAL){
                ASSERT_EQ(sample_size, (sizeof(uint8_t)+stream.second.stream->get_configuration()->info.sample_max_size_bytes)*stream.second.stream->get_configuration()->info.sample_max_number);
            }else{
                ASSERT_EQ(sample_size, (stream.second.stream->get_configuration()->info.sample_max_size_bytes)*stream.second.stream->get_configuration()->info.sample_max_number);
            }
            frame_index += sample_size;
        }

        // Decode sample
        memhooks_section_start();
        channel1->decode((const char*)channel0->buffer().data(), channel0->buffer().size());
        ASSERT_TRUE(memhooks_section_stop());

        // Check frame header
        if(channel0->get_configuration()->header.enable == ED247_YESNO_YES){
            ASSERT_EQ(channel0->get_header()._send_header.component_identifier, channel1->get_header()._recv_headers_iter->component_identifier); 
            ASSERT_EQ(channel0->get_header()._send_header.sequence_number-1, channel1->get_header()._recv_headers_iter->sequence_number);
        }
        if(channel0->get_configuration()->header.transport_timestamp == ED247_YESNO_YES){
            ASSERT_EQ(channel0->get_header()._send_header.transport_timestamp.epoch_s, channel1->get_header()._recv_headers_iter->transport_timestamp.epoch_s);
            ASSERT_EQ(channel0->get_header()._send_header.transport_timestamp.offset_ns, channel1->get_header()._recv_headers_iter->transport_timestamp.offset_ns);
        }

        // Pop sample & check samples
        bool empty;
        for(auto stream : channel1->streams()){
            for(size_t i = 0 ; i < stream.second.stream->get_configuration()->info.sample_max_number ; i++){
                oss.str("");
                oss << std::setw(stream.second.stream->get_configuration()->info.sample_max_size_bytes) << std::setfill('0') << i;
                memhooks_section_start();
                auto sample = stream.second.stream->pop_sample(&empty);
                ASSERT_TRUE(memhooks_section_stop());
                auto str_sample = oss.str();
                auto str_sample_recv = std::string((char*)sample->data(), stream.second.stream->get_configuration()->info.sample_max_size_bytes);
                ASSERT_EQ(str_sample, str_sample_recv);
                // Check header
                if(channel0->get_configuration()->header.enable == ED247_YESNO_YES){
                    ASSERT_EQ(sample->info()->component_identifier, channel0->get_header()._send_header.component_identifier);
                    ASSERT_EQ(sample->info()->sequence_number, i ? (channel0->get_header()._send_header.sequence_number-1) : 0);
                }
                if(channel0->get_configuration()->header.transport_timestamp == ED247_YESNO_YES){
                    ASSERT_EQ(sample->info()->transport_timestamp.epoch_s, channel0->get_header()._send_header.transport_timestamp.epoch_s);
                    ASSERT_EQ(sample->info()->transport_timestamp.offset_ns, channel0->get_header()._send_header.transport_timestamp.offset_ns);
                }
                if(i < (stream.second.stream->get_configuration()->info.sample_max_number-1))
                    ASSERT_FALSE(empty);
                else
                    ASSERT_TRUE(empty);
            }
        }
    }
    catch(std::exception & e)
    {
        LOG_INFO() << "Failure: " << e.what() << LOG_END;
        ASSERT_TRUE(false);
    }
    catch(...)
    {
        LOG_INFO() << "Failure" << LOG_END;
        ASSERT_TRUE(false);
    }
}

const std::vector<std::string> configuration_files = {
    std::string("channels0.xml"),
    std::string("channels1.xml")
};

INSTANTIATE_TEST_CASE_P(ChannelTests, ChannelContext,
    ::testing::ValuesIn(configuration_files));

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}