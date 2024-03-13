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
#include "two_actors_test.h"
#define TEST_ACTOR_ID TEST_ACTOR2_ID


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
class SimpleStreamContext : public TestContext {};
class SignalContext : public TestContext {};

// Create two dummy timestamps and associated function
// that may be registered as recv_timestamp handlers
ed247_timestamp_t timestamp1 = {123, 456};
ed247_timestamp_t timestamp2 = {456, 789};

void get_time_test1(ed247_timestamp_t* timestamp) {
  *timestamp = timestamp1;
}

void get_time_test2(ed247_timestamp_t* timestamp) {
  *timestamp = timestamp2;
}

uint32_t checkpoints;
const char * stream_name;
uint32_t global_counter;
uint32_t recv_counter;

/******************************************************************************
This test case also specifically checks the behavior of the get_simulation_time
******************************************************************************/
TEST_P(StreamContext, SingleFrame)
{
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    uint32_t sample_max_size_bytes;
    uint32_t sample_max_number;
    const void *sample;
    uint32_t sample_size;
    bool empty;
    std::string str_send, str_recv;
    const ed247_timestamp_t* receive_timestamp;
    ed247_stream_t stream0, stream1;
    ed247_stream_recv_callback_t callback;

    // Self-Test consistency check
    ASSERT_NE(timestamp1.epoch_s, timestamp2.epoch_s);
    ASSERT_NE(timestamp1.offset_ns, timestamp2.offset_ns);

    // Checkpoint n~1
    SAY_SELF("Checkpoint n~1");
    TEST_SYNC();

    malloc_count_start();
    // Set a first receive handler to timestamp the received frames
    ed247_set_receive_timestamp_callback(&get_time_test1);

    // Recv a single frame
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    sample_max_size_bytes = ed247_stream_get_sample_max_size_bytes(stream);
    sample_max_number = ed247_stream_get_sample_max_number(stream);
    ASSERT_EQ(ed247_stream_pop_sample(stream, &sample, &sample_size, NULL, &receive_timestamp, NULL, &empty), ED247_STATUS_SUCCESS);

    ASSERT_EQ(malloc_count_stop(), 0);

    // Limit cases
    ASSERT_EQ(ed247_wait_frame(NULL, &streams, 10000000), ED247_STATUS_FAILURE);

    // Extract and check content of payload
    str_send = strize() << std::setw(sample_max_size_bytes) << std::setfill('0') << 1;
    str_recv = std::string((char*)sample, sample_max_size_bytes);
    ASSERT_EQ(str_send, str_recv);
    // Check the received timestamp is the expected one
    ASSERT_EQ(timestamp1.epoch_s, receive_timestamp->epoch_s);
    ASSERT_EQ(timestamp1.offset_ns, receive_timestamp->offset_ns);

    // Checkpoint n~2
    SAY_SELF("Checkpoint n~2");
    TEST_SYNC();

    // Receive multiple frames for 10 seconds
    malloc_count_start();
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    // Change the reception routine just after reception.
    // The timestamps of the already received frames shall not be changed
    ed247_set_receive_timestamp_callback(&get_time_test2);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    sample_max_size_bytes = ed247_stream_get_sample_max_size_bytes(stream);
    sample_max_number = ed247_stream_get_sample_max_number(stream);
    ASSERT_EQ(malloc_count_stop(), 0);
    for(unsigned i = 0 ; i < sample_max_number ; i++){
        // Extract and check content of payload for each frame
        str_send = strize() << std::setw(sample_max_size_bytes) << std::setfill('0') << i;
        malloc_count_start();
        ASSERT_EQ(ed247_stream_pop_sample(stream, &sample, &sample_size, NULL, &receive_timestamp, NULL, &empty), ED247_STATUS_SUCCESS);
        uint32_t stack_size = 0;
        ASSERT_EQ(ed247_stream_samples_number(stream, ED247_DIRECTION_IN, &stack_size), ED247_STATUS_SUCCESS);
        ASSERT_EQ(malloc_count_stop(), 0);
        SAY_SELF("Receive stack size [" << stack_size << "]");
        ASSERT_TRUE(i == (sample_max_number-1) ? empty : !empty);
        str_recv = std::string((char*)sample, sample_max_size_bytes);
        ASSERT_EQ(str_send, str_recv);
        // Check the received data still use the old timestamp
        ASSERT_EQ(timestamp1.epoch_s, receive_timestamp->epoch_s);
        ASSERT_EQ(timestamp1.offset_ns, receive_timestamp->offset_ns);
    }

    // Checkpoint n~3
    SAY_SELF("Checkpoint n~3");
    TEST_SYNC();

    // Receive other frames with the second handler
    malloc_count_start();
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    ASSERT_EQ(malloc_count_stop(), 0);
    while(ed247_stream_list_next(streams, &stream) == ED247_STATUS_SUCCESS && stream != NULL){
        malloc_count_start();
        sample_max_size_bytes = ed247_stream_get_sample_max_size_bytes(stream);
        sample_max_number = ed247_stream_get_sample_max_number(stream);
        ASSERT_EQ(malloc_count_stop(), 0);
        for(unsigned i = 0 ; i < sample_max_number ; i++){
            // Extract and check content of payload for each frame
            str_send = strize() << std::setw(sample_max_size_bytes) << std::setfill('0') << i;
            malloc_count_start();
            ASSERT_EQ(ed247_stream_pop_sample(stream, &sample, &sample_size, NULL, &receive_timestamp, NULL, &empty), ED247_STATUS_SUCCESS);
            uint32_t stack_size = 0;
            ASSERT_EQ(ed247_stream_samples_number(stream, ED247_DIRECTION_IN, &stack_size), ED247_STATUS_SUCCESS);
            ASSERT_EQ(malloc_count_stop(), 0);
            SAY_SELF("Receive stack size [" << stack_size << "]");
            ASSERT_TRUE(i == (sample_max_number-1) ? empty : !empty);
            str_recv = std::string((char*)sample, sample_max_size_bytes);
            ASSERT_EQ(str_send, str_recv);
            // Verify the new timestamp is now used
            ASSERT_EQ(timestamp2.epoch_s, receive_timestamp->epoch_s);
            ASSERT_EQ(timestamp2.offset_ns, receive_timestamp->offset_ns);
        }
    }

    // Checkpoint n~4
    SAY_SELF("Checkpoint n~4");
    TEST_SYNC();

    // Receive other frames with the second handler
    malloc_count_start();
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    ASSERT_EQ(malloc_count_stop(), 0);
    while(ed247_stream_list_next(streams, &stream) == ED247_STATUS_SUCCESS && stream != NULL){
        malloc_count_start();
        sample_max_size_bytes = ed247_stream_get_sample_max_size_bytes(stream);
        sample_max_number = ed247_stream_get_sample_max_number(stream);
        ASSERT_EQ(malloc_count_stop(), 0);
        for(unsigned i = 0 ; i < sample_max_number ; i++){
            // Extract and check content of payload for each frame
            str_send = strize() << std::setw(sample_max_size_bytes) << std::setfill('0') << i;
            malloc_count_start();
            ASSERT_EQ(ed247_stream_pop_sample(stream, &sample, &sample_size, NULL, &receive_timestamp, NULL, &empty), ED247_STATUS_SUCCESS);
            uint32_t stack_size = 0;
            ASSERT_EQ(ed247_stream_samples_number(stream, ED247_DIRECTION_IN, &stack_size), ED247_STATUS_SUCCESS);
            ASSERT_EQ(malloc_count_stop(), 0);
            SAY_SELF("Receive stack size [" << stack_size << "]");
            ASSERT_TRUE(i == (sample_max_number-1) ? empty : !empty);
            str_recv = std::string((char*)sample, sample_max_size_bytes);
            SAY_SELF("Received [" << str_recv << "]");
            ASSERT_EQ(str_send, str_recv);
            // Verify the new timestamp is now used
            ASSERT_EQ(timestamp2.epoch_s, receive_timestamp->epoch_s);
            ASSERT_EQ(timestamp2.offset_ns, receive_timestamp->offset_ns);
        }
    }

    // Checkpoint n~5.1
    SAY_SELF("Checkpoint n~5.1");
    TEST_SYNC();

    // Setup recv callback on a single stream, not all

    // Retrieve streams
    ASSERT_EQ(ed247_find_streams(_context, "Stream0", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream0), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);

    ASSERT_EQ(ed247_find_streams(_context, "Stream1", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream1), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);

    // Reset globals
    checkpoints = 0;
    stream_name = nullptr;
    uint64_t data = 123456789;
    void *puser_data;
    recv_counter = 0;
    ASSERT_EQ(ed247_component_set_user_data(NULL, (void*)&data), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_component_set_user_data(_context, (void*)&data), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_component_get_user_data(NULL, &puser_data), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_component_get_user_data(_context, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_component_get_user_data(_context, &puser_data), ED247_STATUS_SUCCESS);
    ASSERT_EQ(*(uint64_t*)puser_data, data);
    // Register the callback
    callback = [](ed247_context_t context, ed247_stream_t stream) -> ed247_status_t {
        void *user_data;
        ed247_component_get_user_data(context, &user_data);
        if(user_data && *(uint64_t*)user_data == 123456789){
            recv_counter++;
        }
        stream_name = ed247_stream_get_name(stream);
        SAY_SELF("Callback on stream [" << ed247_stream_get_name(stream) << "]");
        checkpoints++;
        return ED247_STATUS_SUCCESS;
    };
    ASSERT_EQ(ed247_stream_register_recv_callback(_context, stream0, callback), ED247_STATUS_SUCCESS);
    // Check limit cases
    ASSERT_EQ(ed247_stream_register_recv_callback(_context, stream0, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_register_recv_callback(NULL, stream0, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_register_recv_callback(_context, NULL, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_register_recv_callback(_context, stream0, NULL), ED247_STATUS_FAILURE);

    global_counter = 0;
    // ed247_com_callback_t recv_callback = []() -> ed247_status_t {
    //     global_counter++;
    // };
    // TODO: Continue

    // Checkpoint n~5.2
    SAY_SELF("Checkpoint n~5.2");
    TEST_SYNC();

    // Perform reception
    malloc_count_start();
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    ASSERT_EQ(malloc_count_stop(), 0);
    // Check that the callback was called
    ASSERT_TRUE(recv_counter > 0);
    ASSERT_EQ(checkpoints, (uint32_t)1); // Only once
    ASSERT_STREQ(stream_name, "Stream0"); // The stream shall be Stream0
    // Unregister the callback
    ASSERT_EQ(ed247_stream_unregister_recv_callback(_context, stream0, callback), ED247_STATUS_SUCCESS);
    // Check limit cases
    ASSERT_EQ(ed247_stream_unregister_recv_callback(_context, stream0, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_unregister_recv_callback(NULL, stream0, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_unregister_recv_callback(_context, NULL, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_unregister_recv_callback(_context, stream0, NULL), ED247_STATUS_FAILURE);
    recv_counter = 0;
    ASSERT_EQ(ed247_component_set_user_data(_context, NULL), ED247_STATUS_SUCCESS);

    // Checkpoint n~6.1
    SAY_SELF("Checkpoint n~6.1");
    TEST_SYNC();

    // Retrieve stream list
    ASSERT_EQ(ed247_find_streams(_context, "Stream0", &streams), ED247_STATUS_SUCCESS);
    // Reset globals
    checkpoints = 0;
    stream_name = nullptr;
    // Check limit cases
    ASSERT_EQ(ed247_stream_register_recv_callback(_context, stream0, callback), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_streams_register_recv_callback(_context, streams, callback), ED247_STATUS_FAILURE); // Already registered
    ASSERT_EQ(ed247_stream_unregister_recv_callback(_context, stream0, callback), ED247_STATUS_SUCCESS);
    // Register callback
    ASSERT_EQ(ed247_streams_register_recv_callback(_context, streams, callback), ED247_STATUS_SUCCESS);
    // Check limit cases
    ASSERT_EQ(ed247_streams_register_recv_callback(_context, streams, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_streams_register_recv_callback(_context, streams, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_streams_register_recv_callback(_context, NULL, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_streams_register_recv_callback(NULL, streams, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);

    // Checkpoint n~6.2
    SAY_SELF("Checkpoint n~6.2");
    TEST_SYNC();

    // Perform reception
    malloc_count_start();
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    ASSERT_EQ(malloc_count_stop(), 0);
    // Check that the callback was called
    ASSERT_EQ(recv_counter, 0);
    ASSERT_EQ(checkpoints, (uint32_t)1); // Only once
    ASSERT_STREQ(stream_name, "Stream0"); // The stream shall be Stream0
    // Unregister the callback
    ASSERT_EQ(ed247_find_streams(_context, "Stream0", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_streams_unregister_recv_callback(_context, streams, callback), ED247_STATUS_SUCCESS);
    // Check limit cases
    ASSERT_EQ(ed247_streams_unregister_recv_callback(_context, streams, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_streams_unregister_recv_callback(NULL, streams, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_streams_unregister_recv_callback(_context, NULL, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_streams_unregister_recv_callback(_context, streams, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);

    // Checkpoint n~7.1
    SAY_SELF("Checkpoint n~7.1");
    TEST_SYNC();

    // Reset globals
    checkpoints = 0;
    stream_name = nullptr;
    // Register callback
    ASSERT_EQ(ed247_register_recv_callback(_context, callback), ED247_STATUS_SUCCESS);
    // Check limit cases
    ASSERT_EQ(ed247_register_recv_callback(_context, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_register_recv_callback(NULL, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_register_recv_callback(_context, NULL), ED247_STATUS_FAILURE);

    // Checkpoint n~7.2
    SAY_SELF("Checkpoint n~7.2");
    TEST_SYNC();

    // Perform reception
    malloc_count_start();
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    ASSERT_EQ(malloc_count_stop(), 0);
    // Check that the callback was called
    ASSERT_EQ(checkpoints, (uint32_t)2);
    ASSERT_NE(stream_name, nullptr);
    // Unregister the callback
    ASSERT_EQ(ed247_unregister_recv_callback(_context, callback), ED247_STATUS_SUCCESS);
    // Check limit cases
    ASSERT_EQ(ed247_unregister_recv_callback(_context, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_unregister_recv_callback(NULL, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_unregister_recv_callback(_context, NULL), ED247_STATUS_FAILURE);

    // Checkpoint n~8
    SAY_SELF("Checkpoint n~8");
    TEST_SYNC();

    // Unload
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
}

TEST_P(SimpleStreamContext, SingleFrame)
{
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    uint32_t sample_max_size_bytes;
    uint32_t sample_max_number;
    const void *sample;
    uint32_t sample_size;
    bool empty;
    std::string str_send, str_recv;
    const ed247_timestamp_t* receive_timestamp;

    // Checkpoint n~1
    SAY_SELF("Checkpoint n~1");
    TEST_SYNC();

    // Set a dummy receive timestamp handler before reception
    malloc_count_start();
    ed247_set_receive_timestamp_callback(&get_time_test1);

    // Recv a first frame
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    sample_max_size_bytes = ed247_stream_get_sample_max_size_bytes(stream);
    sample_max_number = ed247_stream_get_sample_max_number(stream);
    ASSERT_EQ(ed247_stream_pop_sample(stream, &sample, &sample_size, NULL, &receive_timestamp, NULL, &empty), ED247_STATUS_SUCCESS);
    ASSERT_EQ(malloc_count_stop(), 0);

    // Extract and check the content of the received frame
    str_send = strize() << std::setw(sample_max_size_bytes) << std::setfill('0') << 1;
    str_recv = std::string((char*)sample, sample_max_size_bytes);
    ASSERT_EQ(str_send, str_recv);
    // Check the received timestamp is the expected one
    ASSERT_EQ(timestamp1.epoch_s, receive_timestamp->epoch_s);
    ASSERT_EQ(timestamp1.offset_ns, receive_timestamp->offset_ns);

    // Checkpoint n~2
    SAY_SELF("Checkpoint n~2");
    TEST_SYNC();

    // Wait for more frames to be received
    malloc_count_start();
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    // Change the reception routine after reception.
    // It shall not modify the recv timestamps of already received frames
    ed247_set_receive_timestamp_callback(&get_time_test2);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    sample_max_size_bytes = ed247_stream_get_sample_max_size_bytes(stream);
    sample_max_number = ed247_stream_get_sample_max_number(stream);
    ASSERT_EQ(malloc_count_stop(), 0);
    for(unsigned i = 0 ; i < sample_max_number ; i++){
        // Extract and check the frame contents
        str_send = strize() << std::setw(sample_max_size_bytes) << std::setfill('0') << i;
        malloc_count_start();
        ASSERT_EQ(ed247_stream_pop_sample(stream, &sample, &sample_size, NULL, &receive_timestamp, NULL, &empty), ED247_STATUS_SUCCESS);
        uint32_t stack_size = 0;
        ASSERT_EQ(ed247_stream_samples_number(stream, ED247_DIRECTION_IN, &stack_size), ED247_STATUS_SUCCESS);
        ASSERT_EQ(malloc_count_stop(), 0);
        SAY_SELF("Receive stack size [" << stack_size << "]");
        ASSERT_TRUE(i == (sample_max_number-1) ? empty : !empty);
        str_recv = std::string((char*)sample, sample_max_size_bytes);
        ASSERT_EQ(str_send, str_recv);
        // Check the received data still use the old timestamp
        ASSERT_EQ(timestamp1.epoch_s, receive_timestamp->epoch_s);
        ASSERT_EQ(timestamp1.offset_ns, receive_timestamp->offset_ns);
    }

    // Checkpoint n~3
    SAY_SELF("Checkpoint n~3");
    TEST_SYNC();

    // Wait for more frames to be received
    malloc_count_start();
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    ASSERT_EQ(malloc_count_stop(), 0);
    while(ed247_stream_list_next(streams, &stream) == ED247_STATUS_SUCCESS && stream != NULL){
        malloc_count_start();
        sample_max_size_bytes = ed247_stream_get_sample_max_size_bytes(stream);
        sample_max_number = ed247_stream_get_sample_max_number(stream);
        ASSERT_EQ(malloc_count_stop(), 0);
        for(unsigned i = 0 ; i < sample_max_number ; i++){
            // Extract and check the frame contents
            str_send = strize() << std::setw(sample_max_size_bytes) << std::setfill('0') << i;
            malloc_count_start();
            ASSERT_EQ(ed247_stream_pop_sample(stream, &sample, &sample_size, NULL, &receive_timestamp, NULL, &empty), ED247_STATUS_SUCCESS);
            uint32_t stack_size = 0;
            ASSERT_EQ(ed247_stream_samples_number(stream, ED247_DIRECTION_IN, &stack_size), ED247_STATUS_SUCCESS);
            ASSERT_EQ(malloc_count_stop(), 0);
            SAY_SELF("Receive stack size [" << stack_size << "]");
            ASSERT_TRUE(i == (sample_max_number-1) ? empty : !empty);
            str_recv = std::string((char*)sample, sample_max_size_bytes);
            ASSERT_EQ(str_send, str_recv);
            // Verify the new timestamp is now used
            ASSERT_EQ(timestamp2.epoch_s, receive_timestamp->epoch_s);
            ASSERT_EQ(timestamp2.offset_ns, receive_timestamp->offset_ns);
        }
    }

    // Checkpoint n~4
    SAY_SELF("Checkpoint n~4");
    TEST_SYNC();

    // Unload
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
}

TEST_P(StreamContext, MultipleFrame)
{
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    uint32_t sample_max_size_bytes;
    uint32_t sample_max_number;
    const void *sample;
    uint32_t sample_size;
    bool empty;
    std::string str_send, str_recv;
    const ed247_timestamp_t* receive_timestamp;

    // Checkpoint n~1
    SAY_SELF("Checkpoint n~1");
    TEST_SYNC();

    // Set a dummy receive timestamp handler before reception
    malloc_count_start();
    ed247_set_receive_timestamp_callback(&get_time_test1);

    // Recv frames
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    sample_max_size_bytes = ed247_stream_get_sample_max_size_bytes(stream);
    sample_max_number = ed247_stream_get_sample_max_number(stream);
    ASSERT_EQ(ed247_stream_pop_sample(stream, &sample, &sample_size, NULL, &receive_timestamp, NULL, &empty), ED247_STATUS_SUCCESS);
    ASSERT_EQ(malloc_count_stop(), 0);

    // Extract and check the content of this frame
    str_send = strize() << std::setw(sample_max_size_bytes) << std::setfill('0') << 0;
    str_recv = std::string((char*)sample, sample_max_size_bytes);
    ASSERT_EQ(str_send, str_recv);
    // Check the received timestamp is the expected one
    ASSERT_EQ(timestamp1.epoch_s, receive_timestamp->epoch_s);
    ASSERT_EQ(timestamp1.offset_ns, receive_timestamp->offset_ns);

    SAY_SELF("Checkpoint n~1.1");
    TEST_SYNC();

    // Recv the other frames
    ASSERT_EQ(ed247_wait_during(NULL, &streams, 1000000), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_wait_during(_context, &streams, 1000000), ED247_STATUS_SUCCESS);
    malloc_count_start();
    // Change the reception routine after reception.
    // It shall not modify the receive timestamps of the already received frames
    ed247_set_receive_timestamp_callback(&get_time_test2);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    sample_max_size_bytes = ed247_stream_get_sample_max_size_bytes(stream);
    sample_max_number = ed247_stream_get_sample_max_number(stream);
    ASSERT_EQ(malloc_count_stop(), 0);
    for(unsigned i = 1 ; i < sample_max_number ; i++){
        malloc_count_start();
        ASSERT_EQ(ed247_stream_pop_sample(stream, &sample, &sample_size, NULL, &receive_timestamp, NULL, &empty), ED247_STATUS_SUCCESS);
        ASSERT_EQ(malloc_count_stop(), 0);
        // Extract and check the content of the payload
        str_send = strize() << std::setw(sample_max_size_bytes) << std::setfill('0') << i;
        str_recv = std::string((char*)sample, sample_max_size_bytes);
        ASSERT_EQ(str_send, str_recv);
        // Check the received data still use the old timestamp
        ASSERT_EQ(timestamp1.epoch_s, receive_timestamp->epoch_s);
        ASSERT_EQ(timestamp1.offset_ns, receive_timestamp->offset_ns);
    }

    // Checkpoint n~2
    SAY_SELF("Checkpoint n~2");
    TEST_SYNC();

    // Unload
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
}

TEST_P(SignalContext, SingleFrame)
{
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    ed247_stream_assistant_t assistant;
    ed247_signal_list_t signals;
    ed247_signal_t signal;
    std::string signal_name;
    ed247_signal_type_t signal_type;
    bool empty;
    std::string str_send, str_recv;
    const ed247_timestamp_t* receive_timestamp;

    // Checkpoint n~1
    SAY_SELF("Checkpoint n~1");
    TEST_SYNC();

    // Set a dummy receive timestamp handler before reception
    malloc_count_start();
    ed247_set_receive_timestamp_callback(&get_time_test1);

    // Recv a frame containing signals
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_assistant(stream, &assistant), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_assistant_pop_sample(assistant, NULL, &receive_timestamp, NULL, &empty), ED247_STATUS_SUCCESS);
    ASSERT_EQ(malloc_count_stop(), 0);
    ASSERT_EQ(ed247_stream_get_signal_list(stream, &signals), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_assistant_pop_sample(NULL, NULL, &receive_timestamp, NULL, &empty), ED247_STATUS_FAILURE);
    while(ed247_signal_list_next(signals, &signal) == ED247_STATUS_SUCCESS && signal != NULL){
        signal_name = ed247_signal_get_name(signal);
        signal_type = ed247_signal_get_type(signal);
        const void * sample_data;
        uint32_t sample_size;
        malloc_count_start();
        // Extract and check the content of each signal of the frame
        ASSERT_EQ(ed247_stream_assistant_read_signal(assistant, signal, &sample_data, &sample_size), ED247_STATUS_SUCCESS);
        ASSERT_EQ(malloc_count_stop(), 0);
        if(signal_type == ED247_SIGNAL_TYPE_DISCRETE || signal_type == ED247_SIGNAL_TYPE_NAD){
            str_send = strize() << std::setw(sample_size) << std::setfill('0') << std::string(signal_name).substr(6,1);
        }else{
            str_send = strize() << std::setw(sample_size) << std::setfill('0') << std::string(signal_name).substr(6,2);
        }
        str_recv = std::string((char*)sample_data, sample_size);
        ASSERT_EQ(str_send, str_recv);
        // Check the received timestamp is the expected one
        ASSERT_EQ(timestamp1.epoch_s, receive_timestamp->epoch_s);
        ASSERT_EQ(timestamp1.offset_ns, receive_timestamp->offset_ns);
    }

    // Checkpoint n~2
    SAY_SELF("Checkpoint n~2");
    TEST_SYNC();

    // Unload
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
}

std::vector<TestParams> stream_files;

INSTANTIATE_TEST_CASE_P(Stream, StreamContext,
    ::testing::ValuesIn(stream_files));

std::vector<TestParams> simple_stream_files;

INSTANTIATE_TEST_CASE_P(SimpleStream, SimpleStreamContext,
    ::testing::ValuesIn(simple_stream_files));

std::vector<TestParams> signal_files;

INSTANTIATE_TEST_CASE_P(Signal, SignalContext,
    ::testing::ValuesIn(signal_files));

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

    stream_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_eth_uc_tester.xml"});
    stream_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_eth_mc_tester.xml"});
    stream_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_a429_uc_tester.xml"});
    stream_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_a429_mc_tester.xml"});
    stream_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_a664_mc_tester.xml"});
    stream_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_a825_uc_tester.xml"});
    stream_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_a825_mc_tester.xml"});
    stream_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_serial_uc_tester.xml"});
    stream_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_serial_mc_tester.xml"});

    simple_stream_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_a429_uc_tester_simple.xml"});
    simple_stream_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_a429_mc_tester_simple.xml"});

    signal_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_dis_mc_tester.xml"});
    signal_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_ana_mc_tester.xml"});
    signal_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_nad_mc_tester.xml"});
    signal_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_vnad_mc_tester.xml"});

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
