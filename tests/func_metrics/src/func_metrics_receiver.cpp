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

class Context : public TestContext {};

/******************************************************************************
The application received frames following a predefined pattern and check
the library return the appropriate number of lost frames
******************************************************************************/
TEST_P(Context, Metrics)
{
    ed247_stream_list_t streams = NULL;
    ed247_stream_list_t temp_list = NULL;
    ed247_stream_t stream = NULL;
    ed247_stream_t tmp_stream = NULL;
    ed247_stream_t second_stream = NULL;
    ed247_stream_assistant_t assistant = NULL;
    ed247_signal_list_t signal_list = NULL;
    ed247_signal_t data_signal = NULL;

    // Chosen values are explained in the sender file as comment for header_values
    uint16_t expected_sn [] = {
        65000,
        65500,
        65535,
        10,
        65500};
    uint16_t expected_sn_size = 5;

    uint32_t count = 0;
    uint32_t data_size = 0;
    const uint16_t *data_buffer = NULL;

    ASSERT_EQ(ed247_find_streams(_context, "MyStream", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_assistant(stream, &assistant), ED247_STATUS_SUCCESS);
    ASSERT_NE(assistant, (ed247_stream_assistant_t)NULL);
    ASSERT_EQ(ed247_find_signals(_context, "Data" , &signal_list), ED247_STATUS_SUCCESS);
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

    ASSERT_EQ(ed247_find_streams(_context, "MySecondStream",&streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &second_stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_assistant(second_stream, &assistant), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_get_assistant(stream, &assistant), ED247_STATUS_SUCCESS);

    // Checkpoint n~1
    SAY_SELF("Checkpoint n~1");
    TEST_SYNC();

    // Check limit case
    ASSERT_EQ(ed247_stream_assistant_read_signal(NULL, data_signal, (const void**)(&data_buffer), &data_size), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_assistant_read_signal(assistant, NULL, (const void**)(&data_buffer), &data_size), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_assistant_read_signal(assistant, data_signal, NULL, &data_size), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_assistant_read_signal(assistant, data_signal, (const void**)(&data_buffer), NULL), ED247_STATUS_FAILURE);

    ASSERT_EQ(ed247_stream_samples_number(NULL, ED247_DIRECTION_IN, &count), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_samples_number(stream, ED247_DIRECTION__INVALID, &count), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_samples_number(stream, ED247_DIRECTION_IN, NULL), ED247_STATUS_FAILURE);

    for (uint32_t i = 0; i < expected_sn_size; i++){

        SAY_SELF("Expecting frame " << i+1 << ": Simulated SN is " << expected_sn[i]);
        malloc_count_start();
        ASSERT_EQ(ed247_wait_frame(_context, &temp_list, 1*1000*1000), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_samples_number(stream, ED247_DIRECTION_IN, &count), ED247_STATUS_SUCCESS);
        ASSERT_EQ(count, (uint32_t)1);

        ASSERT_EQ(ed247_stream_assistant_pop_sample(assistant, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_assistant_read_signal(assistant, data_signal, (const void**)(&data_buffer), &data_size), ED247_STATUS_SUCCESS);
        ASSERT_EQ(malloc_count_stop(), 0);

        ASSERT_EQ(data_size, (uint32_t)2);
        ASSERT_NE(data_buffer, (const uint16_t*)NULL);
        ASSERT_EQ(*data_buffer, (uint16_t)i);

        // Checkpoint n~2
        SAY_SELF("Checkpoint n~2");
        TEST_SYNC();

        // Double receive for the second stream even if not analysed
        uint32_t* sample_data = NULL;
        uint32_t sample_data_size = 0;
        ASSERT_EQ(ed247_wait_frame(_context, &temp_list, 1*1000*1000), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_pop_sample(second_stream, (const void**)(&sample_data), &sample_data_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_pop_sample(NULL, (const void**)(&sample_data), &sample_data_size, NULL, NULL, NULL, NULL), ED247_STATUS_FAILURE);
        ASSERT_EQ(ed247_stream_pop_sample(second_stream, NULL, &sample_data_size, NULL, NULL, NULL, NULL), ED247_STATUS_FAILURE);
        ASSERT_EQ(ed247_stream_pop_sample(second_stream, (const void**)(&sample_data), NULL, NULL, NULL, NULL, NULL), ED247_STATUS_FAILURE);

        // Checkpoint n~3
        SAY_SELF("Checkpoint n~3");
        TEST_SYNC();
    }

    // Checkpoint n~4
    SAY_SELF("Checkpoint n~4");
    TEST_SYNC();

    // Unload
    // ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
    // ASSERT_EQ(ed247_stream_list_free(temp_list), ED247_STATUS_SUCCESS);
    // ASSERT_EQ(ed247_signal_list_free(signal_list), ED247_STATUS_SUCCESS);
}

/******************************************************************************
The application received frames following a predefined pattern and check
the library return the appropriate number of lost frames
******************************************************************************/
TEST_P(Context, MetricsCross)
{
    ed247_stream_list_t streams = NULL;
    ed247_stream_list_t temp_list = NULL;
    ed247_stream_t stream = NULL;
    ed247_stream_t tmp_stream = NULL;
    ed247_stream_t second_stream = NULL;
    ed247_stream_assistant_t assistant = NULL;
    ed247_signal_list_t signal_list = NULL;
    ed247_signal_t data_signal = NULL;
    const ed247_sample_details_t *sample_details;

    uint16_t expected_sn_cross [2][4] = {
        {10, 15, 56, 413},
        {52, 86, 96, 555}
    };

    uint32_t count = 0;
    uint32_t data_size = 0;
    const uint16_t *data_buffer = NULL;

    ASSERT_EQ(ed247_find_streams(_context, "MyStream", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_assistant(stream, &assistant), ED247_STATUS_SUCCESS);
    ASSERT_NE(assistant, (ed247_stream_assistant_t)NULL);
    ASSERT_EQ(ed247_find_signals(_context, "Data" , &signal_list), ED247_STATUS_SUCCESS);
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

    ASSERT_EQ(ed247_find_streams(_context, "MySecondStream",&streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &second_stream), ED247_STATUS_SUCCESS);

    // Checkpoint n~1
    SAY_SELF("Checkpoint n~1");
    TEST_SYNC();

    uint16_t counter = 0;
    for (uint16_t i = 0; i < 4; i++){
        for (uint16_t j = 0; j < 2; j++){
            SAY_SELF("Sending frame [" << counter << "] Simulated SN [" << expected_sn_cross[j][i] << "] for PID [" << j << "]");
            malloc_count_start();
            ASSERT_EQ(ed247_wait_frame(_context, &temp_list, 1*1000*1000), ED247_STATUS_SUCCESS);
            ASSERT_EQ(ed247_stream_samples_number(stream, ED247_DIRECTION_IN, &count), ED247_STATUS_SUCCESS);
            ASSERT_EQ(count, (uint32_t)1);

            ASSERT_EQ(ed247_stream_assistant_pop_sample(assistant, NULL, NULL, &sample_details, NULL), ED247_STATUS_SUCCESS);
            ASSERT_EQ(sample_details->transport_timestamp.epoch_s,(uint16_t)10);
            ASSERT_EQ(sample_details->transport_timestamp.offset_ns,(uint16_t)12);
            ASSERT_EQ(ed247_stream_assistant_read_signal(assistant, data_signal, (const void**)(&data_buffer), &data_size), ED247_STATUS_SUCCESS);
            ASSERT_EQ(malloc_count_stop(), 0);

            ASSERT_EQ(data_size, (uint32_t)2);
            ASSERT_NE(data_buffer, (const uint16_t*)NULL);
            ASSERT_EQ(*data_buffer, (uint16_t)counter);

            // Checkpoint n~2
            SAY_SELF("Checkpoint n~2");
            TEST_SYNC();

            // Double receive for the second stream even if not analysed
            uint32_t* sample_data = NULL;
            uint32_t sample_data_size = 0;
            ASSERT_EQ(ed247_wait_frame(_context, &temp_list, 1*1000*1000), ED247_STATUS_SUCCESS);
            ASSERT_EQ(ed247_stream_pop_sample(second_stream, (const void**)(&sample_data), &sample_data_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
            ASSERT_EQ(ed247_stream_pop_sample(NULL, (const void**)(&sample_data), &sample_data_size, NULL, NULL, NULL, NULL), ED247_STATUS_FAILURE);
            ASSERT_EQ(ed247_stream_pop_sample(second_stream, NULL, &sample_data_size, NULL, NULL, NULL, NULL), ED247_STATUS_FAILURE);
            ASSERT_EQ(ed247_stream_pop_sample(second_stream, (const void**)(&sample_data), NULL, NULL, NULL, NULL, NULL), ED247_STATUS_FAILURE);

            // Checkpoint n~3
            SAY_SELF("Checkpoint n~3");
            TEST_SYNC();

            counter++;
        }
    }

    // Checkpoint n~4
    SAY_SELF("Checkpoint n~4");
    TEST_SYNC();

    // Unload
    // ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
    // ASSERT_EQ(ed247_stream_list_free(temp_list), ED247_STATUS_SUCCESS);
    // ASSERT_EQ(ed247_signal_list_free(signal_list), ED247_STATUS_SUCCESS);
}

std::vector<TestParams> ecic_files;

INSTANTIATE_TEST_CASE_P(FuncMetrics, Context,
    ::testing::ValuesIn(ecic_files));

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

    ecic_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_metrics_recv.xml"});

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
