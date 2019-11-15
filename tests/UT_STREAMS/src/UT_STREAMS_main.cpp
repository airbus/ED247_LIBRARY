/******************************************************************************
 * The MIT Licence
 *
 * Copyright (c) 2019 Airbus Operations S.A.S
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

TEST(CircularStreamSampleBuffer, Main)
{
    CircularStreamSampleBuffer cbuffer;
    cbuffer.allocate(sizeof(uint32_t), 4);
    ASSERT_EQ(cbuffer.size(), (size_t)0);
    ASSERT_EQ(cbuffer.front(), nullptr);
    ASSERT_EQ(cbuffer.back(), nullptr);

    uint32_t i = 0;
    StreamSample sample((void *)&i, sizeof(i));
    i = 1;
    ASSERT_EQ(*(uint32_t*)sample.data(),i);

    ASSERT_FALSE(cbuffer.push_back(sample));
    ASSERT_EQ(cbuffer.size(), (size_t)1);
    ASSERT_EQ(*(uint32_t*)(cbuffer.front()->data()), (uint32_t)1);
    ASSERT_EQ(*(uint32_t*)(cbuffer.back()->data()), (uint32_t)1);

    bool empty;
    cbuffer.pop_front(&empty);
    ASSERT_TRUE(empty);
    ASSERT_EQ(cbuffer.size(), (size_t)0);
    cbuffer.pop_front(&empty);
    ASSERT_TRUE(empty);

    i = 0;
    i++;ASSERT_FALSE(cbuffer.push_back(sample)); // 1
    i++;ASSERT_FALSE(cbuffer.push_back(sample)); // 2
    i++;ASSERT_FALSE(cbuffer.push_back(sample)); // 3
    i++;ASSERT_TRUE(cbuffer.push_back(sample)); // 4
    ASSERT_EQ(cbuffer.size(), (size_t)4);
    ASSERT_EQ(*(uint32_t*)(cbuffer.front()->data()), (uint32_t)1);
    ASSERT_EQ(*(uint32_t*)(cbuffer.back()->data()), (uint32_t)4);
    
    i++;ASSERT_TRUE(cbuffer.push_back(sample));
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
            LOG_DEBUG() << "StreamContext::Ctor" << LOG_END;
        }

        ~StreamContext() override
        {
            LOG_DEBUG() << "StreamContext::Dtor" << LOG_END;
        }

        // Per-test-suite set-up
        // Called before the first test in this test suite
        // Can be omitted if not needed
        static void SetUpTestSuite()
        {
            LOG_DEBUG() << "StreamContext::SetUpTestSuite" << LOG_END;
        }

        // Per-test-suite tear-down
        // Called after the last test in this test suite
        // Can be omitted if not needed
        static void TearDownTestSuite()
        {
            LOG_DEBUG() << "StreamContext::TearDownTestSuite" << LOG_END;
        }

        // Per-test set-up logic
        virtual void SetUp()
        {
            LOG_DEBUG() << "StreamContext::SetUp" << LOG_END;
        }

        // Per-test tear-down logic
        virtual void TearDown()
        {
            LOG_DEBUG() << "StreamContext::TearDown" << LOG_END;
        }
};

TEST_P(StreamContext, SinglePushPop)
{
    try{
        std::ostringstream oss;
        oss << "Load content of [" << GetParam() << "]";
        RecordProperty("description",oss.str());

        std::string filepath{CONFIG_PATH"/ut_streams/"};
        filepath += GetParam();
        Context * context = Context::Builder::create(filepath,libed247_configuration_t(LIBED247_CONFIGURATION_DEFAULT));
        Context::Builder::initialize(*context);

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
        oss.str("");
        oss << std::setw(stream_1->get_configuration()->info.sample_max_size_bytes) << std::setfill('0') << "H";
        std::string str_sample = oss.str();
        stream_1_sample->copy(str_sample.c_str(), stream_1->get_configuration()->info.sample_max_size_bytes);


        // Push sample (to send stack)
        bool full;
        memhooks_section_start();
        stream_1->push_sample(*stream_1_sample, &full);
        ASSERT_TRUE(full); // Should be full
        ASSERT_TRUE(memhooks_section_stop());
        std::string str_sample_recv = std::string((char *)stream_1->send_stack().front()->data(), stream_1->get_configuration()->info.sample_max_size_bytes);
        ASSERT_TRUE(memcmp(stream_1_sample->data(),stream_1->send_stack().front()->data(),stream_1_sample->size()) == 0);
        ASSERT_EQ(str_sample, str_sample_recv);

        // Encode sample
        memhooks_section_start();
        stream_1->encode();
        ASSERT_TRUE(memhooks_section_stop());
        if(stream_1->get_configuration()->info.type == ED247_STREAM_TYPE_VNAD)
            ASSERT_TRUE(memcmp((char*)stream_1->buffer().data() + sizeof(uint16_t), stream_1_sample->data(), stream_1_sample->size()) == 0);
        else if(stream_1->get_configuration()->info.type == ED247_STREAM_TYPE_A664 &&
            static_cast<const xml::A664Stream*>(stream_1->get_configuration())->enable_message_size == ED247_YESNO_YES)
            ASSERT_TRUE(memcmp((char*)stream_1->buffer().data() + sizeof(uint16_t), stream_1_sample->data(), stream_1_sample->size()) == 0);
        else if(stream_1->get_configuration()->info.type == ED247_STREAM_TYPE_A825)
            ASSERT_TRUE(memcmp((char*)stream_1->buffer().data() + sizeof(uint8_t), stream_1_sample->data(), stream_1_sample->size()) == 0);
        else if(stream_1->get_configuration()->info.type == ED247_STREAM_TYPE_SERIAL)
            ASSERT_TRUE(memcmp((char*)stream_1->buffer().data() + sizeof(uint8_t), stream_1_sample->data(), stream_1_sample->size()) == 0);
        else
            ASSERT_TRUE(memcmp((char*)stream_1->buffer().data(), stream_1_sample->data(), stream_1_sample->size()) == 0);

        // Decode sample
        memhooks_section_start();
        stream_1->decode((const char *)stream_1->buffer().data(), stream_1->buffer().size());
        ASSERT_TRUE(memhooks_section_stop());

        if ((stream_1->get_configuration()->info.direction & ED247_DIRECTION_IN) != 0)
        {
            // Pop sample
            bool empty;
            memhooks_section_start();
            auto stream_1_recv_sample = stream_1->pop_sample(&empty);
            ASSERT_TRUE(memhooks_section_stop());
            str_sample_recv = std::string((char *)stream_1_recv_sample->data(), stream_1->get_configuration()->info.sample_max_size_bytes);
            ASSERT_EQ(str_sample, str_sample_recv);
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

TEST_P(StreamContext, MultiPushPop)
{
    try{
        std::ostringstream oss;
        oss << "Load content of [" << GetParam() << "]";
        RecordProperty("description",oss.str());

        std::string filepath{CONFIG_PATH"/ut_streams/"};
        filepath += GetParam();
        Context * context = Context::Builder::create(filepath,libed247_configuration_t(LIBED247_CONFIGURATION_DEFAULT));
        Context::Builder::initialize(*context);

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
            oss.str("");
            oss << std::setw(stream_1->get_configuration()->info.sample_max_size_bytes) << std::setfill('0') << i;
            str_sample = oss.str();
            stream_1_sample->copy(str_sample.c_str(), stream_1->get_configuration()->info.sample_max_size_bytes);
            memhooks_section_start();
            stream_1->push_sample(*stream_1_sample, &full);
            ASSERT_TRUE(memhooks_section_stop());
            if(i < 9)
                ASSERT_FALSE(full); // Should no be full as max sample number is 10
            else
                ASSERT_TRUE(full);
        }

        // Check send stack
        std::string str_sample_send;
        for(uint32_t i = 0 ; i < 10 ; i++){
            oss.str("");
            oss << std::setw(stream_1->get_configuration()->info.sample_max_size_bytes) << std::setfill('0') << i;
            str_sample = oss.str();
            str_sample_send = std::string((char*)stream_1->send_stack().at(i)->data(), stream_1->get_configuration()->info.sample_max_size_bytes);
            ASSERT_EQ(str_sample, str_sample_send);
        }

        // Encode sample & check generated frame
        stream_1->encode();
        std::string str_sample_frame;
        size_t frame_index = 0;
        if(stream_1->get_configuration()->info.type != ED247_STREAM_TYPE_VNAD){
            for(uint32_t i = 0 ; i < 10 ; i++){
                oss.str("");
                oss << std::setw(stream_1->get_configuration()->info.sample_max_size_bytes) << std::setfill('0') << i;
                str_sample = oss.str();
                auto sample_size = stream_1->get_configuration()->info.sample_max_size_bytes;
                if(stream_1->get_configuration()->info.type == ED247_STREAM_TYPE_A664 &&
                    static_cast<const xml::A664Stream*>(stream_1->get_configuration())->enable_message_size == ED247_YESNO_YES){
                    sample_size = ntohs(*(uint16_t*)((char*)stream_1->buffer().data()+frame_index));
                    frame_index += sizeof(uint16_t);
                }else if(stream_1->get_configuration()->info.type == ED247_STREAM_TYPE_A825){
                    sample_size = *(uint8_t*)((char*)stream_1->buffer().data()+frame_index);
                    frame_index += sizeof(uint8_t);
                }else if(stream_1->get_configuration()->info.type == ED247_STREAM_TYPE_SERIAL){
                    sample_size = *(uint8_t*)((char*)stream_1->buffer().data()+frame_index);
                    frame_index += sizeof(uint8_t);
                }
                str_sample_frame = std::string((char*)stream_1->buffer().data()+frame_index, sample_size);
                ASSERT_EQ(str_sample, str_sample_frame);
                frame_index += sample_size;
            }
        }

        // Decode sample
        memhooks_section_start();
        stream_1->decode((const char*)stream_1->buffer().data(), stream_1->buffer().size());
        ASSERT_TRUE(memhooks_section_stop());

        if ((stream_1->get_configuration()->info.direction & ED247_DIRECTION_IN) != 0)
        {
            // Pop sample & check samples
            bool empty;
            std::string str_sample_recv;
            for(uint32_t i = 0; i < 10 ; i++){
                oss.str("");
                oss << std::setw(stream_1->get_configuration()->info.sample_max_size_bytes) << std::setfill('0') << i;
                str_sample = oss.str();
                memhooks_section_start();
                auto sample = stream_1->pop_sample(&empty);
                ASSERT_TRUE(memhooks_section_stop());
                str_sample_recv = std::string((char*)sample->data(), stream_1->get_configuration()->info.sample_max_size_bytes);
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
        LOG_INFO() << "Failure: " << e.what() << LOG_END;
        ASSERT_TRUE(false);
    }
    catch(...)
    {
        LOG_INFO() << "Failure" << LOG_END;
        ASSERT_TRUE(false);
    }
}

TEST_P(StreamContext, MultiPushPopDataTimestamp)
{
    try{
        std::ostringstream oss;
        oss << "Load content of [" << GetParam() << "]";
        RecordProperty("description",oss.str());

        std::string filepath{CONFIG_PATH"/ut_streams/"};
        filepath += GetParam();
        Context * context = Context::Builder::create(filepath,libed247_configuration_t(LIBED247_CONFIGURATION_DEFAULT));
        Context::Builder::initialize(*context);

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
            oss.str("");
            oss << std::setw(stream_out->get_configuration()->info.sample_max_size_bytes) << std::setfill('0') << i;
            str_sample = oss.str();
            timestamp.epoch_s = 1234567+i%2;
            timestamp.offset_ns = 8910+i;
            stream_out_sample->copy(str_sample.c_str(), stream_out->get_configuration()->info.sample_max_size_bytes);
            stream_out_sample->set_data_timestamp(timestamp);
            memhooks_section_start();
            stream_out->push_sample(*stream_out_sample, &full);
            ASSERT_TRUE(memhooks_section_stop());
            if(i < 9)
                ASSERT_FALSE(full); // Should no be full as max sample number is 10
            else
                ASSERT_TRUE(full);
        }

        // Check send stack
        std::string str_sample_send;
        for(uint32_t i = 0 ; i < 10 ; i++){
            oss.str("");
            oss << std::setw(stream_out->get_configuration()->info.sample_max_size_bytes) << std::setfill('0') << i;
            str_sample = oss.str();
            str_sample_send = std::string((char*)stream_out->send_stack().at(i)->data(), stream_out->get_configuration()->info.sample_max_size_bytes);
            ASSERT_EQ(str_sample, str_sample_send);
            timestamp.epoch_s = 1234567+i%2;
            timestamp.offset_ns = 8910+i;
            ASSERT_EQ(timestamp.epoch_s, stream_out->send_stack().at(i)->data_timestamp()->epoch_s);
            ASSERT_EQ(timestamp.offset_ns, stream_out->send_stack().at(i)->data_timestamp()->offset_ns);
        }

        // Encode sample & check generated frame
        stream_out->encode();
        std::string str_sample_frame;
        ed247_timestamp_t data_timestamp;
        ed247_timestamp_t timestamp_frame;
        size_t frame_index = 0;
        bool precise_timestamp = stream_out->get_configuration()->data_timestamp.enable_sample_offset == ED247_YESNO_YES;
        if(stream_out->get_configuration()->info.type != ED247_STREAM_TYPE_VNAD){
            for(uint32_t i = 0 ; i < 10 ; i++){
                oss.str("");
                oss << std::setw(stream_out->get_configuration()->info.sample_max_size_bytes) << std::setfill('0') << i;
                str_sample = oss.str();
                if(i == 0){
                    timestamp.epoch_s = 1234567;
                    timestamp.offset_ns = 8910;
                    timestamp_frame.epoch_s = ntohl(*(uint32_t*)((char*)stream_out->buffer().data()+frame_index));
                    frame_index += sizeof(uint32_t);
                    ASSERT_EQ(timestamp.epoch_s, timestamp_frame.epoch_s);
                    timestamp_frame.offset_ns = ntohl(*(uint32_t*)((char*)stream_out->buffer().data()+frame_index));
                    frame_index += sizeof(uint32_t);
                    ASSERT_EQ(timestamp.offset_ns, timestamp_frame.offset_ns);
                    data_timestamp = timestamp;
                }else if(precise_timestamp){
                    timestamp.epoch_s = 1234567+i%2;
                    timestamp.offset_ns = 8910+i;
                    int32_t offset_ns = (int32_t)ntohl(*(uint32_t*)((char*)stream_out->buffer().data()+frame_index));
                    frame_index += sizeof(int32_t);
                    timestamp_frame = data_timestamp;
                    timestamp_frame.epoch_s += offset_ns / 1000000000;
                    ASSERT_EQ(timestamp.epoch_s, timestamp_frame.epoch_s);
                    timestamp_frame.offset_ns += (offset_ns - (offset_ns / 1000000000) * 1000000000);
                    ASSERT_EQ(timestamp.offset_ns, timestamp_frame.offset_ns);
                }
                auto sample_size = stream_out->get_configuration()->info.sample_max_size_bytes;
                if(stream_out->get_configuration()->info.type == ED247_STREAM_TYPE_A664 &&
                    static_cast<const xml::A664Stream*>(stream_out->get_configuration())->enable_message_size == ED247_YESNO_YES){
                    sample_size = ntohs(*(uint16_t*)((char*)stream_out->buffer().data()+frame_index));
                    frame_index += sizeof(uint16_t);
                }else if(stream_out->get_configuration()->info.type == ED247_STREAM_TYPE_A825){
                    sample_size = *(uint8_t*)((char*)stream_out->buffer().data()+frame_index);
                    frame_index += sizeof(uint8_t);
                }else if(stream_out->get_configuration()->info.type == ED247_STREAM_TYPE_SERIAL){
                    sample_size = *(uint8_t*)((char*)stream_out->buffer().data()+frame_index);
                    frame_index += sizeof(uint8_t);
                }
                str_sample_frame = std::string((char*)stream_out->buffer().data()+frame_index, sample_size);
                ASSERT_EQ(str_sample, str_sample_frame);
                frame_index += sample_size;
                // str_sample_frame = std::string((char*)stream_out->buffer().data()+frame_index, stream_out->get_configuration()->info.sample_max_size_bytes);
                // ASSERT_EQ(str_sample, str_sample_frame);
                // frame_index += stream_out->get_configuration()->info.sample_max_size_bytes;
            }
        }

        // Decode sample
        memhooks_section_start();
        stream_out->decode((const char*)stream_out->buffer().data(), stream_out->buffer().size());
        ASSERT_TRUE(memhooks_section_stop());

        if ((stream_out->get_configuration()->info.direction & ED247_DIRECTION_IN) != 0 &&
            std::string(stream_out->get_configuration()->info.name) != "StreamDatatimestampOut")
        {
            // Pop sample & check samples
            bool empty;
            std::string str_sample_recv;
            for(uint32_t i = 0; i < 10 ; i++){
                oss.str("");
                oss << std::setw(stream_out->get_configuration()->info.sample_max_size_bytes) << std::setfill('0') << i;
                str_sample = oss.str();
                if(precise_timestamp){
                    timestamp.epoch_s = 1234567+i%2;
                    timestamp.offset_ns = 8910+i;
                }else{
                    timestamp.epoch_s = 1234567;
                    timestamp.offset_ns = 8910;
                }
                memhooks_section_start();
                auto sample = stream_out->pop_sample(&empty);
                ASSERT_TRUE(memhooks_section_stop());
                str_sample_recv = std::string((char*)sample->data(), stream_out->get_configuration()->info.sample_max_size_bytes);
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
        LOG_INFO() << "Failure: " << e.what() << LOG_END;
        ASSERT_TRUE(false);
    }
    catch(...)
    {
        LOG_INFO() << "Failure" << LOG_END;
        ASSERT_TRUE(false);
    }
}

std::vector<std::string> configuration_files = {
    std::string("a429.xml"),
    std::string("a664.xml"),
    std::string("a825.xml"),
    std::string("serial.xml"),
    std::string("dis.xml"),
    std::string("ana.xml"),
    std::string("nad.xml"),
    std::string("vnad.xml")
};

INSTANTIATE_TEST_CASE_P(StreamTests, StreamContext,
    ::testing::ValuesIn(configuration_files));

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}