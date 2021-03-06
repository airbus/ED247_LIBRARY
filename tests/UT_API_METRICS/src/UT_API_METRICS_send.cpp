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

#ifdef __linux
    #include <arpa/inet.h>
#elif _WIN32
    #include <winsock2.h>
#endif

/***********
 * Defines *
 ***********/

#define TEST_ENTITY_SRC_ID 2
#define TEST_ENTITY_DST_ID 1

/********
 * Test *
 ********/
/******************************************************************************
This application is the sender application for the runtime metrics test
******************************************************************************/

class TestMetricsOut : public ::testing::Test{};

/******************************************************************************
This sequence sends frames that do not contain any sequence number
******************************************************************************/
TEST(OutSequence, UTMetrics)
{
    test::Entity::init();
    test::Entity synchronizer(TEST_ENTITY_SRC_ID);
    ed247_context_t context = NULL;
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
    std::string ecic_filename = CONFIG_PATH"/ut_api_metrics/ecic_send.xml";
    
    uint16_t header_values [] = {
        65000,
        65500,
        65535,
        10,
        65500};
    uint16_t header_values_size = 5;
    
    ASSERT_EQ(ed247_load(ecic_filename.c_str(), NULL, &context), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_streams(context, "MyStream",&streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_assistant(stream, &assistant), ED247_STATUS_SUCCESS);
    ASSERT_NE(assistant, (ed247_stream_assistant_t)NULL);
    ASSERT_EQ(ed247_find_signals(context, "DummyHeaderPID" , &signals), ED247_STATUS_SUCCESS);
    ASSERT_NE(signals, (ed247_signal_list_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signals, &dummy_header_pid), ED247_STATUS_SUCCESS);
    ASSERT_NE(dummy_header_pid, (ed247_signal_t)NULL);
    ASSERT_EQ(ed247_signal_allocate_sample(dummy_header_pid, &pid_value, &pid_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_signals(context, "DummyHeaderSN" , &signals), ED247_STATUS_SUCCESS);
    ASSERT_NE(signals, (ed247_signal_list_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signals, &dummy_header_sn), ED247_STATUS_SUCCESS);
    ASSERT_NE(dummy_header_sn, (ed247_signal_t)NULL);
    ASSERT_EQ(ed247_signal_allocate_sample(dummy_header_pid, &sn_value, &sn_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_signals(context, "DummyHeaderTTS1" , &signals), ED247_STATUS_SUCCESS);
    ASSERT_NE(signals, (ed247_signal_list_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signals, &dummy_header_tts1), ED247_STATUS_SUCCESS);
    ASSERT_NE(dummy_header_tts1, (ed247_signal_t)NULL);
    ASSERT_EQ(ed247_signal_allocate_sample(dummy_header_tts1, &tts1_value, &tts1_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_signals(context, "DummyHeaderTTS2" , &signals), ED247_STATUS_SUCCESS);
    ASSERT_NE(signals, (ed247_signal_list_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signals, &dummy_header_tts2), ED247_STATUS_SUCCESS);
    ASSERT_NE(dummy_header_tts2, (ed247_signal_t)NULL);
    ASSERT_EQ(ed247_signal_allocate_sample(dummy_header_tts2, &tts2_value, &tts2_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_signals(context, "Data" , &signals), ED247_STATUS_SUCCESS);
    ASSERT_NE(signals, (ed247_signal_list_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signals, &data_signal), ED247_STATUS_SUCCESS);
    ASSERT_NE(data_signal, (ed247_signal_t)NULL);
    ASSERT_EQ(ed247_signal_allocate_sample(data_signal, &data_value, &data_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_signal_list_free(signals), ED247_STATUS_SUCCESS);
    // uint16_t data = 0x0000;
    
    ASSERT_EQ(ed247_find_streams(context, "MySecondStream",&streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &second_stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_allocate_sample(second_stream, &second_stream_value, &second_stream_size), ED247_STATUS_SUCCESS);
    
    // Synchro after initialisation
    synchronizer.send(TEST_ENTITY_DST_ID);
    ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_DST_ID));
    
    for (uint16_t i = 0; i < header_values_size; i++){
        *(uint16_t*)pid_value = (uint16_t)0;
        *(uint16_t*)sn_value = (uint16_t)(header_values[i]);
        // *(uint16_t*)sn_value = htons(header_values[i]);
        *(uint32_t*)tts1_value = (uint32_t)10;
        *(uint32_t*)tts2_value = (uint32_t)12;
        *(uint16_t*)data_value = i;
        std::cout << "Sending frame " << i+1 << ": Simulated SN is " << header_values[i] << std::endl;
        ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, dummy_header_pid, pid_value, pid_size), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, dummy_header_sn, sn_value, sn_size), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, dummy_header_tts1, tts1_value, tts1_size), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, dummy_header_tts2, tts2_value, tts2_size), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, data_signal, data_value, data_size), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_assistant_push_sample(assistant, NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_send_pushed_samples(context), ED247_STATUS_SUCCESS);
        
        // Synchro after each send
        synchronizer.send(TEST_ENTITY_DST_ID);
        ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_DST_ID));
        
        // Send the second stream to check the runtime metrics is not disturbed
        *(uint32_t*)second_stream_value = 0x12345678;
        ed247_stream_push_sample(second_stream, second_stream_value, second_stream_size, NULL, NULL);
        ASSERT_EQ(ed247_send_pushed_samples(context), ED247_STATUS_SUCCESS);
        
        // Synchro after each send
        synchronizer.send(TEST_ENTITY_DST_ID);
        ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_DST_ID));
    }
    
    synchronizer.send(TEST_ENTITY_DST_ID);
    ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_DST_ID));

    free(pid_value);
    free(sn_value);
    free(tts1_value);
    free(tts2_value);
    free(data_value);
    free(second_stream_value);
    
    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
    
    synchronizer.send(TEST_ENTITY_DST_ID);
    ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_DST_ID));
}

/******************************************************************************
This sequence sends frames that do not contain any sequence number
******************************************************************************/
TEST(OutSequence, UTMetricsCross)
{
    test::Entity::init();
    test::Entity synchronizer(TEST_ENTITY_SRC_ID);
    ed247_context_t context = NULL;
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
    std::string ecic_filename = CONFIG_PATH"/ut_api_metrics/ecic_send.xml";

    uint16_t header_values_cross [2][4] = {
        {10, 15, 56, 413},
        {52, 86, 96, 555}
    };
    
    ASSERT_EQ(ed247_load(ecic_filename.c_str(), NULL, &context), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_streams(context, "MyStream",&streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_assistant(stream, &assistant), ED247_STATUS_SUCCESS);
    ASSERT_NE(assistant, (ed247_stream_assistant_t)NULL);
    ASSERT_EQ(ed247_find_signals(context, "DummyHeaderPID" , &signals), ED247_STATUS_SUCCESS);
    ASSERT_NE(signals, (ed247_signal_list_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signals, &dummy_header_pid), ED247_STATUS_SUCCESS);
    ASSERT_NE(dummy_header_pid, (ed247_signal_t)NULL);
    ASSERT_EQ(ed247_signal_allocate_sample(dummy_header_pid, &pid_value, &pid_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_signals(context, "DummyHeaderSN" , &signals), ED247_STATUS_SUCCESS);
    ASSERT_NE(signals, (ed247_signal_list_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signals, &dummy_header_sn), ED247_STATUS_SUCCESS);
    ASSERT_NE(dummy_header_sn, (ed247_signal_t)NULL);
    ASSERT_EQ(ed247_signal_allocate_sample(dummy_header_pid, &sn_value, &sn_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_signals(context, "DummyHeaderTTS1" , &signals), ED247_STATUS_SUCCESS);
    ASSERT_NE(signals, (ed247_signal_list_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signals, &dummy_header_tts1), ED247_STATUS_SUCCESS);
    ASSERT_NE(dummy_header_tts1, (ed247_signal_t)NULL);
    ASSERT_EQ(ed247_signal_allocate_sample(dummy_header_tts1, &tts1_value, &tts1_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_signals(context, "DummyHeaderTTS2" , &signals), ED247_STATUS_SUCCESS);
    ASSERT_NE(signals, (ed247_signal_list_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signals, &dummy_header_tts2), ED247_STATUS_SUCCESS);
    ASSERT_NE(dummy_header_tts2, (ed247_signal_t)NULL);
    ASSERT_EQ(ed247_signal_allocate_sample(dummy_header_tts2, &tts2_value, &tts2_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_signals(context, "Data" , &signals), ED247_STATUS_SUCCESS);
    ASSERT_NE(signals, (ed247_signal_list_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signals, &data_signal), ED247_STATUS_SUCCESS);
    ASSERT_NE(data_signal, (ed247_signal_t)NULL);
    ASSERT_EQ(ed247_signal_allocate_sample(data_signal, &data_value, &data_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_signal_list_free(signals), ED247_STATUS_SUCCESS);
    // uint16_t data = 0x0000;
    
    ASSERT_EQ(ed247_find_streams(context, "MySecondStream",&streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &second_stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_allocate_sample(second_stream, &second_stream_value, &second_stream_size), ED247_STATUS_SUCCESS);
    
    // Synchro after initialisation
    synchronizer.send(TEST_ENTITY_DST_ID);
    ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_DST_ID));
    
    uint16_t counter = 0;
    for (uint16_t i = 0; i < 4; i++){
        for (uint16_t j = 0; j < 2; j++){
            
            *(uint16_t*)pid_value = (uint16_t)j;
            *(uint16_t*)sn_value = (uint16_t)(header_values_cross[j][i]);
            // *(uint16_t*)sn_value = htons(header_values_cross[j][i]);
            *(uint32_t*)tts1_value = (uint32_t)10;
            *(uint32_t*)tts2_value = (uint32_t)12;
            *(uint16_t*)data_value = counter;
            std::cout << "Sending frame [" << counter << "] Simulated SN [" << header_values_cross[j][i] << "] for PID [" << j << "]" << std::endl;
            ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, dummy_header_pid, pid_value, pid_size), ED247_STATUS_SUCCESS);
            ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, dummy_header_sn, sn_value, sn_size), ED247_STATUS_SUCCESS);
            ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, dummy_header_tts1, tts1_value, tts1_size), ED247_STATUS_SUCCESS);
            ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, dummy_header_tts2, tts2_value, tts2_size), ED247_STATUS_SUCCESS);
            ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, data_signal, data_value, data_size), ED247_STATUS_SUCCESS);
            ASSERT_EQ(ed247_stream_assistant_push_sample(assistant, NULL, NULL), ED247_STATUS_SUCCESS);
            ASSERT_EQ(ed247_send_pushed_samples(context), ED247_STATUS_SUCCESS);
            
            // Synchro after each send
            synchronizer.send(TEST_ENTITY_DST_ID);
            ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_DST_ID));
            
            // Send the second stream to check the runtime metrics is not disturbed
            *(uint32_t*)second_stream_value = 0x12345678;
            ed247_stream_push_sample(second_stream, second_stream_value, second_stream_size, NULL, NULL);
            ASSERT_EQ(ed247_send_pushed_samples(context), ED247_STATUS_SUCCESS);
            
            // Synchro after each send
            synchronizer.send(TEST_ENTITY_DST_ID);
            ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_DST_ID));
            counter++;
        }
    }
    
    synchronizer.send(TEST_ENTITY_DST_ID);
    ASSERT_TRUE(synchronizer.wait(TEST_ENTITY_DST_ID));

    free(pid_value);
    free(sn_value);
    free(tts1_value);
    free(tts2_value);
    free(data_value);
    free(second_stream_value);
    
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
