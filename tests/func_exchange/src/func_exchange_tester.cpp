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

#define TEST_ENTITY_SRC_ID 2
#define TEST_ENTITY_DST_ID 1

#define TEST_CONTEXT_SYNC_MAIN TestSend(); TestWait();
#define TEST_CONTEXT_SYNC_TESTER TestWait(); TestSend();

#define TEST_CONTEXT_SYNC TEST_CONTEXT_SYNC_TESTER

/********
 * Test *
 ********/

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

ed247_status_t get_time_test1(ed247_time_sample_t time_sample, void *user_data)
{
    _UNUSED(user_data);
    return libed247_update_time(time_sample, timestamp1.epoch_s, timestamp1.offset_ns);
}
ed247_status_t get_time_test2(ed247_time_sample_t time_sample, void *user_data)
{
    if(user_data){
        return libed247_update_time(time_sample, timestamp2.epoch_s+*(uint32_t*)user_data, timestamp2.offset_ns);
    }else{
        return libed247_update_time(time_sample, timestamp2.epoch_s, timestamp2.offset_ns);
    }
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
    const ed247_stream_info_t *stream_info;
    const void *sample;
    size_t sample_size;
    bool empty;
    std::string str_send, str_recv;
    std::ostringstream oss;
    const ed247_timestamp_t* frame_timestamp;
    ed247_stream_t stream0, stream1;
    ed247_stream_recv_callback_t callback;

    // Self-Test consistency check
    ASSERT_NE(timestamp1.epoch_s, timestamp2.epoch_s);
    ASSERT_NE(timestamp1.offset_ns, timestamp2.offset_ns);
    
    // Checkpoint n°1
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°1" << std::endl;
    TEST_CONTEXT_SYNC

    // Try to set an unvalid the reveice timestamp handler
    ASSERT_EQ(libed247_register_set_simulation_time_ns_handler(NULL, NULL), ED247_STATUS_FAILURE);

    memhooks_section_start();
    // Set a first receive handler to timestamp the received frames
    ASSERT_EQ(libed247_register_set_simulation_time_ns_handler(get_time_test1, NULL), ED247_STATUS_SUCCESS);

    // Recv a single frame
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_pop_sample(stream, &sample, &sample_size, NULL, &frame_timestamp, NULL, &empty), ED247_STATUS_SUCCESS);
    
    ASSERT_TRUE(memhooks_section_stop());

    // Limit cases
    ASSERT_EQ(ed247_wait_frame(NULL, &streams, 10000000), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_wait_frame(_context, NULL, 10000000), ED247_STATUS_FAILURE);
    
    // Extract and check content of payload
    oss.str("");
    oss << std::setw(stream_info->sample_max_size_bytes) << std::setfill('0') << 1;
    str_send = oss.str();
    str_recv = std::string((char*)sample, stream_info->sample_max_size_bytes);
    ASSERT_EQ(str_send, str_recv);
    // Check the received timestamp is the expected one
    ASSERT_EQ(timestamp1.epoch_s, frame_timestamp->epoch_s);
    ASSERT_EQ(timestamp1.offset_ns, frame_timestamp->offset_ns);

    // Checkpoint n°2
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°2" << std::endl;
    TEST_CONTEXT_SYNC

    // Receive multiple frames for 10 seconds
    memhooks_section_start();
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    // Change the reception routine just after reception.
    // The timestamps of the already received frames shall not be changed
    uint32_t user_data = 10;
    ASSERT_EQ(libed247_register_set_simulation_time_ns_handler(get_time_test2, &user_data), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(memhooks_section_stop());
    for(unsigned i = 0 ; i < stream_info->sample_max_number ; i++){
        // Extract and check content of payload for each frame
        oss.str("");
        oss << std::setw(stream_info->sample_max_size_bytes) << std::setfill('0') << i;
        str_send = oss.str();
        memhooks_section_start();
        ASSERT_EQ(ed247_stream_pop_sample(stream, &sample, &sample_size, NULL, &frame_timestamp, NULL, &empty), ED247_STATUS_SUCCESS);
        size_t stack_size = 0;
        ASSERT_EQ(ed247_stream_samples_number(stream, ED247_DIRECTION_IN, &stack_size), ED247_STATUS_SUCCESS);
        ASSERT_TRUE(memhooks_section_stop());
        std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Receive stack size [" << stack_size << "]" << std::endl;
        ASSERT_TRUE(i == (stream_info->sample_max_number-1) ? empty : !empty);
        str_recv = std::string((char*)sample, stream_info->sample_max_size_bytes);
        ASSERT_EQ(str_send, str_recv);
        // Check the received data still use the old timestamp
        ASSERT_EQ(timestamp1.epoch_s, frame_timestamp->epoch_s);
        ASSERT_EQ(timestamp1.offset_ns, frame_timestamp->offset_ns);
    }

    // Checkpoint n°3
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°3" << std::endl;
    TEST_CONTEXT_SYNC

    // Receive other frames with the second handler
    memhooks_section_start();
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(memhooks_section_stop());
    while(ed247_stream_list_next(streams, &stream) == ED247_STATUS_SUCCESS && stream != NULL){
        memhooks_section_start();
        ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);
        ASSERT_TRUE(memhooks_section_stop());
        for(unsigned i = 0 ; i < stream_info->sample_max_number ; i++){
            // Extract and check content of payload for each frame
            oss.str("");
            oss << std::setw(stream_info->sample_max_size_bytes) << std::setfill('0') << i;
            str_send = oss.str();
            memhooks_section_start();
            ASSERT_EQ(ed247_stream_pop_sample(stream, &sample, &sample_size, NULL, &frame_timestamp, NULL, &empty), ED247_STATUS_SUCCESS);
            size_t stack_size = 0;
            ASSERT_EQ(ed247_stream_samples_number(stream, ED247_DIRECTION_IN, &stack_size), ED247_STATUS_SUCCESS);
            ASSERT_TRUE(memhooks_section_stop());
            std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Receive stack size [" << stack_size << "]" << std::endl;
            ASSERT_TRUE(i == (stream_info->sample_max_number-1) ? empty : !empty);
            str_recv = std::string((char*)sample, stream_info->sample_max_size_bytes);
            ASSERT_EQ(str_send, str_recv);
            // Verify the new timestamp is now used
            ASSERT_EQ(timestamp2.epoch_s+user_data, frame_timestamp->epoch_s);
            ASSERT_EQ(timestamp2.offset_ns, frame_timestamp->offset_ns);
        }
    }

    // Checkpoint n°4
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°4" << std::endl;
    TEST_CONTEXT_SYNC

    // Receive other frames with the second handler
    memhooks_section_start();
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(memhooks_section_stop());
    while(ed247_stream_list_next(streams, &stream) == ED247_STATUS_SUCCESS && stream != NULL){
        memhooks_section_start();
        ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);
        ASSERT_TRUE(memhooks_section_stop());
        for(unsigned i = 0 ; i < stream_info->sample_max_number ; i++){
            // Extract and check content of payload for each frame
            oss.str("");
            oss << std::setw(stream_info->sample_max_size_bytes) << std::setfill('0') << i;
            str_send = oss.str();
            memhooks_section_start();
            ASSERT_EQ(ed247_stream_pop_sample(stream, &sample, &sample_size, NULL, &frame_timestamp, NULL, &empty), ED247_STATUS_SUCCESS);
            size_t stack_size = 0;
            ASSERT_EQ(ed247_stream_samples_number(stream, ED247_DIRECTION_IN, &stack_size), ED247_STATUS_SUCCESS);
            ASSERT_TRUE(memhooks_section_stop());
            std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Receive stack size [" << stack_size << "]" << std::endl;
            ASSERT_TRUE(i == (stream_info->sample_max_number-1) ? empty : !empty);
            str_recv = std::string((char*)sample, stream_info->sample_max_size_bytes);
            std::cout << "## Received [" << str_recv << "]" << std::endl;
            ASSERT_EQ(str_send, str_recv);
            // Verify the new timestamp is now used
            ASSERT_EQ(timestamp2.epoch_s+user_data, frame_timestamp->epoch_s);
            ASSERT_EQ(timestamp2.offset_ns, frame_timestamp->offset_ns);
        }
    }

    // Checkpoint n°5.1
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°5.1" << std::endl;
    TEST_CONTEXT_SYNC

    // Setup recv callback on a single stream, not all

    // Retrieve streams
    ASSERT_EQ(ed247_find_streams(_context, "Stream0", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream0), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_streams(_context, "Stream1", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream1), ED247_STATUS_SUCCESS);
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
        const ed247_stream_info_t *info;
        ed247_stream_get_info(stream, &info);
        stream_name = info->name;
        std::cout << "Callback on stream [" << stream_name << "]" << std::endl;
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

    // Checkpoint n°5.2
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°5.2" << std::endl;
    TEST_CONTEXT_SYNC

    // Perform reception
    memhooks_section_start();
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(memhooks_section_stop());
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

    // Checkpoint n°6.1
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°6.1" << std::endl;
    TEST_CONTEXT_SYNC

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

    // Checkpoint n°6.2
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°6.2" << std::endl;
    TEST_CONTEXT_SYNC

    // Perform reception
    memhooks_section_start();
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(memhooks_section_stop());
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

    // Checkpoint n°7.1
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°7.1" << std::endl;
    TEST_CONTEXT_SYNC

    // Reset globals
    checkpoints = 0;
    stream_name = nullptr;
    // Register callback
    ASSERT_EQ(ed247_register_recv_callback(_context, callback), ED247_STATUS_SUCCESS);
    // Check limit cases
    ASSERT_EQ(ed247_register_recv_callback(_context, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_register_recv_callback(NULL, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_register_recv_callback(_context, NULL), ED247_STATUS_FAILURE);

    // Checkpoint n°7.2
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°7.2" << std::endl;
    TEST_CONTEXT_SYNC

    // Perform reception
    memhooks_section_start();
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(memhooks_section_stop());
    // Check that the callback was called
    ASSERT_EQ(checkpoints, (uint32_t)2);
    ASSERT_STREQ(stream_name, "Stream1"); // The stream shall be Stream1
    // Unregister the callback
    ASSERT_EQ(ed247_unregister_recv_callback(_context, callback), ED247_STATUS_SUCCESS);
    // Check limit cases
    ASSERT_EQ(ed247_unregister_recv_callback(_context, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_unregister_recv_callback(NULL, callback), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_unregister_recv_callback(_context, NULL), ED247_STATUS_FAILURE);

    // Checkpoint n°8
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°8" << std::endl;
    TEST_CONTEXT_SYNC

    // Unload
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
}

TEST_P(SimpleStreamContext, SingleFrame)
{
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    const ed247_stream_info_t *stream_info;
    const void *sample;
    size_t sample_size;
    bool empty;
    std::string str_send, str_recv;
    std::ostringstream oss;
    const ed247_timestamp_t* frame_timestamp;

    // Checkpoint n°1
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°1" << std::endl;
    TEST_CONTEXT_SYNC

    // Set a dummy receive timestamp handler before reception
    memhooks_section_start();
    ASSERT_EQ(libed247_register_set_simulation_time_ns_handler(get_time_test1, NULL), ED247_STATUS_SUCCESS);

    // Recv a first frame
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_pop_sample(stream, &sample, &sample_size, NULL, &frame_timestamp, NULL, &empty), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(memhooks_section_stop());
    
    // Extract and check the content of the received frame
    oss.str("");
    oss << std::setw(stream_info->sample_max_size_bytes) << std::setfill('0') << 1;
    str_send = oss.str();
    str_recv = std::string((char*)sample, stream_info->sample_max_size_bytes);
    ASSERT_EQ(str_send, str_recv);
    // Check the received timestamp is the expected one
    ASSERT_EQ(timestamp1.epoch_s, frame_timestamp->epoch_s);
    ASSERT_EQ(timestamp1.offset_ns, frame_timestamp->offset_ns);

    // Checkpoint n°2
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°2" << std::endl;
    TEST_CONTEXT_SYNC

    // Wait for more frames to be received
    memhooks_section_start();
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    // Change the reception routine after reception.
    // It shall not modify the recv timestamps of already received frames
    ASSERT_EQ(libed247_register_set_simulation_time_ns_handler(get_time_test2, NULL), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(memhooks_section_stop());
    for(unsigned i = 0 ; i < stream_info->sample_max_number ; i++){
        // Extract and check the frame contents
        oss.str("");
        oss << std::setw(stream_info->sample_max_size_bytes) << std::setfill('0') << i;
        str_send = oss.str();
        memhooks_section_start();
        ASSERT_EQ(ed247_stream_pop_sample(stream, &sample, &sample_size, NULL, &frame_timestamp, NULL, &empty), ED247_STATUS_SUCCESS);
        size_t stack_size = 0;
        ASSERT_EQ(ed247_stream_samples_number(stream, ED247_DIRECTION_IN, &stack_size), ED247_STATUS_SUCCESS);
        ASSERT_TRUE(memhooks_section_stop());
        std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Receive stack size [" << stack_size << "]" << std::endl;
        ASSERT_TRUE(i == (stream_info->sample_max_number-1) ? empty : !empty);
        str_recv = std::string((char*)sample, stream_info->sample_max_size_bytes);
        ASSERT_EQ(str_send, str_recv);
        // Check the received data still use the old timestamp
        ASSERT_EQ(timestamp1.epoch_s, frame_timestamp->epoch_s);
        ASSERT_EQ(timestamp1.offset_ns, frame_timestamp->offset_ns);
    }

    // Checkpoint n°3
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°3" << std::endl;
    TEST_CONTEXT_SYNC

    // Wait for more frames to be received
    memhooks_section_start();
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(memhooks_section_stop());
    while(ed247_stream_list_next(streams, &stream) == ED247_STATUS_SUCCESS && stream != NULL){
        memhooks_section_start();
        ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);
        ASSERT_TRUE(memhooks_section_stop());
        for(unsigned i = 0 ; i < stream_info->sample_max_number ; i++){
            // Extract and check the frame contents
            oss.str("");
            oss << std::setw(stream_info->sample_max_size_bytes) << std::setfill('0') << i;
            str_send = oss.str();
            memhooks_section_start();
            ASSERT_EQ(ed247_stream_pop_sample(stream, &sample, &sample_size, NULL, &frame_timestamp, NULL, &empty), ED247_STATUS_SUCCESS);
            size_t stack_size = 0;
            ASSERT_EQ(ed247_stream_samples_number(stream, ED247_DIRECTION_IN, &stack_size), ED247_STATUS_SUCCESS);
            ASSERT_TRUE(memhooks_section_stop());
            std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Receive stack size [" << stack_size << "]" << std::endl;
            ASSERT_TRUE(i == (stream_info->sample_max_number-1) ? empty : !empty);
            str_recv = std::string((char*)sample, stream_info->sample_max_size_bytes);
            ASSERT_EQ(str_send, str_recv);
            // Verify the new timestamp is now used
            ASSERT_EQ(timestamp2.epoch_s, frame_timestamp->epoch_s);
            ASSERT_EQ(timestamp2.offset_ns, frame_timestamp->offset_ns);
        }
    }

    // Checkpoint n°4
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°4" << std::endl;
    TEST_CONTEXT_SYNC

    // Unload
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
}

TEST_P(StreamContext, MultipleFrame)
{
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    const ed247_stream_info_t *stream_info;
    const void *sample;
    size_t sample_size;
    bool empty;
    std::string str_send, str_recv;
    std::ostringstream oss;
    const ed247_timestamp_t* frame_timestamp;

    // Checkpoint n°1
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°1" << std::endl;
    TEST_CONTEXT_SYNC

    // Set a dummy receive timestamp handler before reception
    memhooks_section_start();
    ASSERT_EQ(libed247_register_set_simulation_time_ns_handler(get_time_test1, NULL), ED247_STATUS_SUCCESS);

    // Recv frames
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_pop_sample(stream, &sample, &sample_size, NULL, &frame_timestamp, NULL, &empty), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(memhooks_section_stop());
    
    // Extract and check the content of this frame
    oss.str("");
    oss << std::setw(stream_info->sample_max_size_bytes) << std::setfill('0') << 0;
    str_send = oss.str();
    str_recv = std::string((char*)sample, stream_info->sample_max_size_bytes);
    ASSERT_EQ(str_send, str_recv);
    // Check the received timestamp is the expected one
    ASSERT_EQ(timestamp1.epoch_s, frame_timestamp->epoch_s);
    ASSERT_EQ(timestamp1.offset_ns, frame_timestamp->offset_ns);

    // Recv the other frames
    ASSERT_EQ(ed247_wait_during(NULL, &streams, 1000000), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_wait_during(_context, NULL, 1000000), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_wait_during(_context, &streams, 1000000), ED247_STATUS_SUCCESS);
    memhooks_section_start();
    // Change the reception routine after reception.
    // It shall not modify the receive timestamps of the already received frames
    ASSERT_EQ(libed247_register_set_simulation_time_ns_handler(get_time_test2, NULL), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(memhooks_section_stop());
    for(unsigned i = 1 ; i < stream_info->sample_max_number ; i++){
        memhooks_section_start();
        ASSERT_EQ(ed247_stream_pop_sample(stream, &sample, &sample_size, NULL, &frame_timestamp, NULL, &empty), ED247_STATUS_SUCCESS);
        ASSERT_TRUE(memhooks_section_stop());
        // Extract and check the content of the payload
        oss.str("");
        oss << std::setw(stream_info->sample_max_size_bytes) << std::setfill('0') << i;
        str_send = oss.str();
        str_recv = std::string((char*)sample, stream_info->sample_max_size_bytes);
        ASSERT_EQ(str_send, str_recv);
        // Check the received data still use the old timestamp
        ASSERT_EQ(timestamp1.epoch_s, frame_timestamp->epoch_s);
        ASSERT_EQ(timestamp1.offset_ns, frame_timestamp->offset_ns);
    }

    // Checkpoint n°2
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°2" << std::endl;
    TEST_CONTEXT_SYNC

    // Unload
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
}

TEST_P(SignalContext, SingleFrame)
{
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    const ed247_stream_info_t *stream_info;
    ed247_stream_assistant_t assistant;
    ed247_signal_list_t signals;
    ed247_signal_t signal;
    const ed247_signal_info_t *signal_info;
    bool empty;
    std::string str_send, str_recv;
    std::ostringstream oss;
    const ed247_timestamp_t* frame_timestamp;

    // Checkpoint n°1
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°1" << std::endl;
    TEST_CONTEXT_SYNC

    // Set a dummy receive timestamp handler before reception
    memhooks_section_start();
    ASSERT_EQ(libed247_register_set_simulation_time_ns_handler(get_time_test1, NULL), ED247_STATUS_SUCCESS);

    // Recv a frame containing signals
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_assistant(stream, &assistant), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_assistant_pop_sample(assistant, NULL, &frame_timestamp, NULL, &empty), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_signals(stream, &signals), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(memhooks_section_stop());
    ASSERT_EQ(ed247_stream_assistant_pop_sample(NULL, NULL, &frame_timestamp, NULL, &empty), ED247_STATUS_FAILURE);
    while(ed247_signal_list_next(signals, &signal) == ED247_STATUS_SUCCESS && signal != NULL){
        ASSERT_EQ(ed247_signal_get_info(signal, &signal_info), ED247_STATUS_SUCCESS);
        const void * sample_data;
        size_t sample_size;
        memhooks_section_start();
        // Extract and check the content of each signal of the frame
        ASSERT_EQ(ed247_stream_assistant_read_signal(assistant, signal, &sample_data, &sample_size), ED247_STATUS_SUCCESS);
        ASSERT_TRUE(memhooks_section_stop());
        oss.str("");
        if(signal_info->type == ED247_SIGNAL_TYPE_DISCRETE || signal_info->type == ED247_SIGNAL_TYPE_NAD){
            oss << std::setw(sample_size) << std::setfill('0') << std::string(signal_info->name).substr(6,1);
        }else{
            oss << std::setw(sample_size) << std::setfill('0') << std::string(signal_info->name).substr(6,2);
        }
        str_send = oss.str();
        str_recv = std::string((char*)sample_data, sample_size);
        ASSERT_EQ(str_send, str_recv);
        // Check the received timestamp is the expected one
        ASSERT_EQ(timestamp1.epoch_s, frame_timestamp->epoch_s);
        ASSERT_EQ(timestamp1.offset_ns, frame_timestamp->offset_ns);
    }

    // Checkpoint n°2
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°2" << std::endl;
    TEST_CONTEXT_SYNC

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

    std::cout << "Configuration path: " << config_path << std::endl;

    stream_files.push_back({TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, config_path+"/ecic_func_exchange_a429_uc_tester.xml"});
    stream_files.push_back({TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, config_path+"/ecic_func_exchange_a429_mc_tester.xml"});
    stream_files.push_back({TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, config_path+"/ecic_func_exchange_a664_mc_tester.xml"});
    stream_files.push_back({TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, config_path+"/ecic_func_exchange_a825_uc_tester.xml"});
    stream_files.push_back({TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, config_path+"/ecic_func_exchange_a825_mc_tester.xml"});
    stream_files.push_back({TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, config_path+"/ecic_func_exchange_serial_uc_tester.xml"});
    stream_files.push_back({TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, config_path+"/ecic_func_exchange_serial_mc_tester.xml"});
    
    simple_stream_files.push_back({TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, config_path+"/ecic_func_exchange_a429_uc_tester_simple.xml"});
    simple_stream_files.push_back({TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, config_path+"/ecic_func_exchange_a429_mc_tester_simple.xml"});
    
    signal_files.push_back({TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, config_path+"/ecic_func_exchange_dis_mc_tester.xml"});
    signal_files.push_back({TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, config_path+"/ecic_func_exchange_ana_mc_tester.xml"});
    signal_files.push_back({TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, config_path+"/ecic_func_exchange_nad_mc_tester.xml"});
    signal_files.push_back({TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, config_path+"/ecic_func_exchange_vnad_mc_tester.xml"});

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
