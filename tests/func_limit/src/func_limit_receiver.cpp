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
#define TEST_ACTOR_ID TEST_ACTOR2_ID
#include "functional_test.h"

std::string config_path = "../config";

/******************************************************************************
This application is the receiver application for the high load test
******************************************************************************/

class StreamContext : public TestContext {};

/******************************************************************************
This sequence receives at first frame per frame and then all frames at once
******************************************************************************/
TEST_P(StreamContext, LimitOneByOne)
{
    //ed247_set_log_level(ED247_LOG_LEVEL_ERROR);
    ed247_stream_list_t streams;
    ed247_stream_list_t temp_list;
    ed247_stream_t stream, tmp_stream;
    size_t size;
    
    // Synchro at startup
    SAY_SELF("Startup");
    TEST_SYNC();
    
    ASSERT_EQ(ed247_get_stream_list(_context, &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_size(streams, &size), ED247_STATUS_SUCCESS);
    // Cornercases
    ASSERT_EQ(ed247_stream_list_size(NULL, &size), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_list_size(streams, NULL), ED247_STATUS_FAILURE);
    SAY_SELF("Stream number [" << size << "]");
    
    uint64_t start = synchro::get_time_us();
    for (uint32_t i = 0; i < size; i++)
    {
        SAY_SELF("Loop [" << i << "/" << size << "]");
        size_t count = 0;
        const void* content = NULL;
        size_t content_size = 0;
        ASSERT_EQ(ed247_wait_frame(_context, &temp_list, 20*1000*1000), ED247_STATUS_SUCCESS);

        ASSERT_EQ(ed247_stream_list_next(temp_list, &tmp_stream), ED247_STATUS_SUCCESS);
        uint32_t expected_content = (uint32_t)ed247_stream_get_uid(tmp_stream);
        ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
        ASSERT_EQ(stream, tmp_stream);
        ASSERT_EQ(ed247_stream_samples_number(stream, ED247_DIRECTION_IN, &count), ED247_STATUS_SUCCESS);
        ASSERT_EQ(count, (size_t)1);
        ASSERT_EQ(ed247_stream_pop_sample(stream, &content, &content_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_EQ(content_size, (size_t)4);
        ASSERT_NE(content, (const void*) NULL);
        ASSERT_EQ(*((uint32_t*)content), expected_content);

        TEST_SYNC();
    }
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    
    uint64_t end = synchro::get_time_us();
    SAY_SELF("Receive & processing time (1 stream by 1 call) [" << (end-start)/1000 << "] ms");
    TEST_SYNC();

    // Unload
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
}

TEST_P(StreamContext, LimitAllInOne)
{
    //ed247_set_log_level(ED247_LOG_LEVEL_ERROR);
    ed247_stream_list_t streams;
    ed247_stream_list_t temp_list;
    ed247_stream_t stream, tmp_stream;
    size_t size;
    
    // Synchro at startup
    SAY_SELF("Startup");
    TEST_SYNC();
    
    ASSERT_EQ(ed247_get_stream_list(_context, &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_size(streams, &size), ED247_STATUS_SUCCESS);
    SAY_SELF("Stream number [" << size << "]");
    
    uint64_t start = synchro::get_time_us();
    ASSERT_EQ(ed247_wait_during(_context, &temp_list, 1000*1000*1), ED247_STATUS_SUCCESS);
    for (uint32_t i = 0; i < size; i++)
    {
        size_t count = 0;
        const void* content = NULL;
        size_t content_size = 0;
        ASSERT_EQ(ed247_stream_list_next(temp_list, &tmp_stream), ED247_STATUS_SUCCESS);
        uint32_t expected_content = (uint32_t)ed247_stream_get_uid(tmp_stream);
        ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
        ASSERT_EQ(stream, tmp_stream);
        ASSERT_EQ(ed247_stream_samples_number(stream, ED247_DIRECTION_IN, &count), ED247_STATUS_SUCCESS);
        ASSERT_EQ(count, (size_t)1);
        ASSERT_EQ(ed247_stream_pop_sample(stream, &content, &content_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_EQ(content_size, (size_t)4);
        ASSERT_NE(content, (const void*) NULL);
        ASSERT_EQ(*((uint32_t*)content), expected_content);
    }

    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    
    uint64_t end = synchro::get_time_us();
    SAY_SELF("Receive & processing time (all streams by 1 call) [" << (end-start)/1000 << "] ms");
    TEST_SYNC();

    // Unload
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
}

std::vector<TestParams> stream_files;

INSTANTIATE_TEST_CASE_P(FT_LIMIT, StreamContext,
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

    stream_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_limit_recv.xml"});

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
