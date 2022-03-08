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
#define TEST_ACTOR1_NAME "send"
#define TEST_ACTOR2_NAME "recv"
#define TEST_ACTOR_ID TEST_ACTOR2_ID
#include "functional_test.h"

std::string config_path = "../config";

/******************************************************************************
This test file globally checks communication for all type of signals and every
configuration. Stream/Signals, Unicast/Multicast and all protocols are checked.
Although multicast is tested, all test cases only involve 2 components: a
sender and a receiver. This file codes for the receiver application.
Further common details of the test sequence are documented in the sender
application.
******************************************************************************/

class StreamContext : public TestContext {};

/******************************************************************************
This test case also specifically checks the behavior of the get_simulation_time
******************************************************************************/
TEST_P(StreamContext, SingleFrame)
{
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    const ed247_stream_info_t *stream_info;
    const void *sample;
    size_t sample_size;
    std::string str_send, str_recv;
    const ed247_timestamp_t* timestamp;
    
    // Checkpoint n~1
    SAY_SELF("Checkpoint n~1");
    TEST_SYNC();

    malloc_count_start();

    // Recv a single frame
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);

    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_pop_sample(stream, &sample, &sample_size, &timestamp, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
    
    ASSERT_EQ(malloc_count_stop(), 0);
    
    // Extract and check content of payload
    str_send = strize() << std::setw(stream_info->sample_max_size_bytes) << std::setfill('0') << 1;
    str_recv = std::string((char*)sample, stream_info->sample_max_size_bytes);
    ASSERT_EQ(str_send, str_recv);
    // Check the received timestamp is the expected one
    ASSERT_EQ(1234567, timestamp->epoch_s);
    ASSERT_EQ(8910, timestamp->offset_ns);

    // Checkpoint n~2
    SAY_SELF("Checkpoint n~2");
    TEST_SYNC();

    // Unload
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
}

uint32_t com_recv_counter;

TEST_P(StreamContext, Callbacks)
{
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    const void *sample;
    size_t sample_size;

    uint32_t user_data = 10;
    com_recv_counter = 0;
    ASSERT_EQ(ed247_component_set_user_data(_context, &user_data), ED247_STATUS_SUCCESS);
    auto callback = [](ed247_context_t context) -> ed247_status_t {
        void *user_data;
        ed247_component_get_user_data(context, &user_data);
        if(user_data){
            com_recv_counter += *(uint32_t*)user_data;
        }
        return ED247_STATUS_SUCCESS;
    };
    ASSERT_EQ(ed247_unregister_com_recv_callback(_context, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_register_com_recv_callback(NULL, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_register_com_recv_callback(_context, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_register_com_recv_callback(_context, callback), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_unregister_com_recv_callback(NULL, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_unregister_com_recv_callback(_context, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_unregister_com_recv_callback(_context, callback), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_register_com_recv_callback(_context, callback), ED247_STATUS_SUCCESS);

    // Checkpoint n~1
    SAY_SELF("Checkpoint n~1");
    TEST_SYNC();

    malloc_count_start();

    // Recv a single frame
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);

    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_pop_sample(stream, &sample, &sample_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
    
    ASSERT_EQ(malloc_count_stop(), 0);

    ASSERT_TRUE(com_recv_counter > 0);

    // Unload
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
}

std::vector<TestParams> stream_files;

INSTANTIATE_TEST_CASE_P(func_exchange_datatimestamp, StreamContext,
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

    tests_tools::display_ed247_lib_infos();
    SAY("Configuration path: " << config_path);

    stream_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_datatimestamp_a429_recv.xml"});

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
