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

#define TEST_ENTITY_SRC_ID 2
#define TEST_ENTITY_DST_ID 1

/********
 * Test *
 ********/
/******************************************************************************
This application is the receiver application for the runtime metrics test
******************************************************************************/

class TestMetricsIn : public ::testing::Test{};

/******************************************************************************
The application received frames following a predefined pattern and check
the library return the appropriate number of lost frames
******************************************************************************/
TEST(InSequence, UTMetrics)
{
    test::Entity::init();
    test::Entity synchronizer(TEST_ENTITY_DST_ID);
    const libed247_runtime_metrics_t* metrics;
    ed247_context_t context = NULL;
    ed247_stream_list_t streams = NULL;
    ed247_stream_list_t temp_list = NULL;
    ed247_stream_t stream = NULL;
    ed247_stream_t tmp_stream = NULL;
    ed247_stream_t second_stream = NULL;
    const ed247_stream_info_t* stream_info = NULL;
    ed247_stream_assistant_t assistant = NULL;
    ed247_signal_list_t signal_list = NULL;
    ed247_signal_t data_signal = NULL;
    std::string ecic_filename = CONFIG_PATH"/ut_api_metrics/ecic_recv.xml";
    
    // Chosen values are explained in the sender file as comment for header_values
    uint16_t expected_sn [] = {
        65000,
        65500,
        65535,
        10,
        65500};
    uint32_t expected_missed [] = {
        0,
        499,
        533,
        543,
        0xFFFF};
    uint16_t expected_sn_size = sizeof(expected_sn)/sizeof(expected_sn[0]);
    
    size_t count = 0;
    size_t data_size = 0;
    const uint16_t *data_buffer = NULL;
    
    ASSERT_EQ(ed247_load(ecic_filename.c_str(), NULL, &context), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_streams(context, "MyStream", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);
    ASSERT_NE(stream_info, (ed247_stream_info_t*)NULL);
    ASSERT_EQ(ed247_stream_get_assistant(stream, &assistant), ED247_STATUS_SUCCESS);
    ASSERT_NE(assistant, (ed247_stream_assistant_t)NULL);
    ASSERT_EQ(ed247_find_signals(context, "Data" , &signal_list), ED247_STATUS_SUCCESS);
    ASSERT_NE(signal_list, (ed247_signal_list_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signal_list, &data_signal), ED247_STATUS_SUCCESS);
    ASSERT_NE(data_signal, (ed247_signal_t)NULL);

    // Check limit cases
    ASSERT_EQ(ed247_stream_get_assistant(NULL, &assistant), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_get_assistant(stream, NULL), ED247_STATUS_FAILURE);

    ASSERT_EQ(ed247_stream_assistant_get_stream(assistant, &tmp_stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(tmp_stream, stream);
    // Check limit cases
    ASSERT_EQ(ed247_stream_assistant_get_stream(NULL, &tmp_stream), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_assistant_get_stream(assistant, NULL), ED247_STATUS_FAILURE);
    
    ASSERT_EQ(ed247_find_streams(context, "MySecondStream",&streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &second_stream), ED247_STATUS_SUCCESS);
    
    // Synchro after sender initialisation
    synchronizer.send(TEST_ENTITY_SRC_ID);
    ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_SRC_ID));
    
    for (uint32_t i = 0; i < expected_sn_size; i++)
    {
        std::cout << "Expecting frame " << i+1 << ": Simulated SN is " << expected_sn[i] << std::endl;
        memhooks_section_start();
        ASSERT_EQ(ed247_wait_frame(context, &temp_list, 1*1000*1000), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_samples_number(stream, ED247_DIRECTION_IN, &count), ED247_STATUS_SUCCESS);
        ASSERT_EQ(count, (size_t)1);
    
        ASSERT_EQ(ed247_stream_assistant_pop_sample(assistant, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_assistant_read_signal(assistant, data_signal, (const void**)(&data_buffer), &data_size), ED247_STATUS_SUCCESS);
        ASSERT_TRUE(memhooks_section_stop());
    
        ASSERT_EQ(data_size, (size_t)2);
        ASSERT_NE(data_buffer, (const uint16_t*)NULL);
        ASSERT_EQ(*data_buffer, (uint16_t)i);
    
        ASSERT_EQ(ed247_get_runtime_metrics(context, &metrics), ED247_STATUS_SUCCESS);
        ASSERT_EQ(metrics->missed_frames, expected_missed[i]);
    
        // Synchronization after each checks
        synchronizer.send(TEST_ENTITY_SRC_ID);
        ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_SRC_ID));
        
        // Double receive for the second stream even if not analysed
        uint32_t* sample_data = NULL;
        size_t sample_data_size = 0;
        ASSERT_EQ(ed247_wait_frame(context, &temp_list, 1*1000*1000), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_pop_sample(second_stream, (const void**)(&sample_data), &sample_data_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_pop_sample(NULL, (const void**)(&sample_data), &sample_data_size, NULL, NULL, NULL, NULL), ED247_STATUS_FAILURE);
        ASSERT_EQ(ed247_stream_pop_sample(second_stream, NULL, &sample_data_size, NULL, NULL, NULL, NULL), ED247_STATUS_FAILURE);
        ASSERT_EQ(ed247_stream_pop_sample(second_stream, (const void**)(&sample_data), NULL, NULL, NULL, NULL, NULL), ED247_STATUS_FAILURE);
        
        // Synchro after each send
        synchronizer.send(TEST_ENTITY_SRC_ID);
        ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_SRC_ID));
    }
    
    // Check the degraded cases
    ASSERT_EQ(ed247_get_runtime_metrics(NULL, &metrics), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_get_runtime_metrics(context, NULL), ED247_STATUS_FAILURE);
    
    // Synchronization after each checks
    synchronizer.send(TEST_ENTITY_SRC_ID);
    ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_SRC_ID));
    
    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
    
    synchronizer.send(TEST_ENTITY_SRC_ID);
    ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_SRC_ID));

}

/******************************************************************************
The application received frames following a predefined pattern and check
the library return the appropriate number of lost frames
******************************************************************************/
TEST(InSequence, UTMetricsCross)
{
    test::Entity::init();
    test::Entity synchronizer(TEST_ENTITY_DST_ID);
    const libed247_runtime_metrics_t* metrics;
    ed247_context_t context = NULL;
    ed247_stream_list_t streams = NULL;
    ed247_stream_list_t temp_list = NULL;
    ed247_stream_t stream = NULL;
    ed247_stream_t tmp_stream = NULL;
    ed247_stream_t second_stream = NULL;
    const ed247_stream_info_t* stream_info = NULL;
    ed247_stream_assistant_t assistant = NULL;
    ed247_signal_list_t signal_list = NULL;
    ed247_signal_t data_signal = NULL;
    std::string ecic_filename = CONFIG_PATH"/ut_api_metrics/ecic_recv.xml";

    uint16_t expected_sn_cross [2][4] = {
        {10, 15, 56, 413},
        {52, 86, 96, 555}
    };
    uint16_t expected_missed_cross [8] = {
        0, 0, 4, 37, 77, 86, 442, 900
    };
    
    size_t count = 0;
    size_t data_size = 0;
    const uint16_t *data_buffer = NULL;
    
    ASSERT_EQ(ed247_load(ecic_filename.c_str(), NULL, &context), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_streams(context, "MyStream", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);
    ASSERT_NE(stream_info, (ed247_stream_info_t*)NULL);
    ASSERT_EQ(ed247_stream_get_assistant(stream, &assistant), ED247_STATUS_SUCCESS);
    ASSERT_NE(assistant, (ed247_stream_assistant_t)NULL);
    ASSERT_EQ(ed247_find_signals(context, "Data" , &signal_list), ED247_STATUS_SUCCESS);
    ASSERT_NE(signal_list, (ed247_signal_list_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signal_list, &data_signal), ED247_STATUS_SUCCESS);
    ASSERT_NE(data_signal, (ed247_signal_t)NULL);

    // Check limit cases
    ASSERT_EQ(ed247_stream_get_assistant(NULL, &assistant), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_get_assistant(stream, NULL), ED247_STATUS_FAILURE);

    ASSERT_EQ(ed247_stream_assistant_get_stream(assistant, &tmp_stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(tmp_stream, stream);
    // Check limit cases
    ASSERT_EQ(ed247_stream_assistant_get_stream(NULL, &tmp_stream), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_assistant_get_stream(assistant, NULL), ED247_STATUS_FAILURE);
    
    ASSERT_EQ(ed247_find_streams(context, "MySecondStream",&streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &second_stream), ED247_STATUS_SUCCESS);
    
    // Synchro after sender initialisation
    synchronizer.send(TEST_ENTITY_SRC_ID);
    ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_SRC_ID));
    
    uint16_t counter = 0;
    for (uint16_t i = 0; i < 4; i++){
        for (uint16_t j = 0; j < 2; j++){
            std::cout << "Sending frame [" << counter << "] Simulated SN [" << expected_sn_cross[j][i] << "] for PID [" << j << "]" << std::endl;
            memhooks_section_start();
            ASSERT_EQ(ed247_wait_frame(context, &temp_list, 1*1000*1000), ED247_STATUS_SUCCESS);
            ASSERT_EQ(ed247_stream_samples_number(stream, ED247_DIRECTION_IN, &count), ED247_STATUS_SUCCESS);
            ASSERT_EQ(count, (size_t)1);
        
            ASSERT_EQ(ed247_stream_assistant_pop_sample(assistant, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
            ASSERT_EQ(ed247_stream_assistant_read_signal(assistant, data_signal, (const void**)(&data_buffer), &data_size), ED247_STATUS_SUCCESS);
            ASSERT_TRUE(memhooks_section_stop());
        
            ASSERT_EQ(data_size, (size_t)2);
            ASSERT_NE(data_buffer, (const uint16_t*)NULL);
            ASSERT_EQ(*data_buffer, (uint16_t)counter);
        
            ASSERT_EQ(ed247_get_runtime_metrics(context, &metrics), ED247_STATUS_SUCCESS);
            ASSERT_EQ(metrics->missed_frames, expected_missed_cross[counter]);
        
            // Synchronization after each checks
            synchronizer.send(TEST_ENTITY_SRC_ID);
            ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_SRC_ID));
            
            // Double receive for the second stream even if not analysed
            uint32_t* sample_data = NULL;
            size_t sample_data_size = 0;
            ASSERT_EQ(ed247_wait_frame(context, &temp_list, 1*1000*1000), ED247_STATUS_SUCCESS);
            ASSERT_EQ(ed247_stream_pop_sample(second_stream, (const void**)(&sample_data), &sample_data_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
            ASSERT_EQ(ed247_stream_pop_sample(NULL, (const void**)(&sample_data), &sample_data_size, NULL, NULL, NULL, NULL), ED247_STATUS_FAILURE);
            ASSERT_EQ(ed247_stream_pop_sample(second_stream, NULL, &sample_data_size, NULL, NULL, NULL, NULL), ED247_STATUS_FAILURE);
            ASSERT_EQ(ed247_stream_pop_sample(second_stream, (const void**)(&sample_data), NULL, NULL, NULL, NULL, NULL), ED247_STATUS_FAILURE);
            
            // Synchro after each send
            synchronizer.send(TEST_ENTITY_SRC_ID);
            ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_SRC_ID));
            counter++;
        }
    }
    
    // Check the degraded cases
    ASSERT_EQ(ed247_get_runtime_metrics(NULL, &metrics), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_get_runtime_metrics(context, NULL), ED247_STATUS_FAILURE);
    
    // Synchronization after each checks
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
