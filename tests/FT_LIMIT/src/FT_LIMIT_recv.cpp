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
#include <test_entity.h>

/***********
 * Defines *
 ***********/

#define TEST_ENTITY_SRC_ID 1
#define TEST_ENTITY_DST_ID 2

/********
 * Test *
 ********/
/******************************************************************************
This application is the receiver application for the high load test
******************************************************************************/

class TestLimitOut : public ::testing::Test{};

uint32_t stream_list_size(ed247_stream_list_t list)
{
    int32_t size = 0;
    ed247_stream_t first = NULL;
    ed247_stream_t current = NULL;
    ed247_stream_list_next(list, &first);
    while (current != first)
    {
        size++;
        ed247_stream_list_next(list, &current);
    }
    for (int32_t i = 0; i < size-1; i++)
    {
        ed247_stream_list_next(list, &current);
    }
    return (size > 0) ? size-1 : 0;
}

/******************************************************************************
This sequence receives at first frame per frame and then all frames at once
******************************************************************************/
TEST(InSequence, FtLimit)
{
    ed247_set_log_level(ED247_LOG_LEVEL_ERROR);
    test::Entity::init();
    test::Entity synchronizer(TEST_ENTITY_DST_ID);
    ed247_context_t context = NULL;
    ed247_stream_list_t streams = NULL;
    ed247_stream_list_t temp_list = NULL;
    ed247_stream_t stream = NULL;
    const ed247_stream_info_t* stream_info = NULL;
    std::string ecic_filename = "ft_limit_recv.xml";
    
    // Synchro at startup
    synchronizer.send(TEST_ENTITY_SRC_ID);
    ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_SRC_ID));
    
    // Initialisation of the receiver side
    uint64_t start = test::get_time_us();
    ASSERT_EQ(ed247_load(ecic_filename.c_str(), NULL, &context), ED247_STATUS_SUCCESS);
    uint64_t end = test::get_time_us();
    std::cout << "Loading time (us) = " << (end-start) << std::endl;
    
    // Synchro after receiver initialisation
    synchronizer.send(TEST_ENTITY_SRC_ID);
    ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_SRC_ID));
    
    // Synchro after sender initialisation
    synchronizer.send(TEST_ENTITY_SRC_ID);
    ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_SRC_ID, 20000000));
    
    ASSERT_EQ(ed247_component_get_streams(context, &streams), ED247_STATUS_SUCCESS);
    uint32_t number_of_streams = stream_list_size(streams);
    std::cout << "Stream size [" << number_of_streams << "]" << std::endl;
    
    // 1st step: transmit stream per stream
    start = test::get_time_us();
    
    for (uint32_t i = 0; i < number_of_streams; i++)
    {
        ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);
        ed247_wait_frame(context, &temp_list, 100000);
        // Anticipate the leaving of the loop !
        // This check/call dumps the receive performances !
        uint32_t number_received = stream_list_size(temp_list);
        if ( number_received == number_of_streams)
        {
            std::cout << "Break" << std::endl;
            break;
        }
    }
    while(ed247_stream_list_next(streams, &stream) == ED247_STATUS_SUCCESS && stream != NULL);
    ASSERT_EQ(stream_list_size(temp_list), number_of_streams);
    
    // Check all values in a second loop
    // ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    for (uint32_t i = 0; i < number_of_streams; i++)
    {
        size_t count = 0;
        const void* content = NULL;
        size_t content_size = 0;
        ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_samples_number(stream, ED247_DIRECTION_IN, &count), ED247_STATUS_SUCCESS);
        ASSERT_EQ(count, (size_t)1);
        ASSERT_EQ(ed247_stream_pop_sample(stream, &content, &content_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_EQ(content_size, (size_t)4);
        ASSERT_NE(content, (const void*) NULL);
        ASSERT_EQ(*((uint32_t*)content), number_of_streams%stream_info->uid);
    }
    
    end = test::get_time_us();
    std::cout << "Time necessary to receive one message after an other (us) = " << (end-start) << std::endl;
    
    // 2nd step: transmit all streams in one single call
    synchronizer.send(TEST_ENTITY_SRC_ID);
    ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_SRC_ID));
    
    start = test::get_time_us();
    ASSERT_EQ(ed247_wait_during(context, &temp_list, 1000*1000*10), ED247_STATUS_SUCCESS);
    
    // Check all values in the loop
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    for (uint32_t i = 0; i < number_of_streams; i++)
    {
        size_t count = 0;
        const void* content = NULL;
        size_t content_size = 0;
        ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_samples_number(stream, ED247_DIRECTION_IN, &count), ED247_STATUS_SUCCESS);
        ASSERT_EQ(count, (size_t)1);
        ASSERT_EQ(ed247_stream_pop_sample(stream, &content, &content_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_EQ(content_size, (size_t)4);
        ASSERT_NE(content, (const void*) NULL);
        ASSERT_EQ(*((uint32_t*)content), number_of_streams%stream_info->uid);
    }
    
    end = test::get_time_us();
    std::cout << "Time necessary to receive all message at once is lower than (us) = " << (end-start) << std::endl;
    
    synchronizer.send(TEST_ENTITY_SRC_ID);
    ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_SRC_ID));
    
    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
    
    synchronizer.send(TEST_ENTITY_SRC_ID);
    ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_SRC_ID));
}

/*************
 * Functions *
 *************/

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
