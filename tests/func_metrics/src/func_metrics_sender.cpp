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
#define TEST_ACTOR_ID TEST_ACTOR1_ID
#include "functional_test.h"

std::string config_path = "../config";

/******************************************************************************
This application is the sender application for the runtime metrics test
******************************************************************************/

class Context : public TestContext {};

/******************************************************************************
This sequence sends frames that do not contain any sequence number
******************************************************************************/
TEST_P(Context, Metrics)
{
    ed247_stream_list_t streams = NULL;
    ed247_stream_t stream = NULL;
    ed247_stream_t second_stream = NULL;
    void *second_stream_value;
    size_t second_stream_size;
    ed247_stream_assistant_t assistant = NULL;
    ed247_signal_list_t signals = NULL;
    ed247_signal_t dummy_header_sn = NULL;
    void *sn_value;
    size_t sn_size;
    ed247_signal_t dummy_header_pid = NULL;
    void *pid_value;
    size_t pid_size;
    ed247_signal_t dummy_header_tts1 = NULL;
    void *tts1_value;
    size_t tts1_size;
    ed247_signal_t dummy_header_tts2 = NULL;
    void *tts2_value;
    size_t tts2_size;
    ed247_signal_t data_signal = NULL;
    void *data_value;
    size_t data_size;
    
    uint16_t header_values [] = {
        65000,
        65500,
        65535,
        10,
        65500};
    uint16_t header_values_size = 5;
    
    ASSERT_EQ(ed247_find_streams(_context, "MyStream",&streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_assistant(stream, &assistant), ED247_STATUS_SUCCESS);
    ASSERT_NE(assistant, (ed247_stream_assistant_t)NULL);
    ASSERT_EQ(ed247_find_signals(_context, "DummyHeaderPID" , &signals), ED247_STATUS_SUCCESS);
    ASSERT_NE(signals, (ed247_signal_list_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signals, &dummy_header_pid), ED247_STATUS_SUCCESS);
    ASSERT_NE(dummy_header_pid, (ed247_signal_t)NULL);
    ASSERT_EQ(ed247_signal_allocate_sample(dummy_header_pid, &pid_value, &pid_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_signals(_context, "DummyHeaderSN" , &signals), ED247_STATUS_SUCCESS);
    ASSERT_NE(signals, (ed247_signal_list_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signals, &dummy_header_sn), ED247_STATUS_SUCCESS);
    ASSERT_NE(dummy_header_sn, (ed247_signal_t)NULL);
    ASSERT_EQ(ed247_signal_allocate_sample(dummy_header_pid, &sn_value, &sn_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_signals(_context, "DummyHeaderTTS1" , &signals), ED247_STATUS_SUCCESS);
    ASSERT_NE(signals, (ed247_signal_list_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signals, &dummy_header_tts1), ED247_STATUS_SUCCESS);
    ASSERT_NE(dummy_header_tts1, (ed247_signal_t)NULL);
    ASSERT_EQ(ed247_signal_allocate_sample(dummy_header_tts1, &tts1_value, &tts1_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_signals(_context, "DummyHeaderTTS2" , &signals), ED247_STATUS_SUCCESS);
    ASSERT_NE(signals, (ed247_signal_list_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signals, &dummy_header_tts2), ED247_STATUS_SUCCESS);
    ASSERT_NE(dummy_header_tts2, (ed247_signal_t)NULL);
    ASSERT_EQ(ed247_signal_allocate_sample(dummy_header_tts2, &tts2_value, &tts2_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_signals(_context, "Data" , &signals), ED247_STATUS_SUCCESS);
    ASSERT_NE(signals, (ed247_signal_list_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signals, &data_signal), ED247_STATUS_SUCCESS);
    ASSERT_NE(data_signal, (ed247_signal_t)NULL);
    ASSERT_EQ(ed247_signal_allocate_sample(data_signal, &data_value, &data_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_signal_list_free(signals), ED247_STATUS_SUCCESS);
    // uint16_t data = 0x0000;
    
    ASSERT_EQ(ed247_find_streams(_context, "MySecondStream",&streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &second_stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_allocate_sample(second_stream, &second_stream_value, &second_stream_size), ED247_STATUS_SUCCESS);
    
    // Checkpoint n~1
    SAY_SELF("Checkpoint n~1");
    TEST_SYNC();
    
    // Check limit cases
    ASSERT_EQ(ed247_stream_assistant_write_signal(NULL, dummy_header_pid, pid_value, pid_size), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, NULL, pid_value, pid_size), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, dummy_header_pid, NULL, pid_size), ED247_STATUS_FAILURE);

    for (uint16_t i = 0; i < header_values_size; i++){

        *(uint16_t*)pid_value = (uint16_t)0;
        *(uint16_t*)sn_value = (uint16_t)(header_values[i]);
        *(uint32_t*)tts1_value = (uint32_t)10;
        *(uint32_t*)tts2_value = (uint32_t)12;
        *(uint16_t*)data_value = i;
        SAY_SELF("Sending frame " << i+1 << ": Simulated SN is " << header_values[i]);
        ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, dummy_header_pid, pid_value, pid_size), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, dummy_header_sn, sn_value, sn_size), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, dummy_header_tts1, tts1_value, tts1_size), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, dummy_header_tts2, tts2_value, tts2_size), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, data_signal, data_value, data_size), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_assistant_push_sample(NULL, NULL, NULL), ED247_STATUS_FAILURE);
        ASSERT_EQ(ed247_stream_assistant_push_sample(assistant, NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
        
        // Checkpoint n~2
        SAY_SELF("Checkpoint n~2");
        TEST_SYNC();
        
        // Send the second stream to check the runtime metrics is not disturbed
        *(uint32_t*)second_stream_value = 0x12345678;
        ed247_stream_push_sample(second_stream, second_stream_value, second_stream_size, NULL, NULL);
        ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
        
        // Checkpoint n~3
        SAY_SELF("Checkpoint n~3");
        TEST_SYNC();
    }
    
    // Checkpoint n~4
    SAY_SELF("Checkpoint n~4");
    TEST_SYNC();

    // Unload
    // ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
    // ASSERT_EQ(ed247_signal_list_free(signals), ED247_STATUS_SUCCESS);
    // free(pid_value);
    // free(sn_value);
    // free(tts1_value);
    // free(tts2_value);
    // free(data_value);
    // free(second_stream_value);
}

/******************************************************************************
This sequence sends frames that do not contain any sequence number
******************************************************************************/
TEST_P(Context, MetricsCross)
{
    ed247_stream_list_t streams = NULL;
    ed247_stream_t stream = NULL;
    ed247_stream_t second_stream = NULL;
    void *second_stream_value;
    size_t second_stream_size;
    ed247_stream_assistant_t assistant = NULL;
    ed247_signal_list_t signals = NULL;
    ed247_signal_t dummy_header_sn = NULL;
    void *sn_value;
    size_t sn_size;
    ed247_signal_t dummy_header_pid = NULL;
    void *pid_value;
    size_t pid_size;
    ed247_signal_t dummy_header_tts1 = NULL;
    void *tts1_value;
    size_t tts1_size;
    ed247_signal_t dummy_header_tts2 = NULL;
    void *tts2_value;
    size_t tts2_size;
    ed247_signal_t data_signal = NULL;
    void *data_value;
    size_t data_size;

    uint16_t header_values_cross [2][4] = {
        {10, 15, 56, 413},
        {52, 86, 96, 555}
    };
    
    ASSERT_EQ(ed247_find_streams(_context, "MyStream",&streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_assistant(stream, &assistant), ED247_STATUS_SUCCESS);
    ASSERT_NE(assistant, (ed247_stream_assistant_t)NULL);
    ASSERT_EQ(ed247_find_signals(_context, "DummyHeaderPID" , &signals), ED247_STATUS_SUCCESS);
    ASSERT_NE(signals, (ed247_signal_list_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signals, &dummy_header_pid), ED247_STATUS_SUCCESS);
    ASSERT_NE(dummy_header_pid, (ed247_signal_t)NULL);
    ASSERT_EQ(ed247_signal_allocate_sample(dummy_header_pid, &pid_value, &pid_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_signals(_context, "DummyHeaderSN" , &signals), ED247_STATUS_SUCCESS);
    ASSERT_NE(signals, (ed247_signal_list_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signals, &dummy_header_sn), ED247_STATUS_SUCCESS);
    ASSERT_NE(dummy_header_sn, (ed247_signal_t)NULL);
    ASSERT_EQ(ed247_signal_allocate_sample(dummy_header_pid, &sn_value, &sn_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_signals(_context, "DummyHeaderTTS1" , &signals), ED247_STATUS_SUCCESS);
    ASSERT_NE(signals, (ed247_signal_list_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signals, &dummy_header_tts1), ED247_STATUS_SUCCESS);
    ASSERT_NE(dummy_header_tts1, (ed247_signal_t)NULL);
    ASSERT_EQ(ed247_signal_allocate_sample(dummy_header_tts1, &tts1_value, &tts1_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_signals(_context, "DummyHeaderTTS2" , &signals), ED247_STATUS_SUCCESS);
    ASSERT_NE(signals, (ed247_signal_list_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signals, &dummy_header_tts2), ED247_STATUS_SUCCESS);
    ASSERT_NE(dummy_header_tts2, (ed247_signal_t)NULL);
    ASSERT_EQ(ed247_signal_allocate_sample(dummy_header_tts2, &tts2_value, &tts2_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_signals(_context, "Data" , &signals), ED247_STATUS_SUCCESS);
    ASSERT_NE(signals, (ed247_signal_list_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signals, &data_signal), ED247_STATUS_SUCCESS);
    ASSERT_NE(data_signal, (ed247_signal_t)NULL);
    ASSERT_EQ(ed247_signal_allocate_sample(data_signal, &data_value, &data_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_signal_list_free(signals), ED247_STATUS_SUCCESS);
    // uint16_t data = 0x0000;
    
    ASSERT_EQ(ed247_find_streams(_context, "MySecondStream",&streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &second_stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_allocate_sample(second_stream, &second_stream_value, &second_stream_size), ED247_STATUS_SUCCESS);
    
    // Checkpoint n~1
    SAY_SELF("Checkpoint n~1");
    TEST_SYNC();
    
    uint16_t counter = 0;
    for (uint16_t i = 0; i < 4; i++){
        for (uint16_t j = 0; j < 2; j++){
            
            *(uint16_t*)pid_value = (uint16_t)j;
            *(uint16_t*)sn_value = (uint16_t)(header_values_cross[j][i]);
            *(uint32_t*)tts1_value = (uint32_t)10;
            *(uint32_t*)tts2_value = (uint32_t)12;
            *(uint16_t*)data_value = counter;
            SAY_SELF("Sending frame [" << counter << "] Simulated SN [" << header_values_cross[j][i] << "] for PID [" << j << "]");
            ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, dummy_header_pid, pid_value, pid_size), ED247_STATUS_SUCCESS);
            ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, dummy_header_sn, sn_value, sn_size), ED247_STATUS_SUCCESS);
            ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, dummy_header_tts1, tts1_value, tts1_size), ED247_STATUS_SUCCESS);
            ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, dummy_header_tts2, tts2_value, tts2_size), ED247_STATUS_SUCCESS);
            ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, data_signal, data_value, data_size), ED247_STATUS_SUCCESS);
            ASSERT_EQ(ed247_stream_assistant_push_sample(assistant, NULL, NULL), ED247_STATUS_SUCCESS);
            ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
            
            // Checkpoint n~2
            SAY_SELF("Checkpoint n~2");
            TEST_SYNC();
            
            // Send the second stream to check the runtime metrics is not disturbed
            *(uint32_t*)second_stream_value = 0x12345678;
            ed247_stream_push_sample(second_stream, second_stream_value, second_stream_size, NULL, NULL);
            ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
            
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
    // ASSERT_EQ(ed247_signal_list_free(signals), ED247_STATUS_SUCCESS);
    // free(pid_value);
    // free(sn_value);
    // free(tts1_value);
    // free(tts2_value);
    // free(data_value);
    // free(second_stream_value);
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

    ecic_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_metrics_send.xml"});
    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
