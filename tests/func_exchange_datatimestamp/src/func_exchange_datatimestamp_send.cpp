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

#include "test_context.h"

/***********
 * Defines *
 ***********/

#define TEST_ENTITY_SRC_ID 1
#define TEST_ENTITY_DST_ID 2

#define TEST_CONTEXT_SYNC_MAIN TestSend(); TestWait();
#define TEST_CONTEXT_SYNC_TESTER TestWait(); TestSend();

#define TEST_CONTEXT_SYNC TEST_CONTEXT_SYNC_MAIN

/********
 * Test *
 ********/

std::string config_path = "../config";

/******************************************************************************
This test file globally checks communication for all type of signals and every
configuration. Stream/Signals, Unicast/Multicast and all protocols are checked.
Although multicast is tested, all test cases only involve 2 components: a
sender and a receiver. This file codes for the sender application.
******************************************************************************/

class StreamContext : public TestContext {};

/******************************************************************************
This test checks the global communication between 2 ed247 components for every
stream protocol present in test vectors. 3 successive sequences are run:
Checkpoint 1 - Send a single data on a single stream
Checkpoint 2 - Send several data on a single stream
Checkpoint 3 - Send several data on a several streams
This is done for every protocol provided and each byte of each sample is filled
with 0s except for the last one that is filled with an other digit depending
on the Checkpoint. This test expects the ECIC files to define channels with
multiple streams which define Stream0 and Stream 1 among them. Provided
configurations are defined in vector stream_files
******************************************************************************/
TEST_P(StreamContext, SingleFrame)
{
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    const ed247_stream_info_t *stream_info;
    void *sample;
    size_t sample_size;
    std::string str_send;
    std::ostringstream oss;

    // Stream
    ASSERT_EQ(ed247_find_streams(_context, "Stream0", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);

    // Sample
    ASSERT_EQ(ed247_stream_allocate_sample(stream, &sample, &sample_size), ED247_STATUS_SUCCESS);

    // Checkpoint n°1
    // For this checkpoint the last byte is filled with 1
    // A single sample is sent on one of the streams of the channel
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°1" << std::endl;
    TEST_CONTEXT_SYNC

    // Payload
    oss.str("");
    oss << std::setw(stream_info->sample_max_size_bytes) << std::setfill('0') << 1;
    str_send = oss.str();
    memcpy(sample, str_send.c_str(), stream_info->sample_max_size_bytes);

    // Data Timestamp
    ed247_timestamp_t timestamp;
    timestamp.epoch_s = 1234567;
    timestamp.offset_ns = 8910;

    memhooks_section_start();

    ASSERT_EQ(ed247_stream_push_sample(stream, sample, sample_size, &timestamp, NULL), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(memhooks_section_stop());

    // Checkpoint n°2
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°2" << std::endl;
    TEST_CONTEXT_SYNC

    // Unload
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
}

uint32_t com_send_counter;

TEST_P(StreamContext, Callbacks)
{
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    const ed247_stream_info_t *stream_info;
    void *sample;
    size_t sample_size;
    std::string str_send;
    std::ostringstream oss;

    uint32_t user_data = 10;
    com_send_counter = 0;
    ASSERT_EQ(ed247_component_set_user_data(_context, &user_data), ED247_STATUS_SUCCESS);
    auto callback = [](ed247_context_t context) -> ed247_status_t {
        void *user_data;
        ed247_component_get_user_data(context, &user_data);
        if(user_data){
            com_send_counter += *(uint32_t*)user_data;
        }
        return ED247_STATUS_SUCCESS;
    };
    ASSERT_EQ(ed247_unregister_com_send_callback(_context, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_register_com_send_callback(NULL, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_register_com_send_callback(_context, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_register_com_send_callback(_context, callback), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_unregister_com_send_callback(NULL, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_unregister_com_send_callback(_context, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_unregister_com_send_callback(_context, callback), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_register_com_send_callback(_context, callback), ED247_STATUS_SUCCESS);

    // Checkpoint n°1
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°1" << std::endl;
    TEST_CONTEXT_SYNC

    // Stream
    ASSERT_EQ(ed247_find_streams(_context, "Stream0", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);

    // Sample
    ASSERT_EQ(ed247_stream_allocate_sample(stream, &sample, &sample_size), ED247_STATUS_SUCCESS);

    // Payload
    oss.str("");
    oss << std::setw(stream_info->sample_max_size_bytes) << std::setfill('0') << 1;
    str_send = oss.str();
    memcpy(sample, str_send.c_str(), stream_info->sample_max_size_bytes);

    memhooks_section_start();

    ASSERT_EQ(ed247_stream_push_sample(stream, sample, sample_size, NULL, NULL), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

    ASSERT_TRUE(memhooks_section_stop());

    ASSERT_TRUE(com_send_counter > 0);

    // Unload
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);

    free(sample);
}

std::vector<TestParams> stream_files;

INSTANTIATE_TEST_CASE_P(FT_EXCHANGE_STREAMS, StreamContext,
    ::testing::ValuesIn(stream_files));

/*************
 * Functions *
 *************/

int main(int argc, char **argv)
{
    if(argc >=1)
        config_path = argv[1];
    else
        config_path = "../config";

    std::cout << "Configuration path: " << config_path << std::endl;

    stream_files.push_back({TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, config_path+"/ecic_func_exchange_datatimestamp_a429_send.xml"});

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
