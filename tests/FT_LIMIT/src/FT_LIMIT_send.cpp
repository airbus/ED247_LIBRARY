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
This application is the sender application for the high load test
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
This sequence sends at first frame per frame and then all frames at once
The preparation is always performed frame per frame
******************************************************************************/
TEST(OutSequence, FtLimit)
{
    ed247_set_log_level(ED247_LOG_LEVEL_ERROR);
    test::Entity::init();
    test::Entity synchronizer(TEST_ENTITY_SRC_ID);
    ed247_context_t context = NULL;
    ed247_stream_list_t streams = NULL;
    ed247_stream_t stream = NULL;
    const ed247_stream_info_t* stream_info = NULL;
    std::string ecic_filename = "ft_limit_send.xml";
    
    // Synchro at startup
    std::cout << "Startup" << std::endl;
    synchronizer.send(TEST_ENTITY_DST_ID);
    ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_DST_ID, 60000000));
    
    // Synchro after receiver initialisation
    std::cout << "Waiting for receiver init" << std::endl;
    synchronizer.send(TEST_ENTITY_DST_ID);
    ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_DST_ID, 60000000));
    
    std::cout << "Sender init" << std::endl;
    uint64_t start = test::get_time_us();
    ASSERT_EQ(ed247_load(ecic_filename.c_str(), NULL, &context), ED247_STATUS_SUCCESS);
    uint64_t end = test::get_time_us();
    std::cout << "Loading time (us) = " << (end-start) << std::endl;
    
    // Synchro after sender initialisation
    std::cout << "Sending sender init ok" << std::endl;
    synchronizer.send(TEST_ENTITY_DST_ID);
    ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_DST_ID, 60000000));
    
    ASSERT_EQ(ed247_component_get_streams(context, &streams), ED247_STATUS_SUCCESS);
    uint32_t number_of_streams = stream_list_size(streams);
    std::cout << "Stream size [" << number_of_streams << "]" << std::endl;
    
    // 1st step: transmit stream per stream
    start = test::get_time_us();
    for (uint32_t i = 0; i < number_of_streams; i++)
    {
        ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);
        uint32_t content = number_of_streams%stream_info->uid;
        ASSERT_EQ(ed247_stream_push_sample(stream, &content, sizeof(content), NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_send_pushed_samples(context), ED247_STATUS_SUCCESS);
    }
    
    end = test::get_time_us();
    std::cout << "Sending time one message at once (us) = " << (end-start) << std::endl;
    
    // 2nd step: transmit all streams in one single call
    synchronizer.send(TEST_ENTITY_DST_ID);
    ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_DST_ID));
    start = test::get_time_us();
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    for (uint32_t i = 0; i < number_of_streams; i++)
    {
        ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);
        uint32_t content = number_of_streams%stream_info->uid;
        ASSERT_EQ(ed247_stream_push_sample(stream, &content, sizeof(content), NULL, NULL), ED247_STATUS_SUCCESS);
    }
    ASSERT_EQ(ed247_send_pushed_samples(context), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_send_pushed_samples(NULL), ED247_STATUS_FAILURE);
    
    end = test::get_time_us();
    std::cout << "Sending all messages at once (us) = " << (end-start) << std::endl;
    
    synchronizer.send(TEST_ENTITY_DST_ID);
    ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_DST_ID));
    
    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
    
    synchronizer.send(TEST_ENTITY_DST_ID);
    ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_DST_ID));
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
