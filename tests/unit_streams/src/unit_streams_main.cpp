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

#include "unitary_test.h"
#include "ed247_context.h"
#include "ed247_xml.h"

std::string config_path = "../config";

TEST(CircularStreamSampleBuffer, Main)
{
    ed247::CircularStreamSampleBuffer cbuffer;
    cbuffer.allocate(sizeof(uint32_t), 4);
    ASSERT_EQ(cbuffer.size(), (size_t)0);
    ASSERT_EQ(cbuffer.front(), nullptr);
    ASSERT_EQ(cbuffer.back(), nullptr);

    uint32_t i = 1;
    ed247::StreamSample sample;
    sample.allocate(sizeof(uint32_t));
    ASSERT_EQ(sample.copy((void *)&i, sizeof(uint32_t)), true);
    ASSERT_EQ(*(uint32_t*)sample.data(), 1);

    ASSERT_TRUE(cbuffer.next_write()->copy(sample));
    ASSERT_FALSE(cbuffer.increment());
    ASSERT_EQ(cbuffer.size(), (size_t)1);
    ASSERT_EQ(*(uint32_t*)(cbuffer.front()->data()), (uint32_t)1);
    ASSERT_EQ(*(uint32_t*)(cbuffer.back()->data()), (uint32_t)1);

    bool empty;
    cbuffer.pop_front(&empty);
    ASSERT_TRUE(empty);
    ASSERT_EQ(cbuffer.size(), (size_t)0);
    cbuffer.pop_front(&empty);
    ASSERT_TRUE(empty);

    i = 1; ASSERT_EQ(sample.copy((void *)&i, sizeof(uint32_t)), true);
    ASSERT_TRUE(cbuffer.next_write()->copy(sample));
    ASSERT_FALSE(cbuffer.increment());
    i = 2; ASSERT_EQ(sample.copy((void *)&i, sizeof(uint32_t)), true);
    ASSERT_TRUE(cbuffer.next_write()->copy(sample));
    ASSERT_FALSE(cbuffer.increment());
    i = 3; ASSERT_EQ(sample.copy((void *)&i, sizeof(uint32_t)), true);
    ASSERT_TRUE(cbuffer.next_write()->copy(sample));
    ASSERT_FALSE(cbuffer.increment());
    i = 4; ASSERT_EQ(sample.copy((void *)&i, sizeof(uint32_t)), true);
    ASSERT_TRUE(cbuffer.next_write()->copy(sample));
    ASSERT_TRUE(cbuffer.increment());
    ASSERT_EQ(cbuffer.size(), (size_t)4);
    ASSERT_EQ(*(uint32_t*)(cbuffer.front()->data()), (uint32_t)1);
    ASSERT_EQ(*(uint32_t*)(cbuffer.back()->data()), (uint32_t)4);

    i = 5; ASSERT_EQ(sample.copy((void *)&i, sizeof(uint32_t)), true);
    ASSERT_TRUE(cbuffer.next_write()->copy(sample));
    ASSERT_TRUE(cbuffer.increment());
    ASSERT_EQ(cbuffer.size(), (size_t)4);
    ASSERT_EQ(*(uint32_t*)(cbuffer.front()->data()), (uint32_t)2);
    ASSERT_EQ(*(uint32_t*)(cbuffer.back()->data()), (uint32_t)5);

    cbuffer.pop_front(&empty);
    ASSERT_FALSE(empty);
}

class StreamContext : public ::testing::TestWithParam<std::string>
{
    protected:

        StreamContext()
        {
            SAY("StreamContext::Ctor");
        }

        ~StreamContext() override
        {
            SAY("StreamContext::Dtor");
        }

        // Per-test-suite set-up
        // Called before the first test in this test suite
        // Can be omitted if not needed
        static void SetUpTestSuite()
        {
            SAY("StreamContext::SetUpTestSuite");
        }

        // Per-test-suite tear-down
        // Called after the last test in this test suite
        // Can be omitted if not needed
        static void TearDownTestSuite()
        {
            SAY("StreamContext::TearDownTestSuite");
        }

        // Per-test set-up logic
        virtual void SetUp()
        {
            SAY("StreamContext::SetUp");
        }

        // Per-test tear-down logic
        virtual void TearDown()
        {
            SAY("StreamContext::TearDown");
        }
};

TEST_P(StreamContext, SinglePushPop)
{
    try{
        RecordProperty("description", strize() << "Load content of [" << GetParam() << "]");

        std::string filepath = GetParam();
        ed247::Context * context = ed247::Context::Builder::create_filepath(filepath);
        ed247::Context::Builder::initialize(*context);

        // Retrieve the pool of streams
        auto pool_streams = context->getPoolStreams();
        ASSERT_EQ(pool_streams->size(), (size_t)6);

        // Check finder for find all
        auto streams_0 = pool_streams->find(".*");
        ASSERT_EQ(streams_0.size(), (size_t)6);

        // Check finder for a single stream
        auto streams_1 = pool_streams->find("Stream1"); // UID=2
        ASSERT_EQ(streams_1.size(), (size_t)1);
        auto stream_1 = streams_1[0];

        // Create a stream sample compatible with the stream
        auto stream_1_sample = stream_1->allocate_sample();
        ASSERT_EQ(stream_1_sample->size(), (size_t)0);
        ASSERT_EQ(stream_1_sample->capacity(), stream_1->get_configuration()->info.sample_max_size_bytes);
        std::string str_sample = strize() << std::setw(stream_1->get_configuration()->info.sample_max_size_bytes) << std::setfill('0') << "H";
        stream_1_sample->copy(str_sample.c_str(), stream_1->get_configuration()->info.sample_max_size_bytes);


        // Push sample (to send stack)
        bool full;
        malloc_count_start();
        ASSERT_TRUE(stream_1->push_sample(stream_1_sample->data(), stream_1_sample->size(), NULL, &full));
        ASSERT_TRUE(full); // Should be full
        ASSERT_EQ(malloc_count_stop(), 0);
        std::string str_sample_recv = std::string(stream_1->send_stack().front()->data(), stream_1->get_configuration()->info.sample_max_size_bytes);
        ASSERT_TRUE(memcmp(stream_1_sample->data(),stream_1->send_stack().front()->data(),stream_1_sample->size()) == 0);
        ASSERT_EQ(str_sample, str_sample_recv);

        // Encode sample
        malloc_count_start();
        uint32_t size = stream_1->encode(stream_1->buffer().data_rw(), stream_1->buffer().capacity());
        stream_1->buffer().set_size(size);
        ASSERT_EQ(malloc_count_stop(), 0);
        if(stream_1->get_configuration()->info.type == ED247_STREAM_TYPE_VNAD)
            ASSERT_TRUE(memcmp(stream_1->buffer().data() + sizeof(uint16_t), stream_1_sample->data(), stream_1_sample->size()) == 0);
        else if(stream_1->get_configuration()->info.type == ED247_STREAM_TYPE_A664 &&
                static_cast<const ed247::xml::A664Stream*>(stream_1->get_configuration())->enable_message_size == ED247_YESNO_YES)
            ASSERT_TRUE(memcmp(stream_1->buffer().data() + sizeof(uint16_t), stream_1_sample->data(), stream_1_sample->size()) == 0);
        else if(stream_1->get_configuration()->info.type == ED247_STREAM_TYPE_A825)
            ASSERT_TRUE(memcmp(stream_1->buffer().data() + sizeof(uint8_t), stream_1_sample->data(), stream_1_sample->size()) == 0);
        else if(stream_1->get_configuration()->info.type == ED247_STREAM_TYPE_SERIAL)
            ASSERT_TRUE(memcmp(stream_1->buffer().data() + sizeof(uint16_t), stream_1_sample->data(), stream_1_sample->size()) == 0);
        else if(stream_1->get_configuration()->info.type == ED247_STREAM_TYPE_ANALOG)
            ASSERT_TRUE(memcmp(stream_1->buffer().data(), stream_1_sample->data(), stream_1_sample->size()) != 0); // SWAP
        else
            ASSERT_TRUE(memcmp(stream_1->buffer().data(), stream_1_sample->data(), stream_1_sample->size()) == 0);

        // Decode sample
        malloc_count_start();
        stream_1->decode(stream_1->buffer().data(), stream_1->buffer().size());
        ASSERT_EQ(malloc_count_stop(), 0);

        if ((stream_1->get_configuration()->info.direction & ED247_DIRECTION_IN) != 0)
        {
            // Pop sample
            bool empty;
            malloc_count_start();
            auto stream_1_recv_sample = stream_1->pop_sample(&empty);
            ASSERT_TRUE((bool)stream_1_recv_sample);
            ASSERT_EQ(malloc_count_stop(), 0);
            str_sample_recv = std::string((char *)stream_1_recv_sample->data(), stream_1->get_configuration()->info.sample_max_size_bytes);
            ASSERT_EQ(str_sample, str_sample_recv);
        }
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

TEST_P(StreamContext, MultiPushPop)
{
    try{
        RecordProperty("description", strize() << "Load content of [" << GetParam() << "]");

        std::string filepath = GetParam();
        ed247::Context * context = ed247::Context::Builder::create_filepath(filepath);
        ed247::Context::Builder::initialize(*context);

        // Retrieve the pool of streams
        auto pool_streams = context->getPoolStreams();
        ASSERT_EQ(pool_streams->size(), (size_t)6);

        // Check finder for find all
        auto streams_0 = pool_streams->find(".*");
        ASSERT_EQ(streams_0.size(), (size_t)6);

        // Check finder for a single stream
        auto streams_1 = pool_streams->find("Stream"); // UID=0
        ASSERT_EQ(streams_1.size(), (size_t)1);
        auto stream_1 = streams_1[0];

        // Create a stream sample compatible with the stream
        auto stream_1_sample = stream_1->allocate_sample();
        ASSERT_EQ(stream_1_sample->size(), (size_t)0);
        ASSERT_EQ(stream_1_sample->capacity(), stream_1->get_configuration()->info.sample_max_size_bytes);

        // Push sample (to send stack)
        bool full;
        std::string str_sample;
        for(uint32_t i = 0 ; i < 10 ; i++){
            str_sample = strize() << std::setw(stream_1->get_configuration()->info.sample_max_size_bytes) << std::setfill('0') << i;
            stream_1_sample->copy(str_sample.c_str(), stream_1->get_configuration()->info.sample_max_size_bytes);
            malloc_count_start();
            ASSERT_TRUE(stream_1->push_sample(stream_1_sample->data(), stream_1_sample->size(), NULL, &full));
            ASSERT_EQ(malloc_count_stop(), 0);
            if(i < 9)
                ASSERT_FALSE(full); // Should no be full as max sample number is 10
            else
                ASSERT_TRUE(full);
        }

        // Check send stack
        std::string str_sample_send;
        for(uint32_t i = 0 ; i < 10 ; i++){
            str_sample = strize() << std::setw(stream_1->get_configuration()->info.sample_max_size_bytes) << std::setfill('0') << i;
            str_sample_send = std::string(stream_1->send_stack().at(i)->data(), stream_1->get_configuration()->info.sample_max_size_bytes);
            ASSERT_EQ(str_sample, str_sample_send);
        }

        // Encode sample & check generated frame
        uint32_t size = stream_1->encode(stream_1->buffer().data_rw(), stream_1->buffer().capacity());
        stream_1->buffer().set_size(size);
        std::string str_sample_frame;
        size_t frame_index = 0;
        if(stream_1->get_configuration()->info.type != ED247_STREAM_TYPE_VNAD){
            for(uint32_t i = 0 ; i < 10 ; i++){
                str_sample = strize() << std::setw(stream_1->get_configuration()->info.sample_max_size_bytes) << std::setfill('0') << i;
                auto sample_size = stream_1->get_configuration()->info.sample_max_size_bytes;
                if(stream_1->get_configuration()->info.type == ED247_STREAM_TYPE_A664 &&
                    static_cast<const ed247::xml::A664Stream*>(stream_1->get_configuration())->enable_message_size == ED247_YESNO_YES){
                    sample_size = ntohs(*(uint16_t*)(stream_1->buffer().data()+frame_index));
                    frame_index += sizeof(uint16_t);
                }else if(stream_1->get_configuration()->info.type == ED247_STREAM_TYPE_A825){
                    sample_size = *(uint8_t*)(stream_1->buffer().data()+frame_index);
                    frame_index += sizeof(uint8_t);
                }else if(stream_1->get_configuration()->info.type == ED247_STREAM_TYPE_SERIAL){
                    sample_size = *(uint16_t*)(stream_1->buffer().data()+frame_index);
                    frame_index += sizeof(uint16_t);
                }
                str_sample_frame = std::string(stream_1->buffer().data()+frame_index, sample_size);
                if(stream_1->get_configuration()->info.type != ED247_STREAM_TYPE_ANALOG){
                    ASSERT_EQ(str_sample, str_sample_frame);
                }
                frame_index += sample_size;
            }
        }

        // Decode sample
        malloc_count_start();
        stream_1->decode(stream_1->buffer().data(), stream_1->buffer().size());
        ASSERT_EQ(malloc_count_stop(), 0);

        if ((stream_1->get_configuration()->info.direction & ED247_DIRECTION_IN) != 0)
        {
            // Pop sample & check samples
            bool empty;
            std::string str_sample_recv;
            for(uint32_t i = 0; i < 10 ; i++){
                str_sample = strize() << std::setw(stream_1->get_configuration()->info.sample_max_size_bytes) << std::setfill('0') << i;
                malloc_count_start();
                auto sample = stream_1->pop_sample(&empty);
                ASSERT_TRUE((bool)sample);
                ASSERT_EQ(malloc_count_stop(), 0);
                str_sample_recv = std::string(sample->data(), stream_1->get_configuration()->info.sample_max_size_bytes);
                ASSERT_EQ(str_sample, str_sample_recv);
                if(i < 9)
                    ASSERT_FALSE(empty);
                else
                    ASSERT_TRUE(empty);
            }
        }
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

TEST_P(StreamContext, MultiPushPopDataTimestamp)
{
    try{
        RecordProperty("description", strize() << "Load content of [" << GetParam() << "]");

        std::string filepath = GetParam();
        ed247::Context * context = ed247::Context::Builder::create_filepath(filepath);
        ed247::Context::Builder::initialize(*context);

        // Retrieve the pool of streams
        auto pool_streams = context->getPoolStreams();
        ASSERT_EQ(pool_streams->size(), (size_t)6);

        // Check finder for find all
        auto streams_0 = pool_streams->find(".*");
        ASSERT_EQ(streams_0.size(), (size_t)6);

        // Check finder for a single stream
        auto streams_out = pool_streams->find("StreamDatatimestampOut"); // UID=4
        ASSERT_EQ(streams_out.size(), (size_t)1);
        auto stream_out = streams_out[0];

        // Create a stream sample compatible with the stream
        auto stream_out_sample = stream_out->allocate_sample();
        ASSERT_EQ(stream_out_sample->size(), (size_t)0);
        ASSERT_EQ(stream_out_sample->capacity(), stream_out->get_configuration()->info.sample_max_size_bytes);

        // Push sample (to send stack)
        bool full;
        std::string str_sample;
        ed247_timestamp_t timestamp;
        for(uint32_t i = 0 ; i < 10 ; i++){
            str_sample = strize() << std::setw(stream_out->get_configuration()->info.sample_max_size_bytes) << std::setfill('0') << i;
            timestamp.epoch_s = 1234567+i%2;
            timestamp.offset_ns = 8910+i;
            stream_out_sample->copy(str_sample.c_str(), stream_out->get_configuration()->info.sample_max_size_bytes);
            stream_out_sample->set_data_timestamp(timestamp);
            malloc_count_start();
            ASSERT_TRUE(stream_out->push_sample(stream_out_sample->data(), stream_out_sample->size(), stream_out_sample->data_timestamp(), &full));
            ASSERT_EQ(malloc_count_stop(), 0);
            if(i < 9)
                ASSERT_FALSE(full); // Should no be full as max sample number is 10
            else
                ASSERT_TRUE(full);
        }

        // Check send stack
        std::string str_sample_send;
        for(uint32_t i = 0 ; i < 10 ; i++){
            str_sample = strize() << std::setw(stream_out->get_configuration()->info.sample_max_size_bytes) << std::setfill('0') << i;
            str_sample_send = std::string(stream_out->send_stack().at(i)->data(), stream_out->get_configuration()->info.sample_max_size_bytes);
            ASSERT_EQ(str_sample, str_sample_send);
            timestamp.epoch_s = 1234567+i%2;
            timestamp.offset_ns = 8910+i;
            ASSERT_EQ(timestamp.epoch_s, stream_out->send_stack().at(i)->data_timestamp()->epoch_s);
            ASSERT_EQ(timestamp.offset_ns, stream_out->send_stack().at(i)->data_timestamp()->offset_ns);
        }

        // Encode sample & check generated frame
        uint32_t size = stream_out->encode(stream_out->buffer().data_rw(), stream_out->buffer().capacity());
        stream_out->buffer().set_size(size);
        std::string str_sample_frame;
        ed247_timestamp_t data_timestamp;
        ed247_timestamp_t timestamp_frame;
        size_t frame_index = 0;
        bool precise_timestamp = stream_out->get_configuration()->data_timestamp.enable_sample_offset == ED247_YESNO_YES;
        if(stream_out->get_configuration()->info.type != ED247_STREAM_TYPE_VNAD){
            for(uint32_t i = 0 ; i < 10 ; i++){
                str_sample = strize() << std::setw(stream_out->get_configuration()->info.sample_max_size_bytes) << std::setfill('0') << i;
                if(i == 0){
                    timestamp.epoch_s = 1234567;
                    timestamp.offset_ns = 8910;
                    timestamp_frame.epoch_s = ntohl(*(uint32_t*)(stream_out->buffer().data()+frame_index));
                    frame_index += sizeof(uint32_t);
                    ASSERT_EQ(timestamp.epoch_s, timestamp_frame.epoch_s);
                    timestamp_frame.offset_ns = ntohl(*(uint32_t*)(stream_out->buffer().data()+frame_index));
                    frame_index += sizeof(uint32_t);
                    ASSERT_EQ(timestamp.offset_ns, timestamp_frame.offset_ns);
                    data_timestamp = timestamp;
                }else if(precise_timestamp){
                    timestamp.epoch_s = 1234567+i%2;
                    timestamp.offset_ns = 8910+i;
                    int32_t offset_ns = (int32_t)ntohl(*(uint32_t*)(stream_out->buffer().data()+frame_index));
                    frame_index += sizeof(int32_t);
                    timestamp_frame = data_timestamp;
                    timestamp_frame.epoch_s += offset_ns / 1000000000;
                    ASSERT_EQ(timestamp.epoch_s, timestamp_frame.epoch_s);
                    timestamp_frame.offset_ns += (offset_ns - (offset_ns / 1000000000) * 1000000000);
                    ASSERT_EQ(timestamp.offset_ns, timestamp_frame.offset_ns);
                }
                auto sample_size = stream_out->get_configuration()->info.sample_max_size_bytes;
                if(stream_out->get_configuration()->info.type == ED247_STREAM_TYPE_A664 &&
                    static_cast<const ed247::xml::A664Stream*>(stream_out->get_configuration())->enable_message_size == ED247_YESNO_YES){
                    sample_size = ntohs(*(uint16_t*)(stream_out->buffer().data()+frame_index));
                    frame_index += sizeof(uint16_t);
                }else if(stream_out->get_configuration()->info.type == ED247_STREAM_TYPE_A825){
                    sample_size = *(uint8_t*)(stream_out->buffer().data()+frame_index);
                    frame_index += sizeof(uint8_t);
                }else if(stream_out->get_configuration()->info.type == ED247_STREAM_TYPE_SERIAL){
                    sample_size = *(uint16_t*)(stream_out->buffer().data()+frame_index);
                    frame_index += sizeof(uint16_t);
                }
                str_sample_frame = std::string(stream_out->buffer().data()+frame_index, sample_size);
                if(stream_out->get_configuration()->info.type != ED247_STREAM_TYPE_ANALOG){
                    ASSERT_EQ(str_sample, str_sample_frame);
                }
                frame_index += sample_size;
            }
        }

        // Decode sample
        malloc_count_start();
        stream_out->decode(stream_out->buffer().data(), stream_out->buffer().size());
        ASSERT_EQ(malloc_count_stop(), 0);

        if ((stream_out->get_configuration()->info.direction & ED247_DIRECTION_IN) != 0 &&
            std::string(stream_out->get_configuration()->info.name) != "StreamDatatimestampOut")
        {
            // Pop sample & check samples
            bool empty;
            std::string str_sample_recv;
            for(uint32_t i = 0; i < 10 ; i++){
                str_sample = strize() << std::setw(stream_out->get_configuration()->info.sample_max_size_bytes) << std::setfill('0') << i;
                if(precise_timestamp){
                    timestamp.epoch_s = 1234567+i%2;
                    timestamp.offset_ns = 8910+i;
                }else{
                    timestamp.epoch_s = 1234567;
                    timestamp.offset_ns = 8910;
                }
                malloc_count_start();
                auto sample = stream_out->pop_sample(&empty);
                ASSERT_TRUE((bool)sample);
                ASSERT_EQ(malloc_count_stop(), 0);
                str_sample_recv = std::string(sample->data(), stream_out->get_configuration()->info.sample_max_size_bytes);
                ASSERT_EQ(str_sample, str_sample_recv);
                ASSERT_EQ(sample->data_timestamp()->epoch_s, timestamp.epoch_s);
                ASSERT_EQ(sample->data_timestamp()->offset_ns, timestamp.offset_ns);
                if(i < 9)
                    ASSERT_FALSE(empty);
                else
                    ASSERT_TRUE(empty);
            }
        }
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

INSTANTIATE_TEST_CASE_P(StreamTests, StreamContext,
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

    configuration_files.push_back(config_path+"/ecic_unit_streams_a429.xml");
    configuration_files.push_back(config_path+"/ecic_unit_streams_a664.xml");
    configuration_files.push_back(config_path+"/ecic_unit_streams_a825.xml");
    configuration_files.push_back(config_path+"/ecic_unit_streams_serial.xml");
    configuration_files.push_back(config_path+"/ecic_unit_streams_dis.xml");
    configuration_files.push_back(config_path+"/ecic_unit_streams_ana.xml");
    configuration_files.push_back(config_path+"/ecic_unit_streams_nad.xml");
    configuration_files.push_back(config_path+"/ecic_unit_streams_vnad.xml");

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
