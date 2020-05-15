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

/***********
 * Defines *
 ***********/

#define TEST_ENTITY_SRC_ID 2
#define TEST_ENTITY_DST_ID 1

/********
 * Test *
 ********/

class ExchangeContext : public TestContext {};

void fill_payload(void *payload, size_t size, uint8_t value)
{
    std::string str;
    std::ostringstream oss;

    oss.str("");
    oss << std::setw(size) << std::setfill('0') << value;
    str = oss.str();
    memcpy(payload, str.c_str(), size);

}

TEST_P(ExchangeContext, AllStreams)
{

    ed247_stream_list_t streams;
    ed247_stream_t stream;
    const ed247_stream_info_t *info;
    ed247_stream_assistant_t assistant;
    ed247_signal_list_t signals;
    ed247_signal_t signal;
    void *sample;
    size_t size;
    std::string str;
    std::ostringstream oss;
    std::vector<void*> samples;
    size_t streams_conf_size;
    size_t streams_recv_size;
    const void * recv_sample;
    size_t recv_size;

    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°1" << std::endl;
    TestWait(); TestSend();

    ASSERT_EQ(ed247_component_get_streams(_context, &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_size(streams, &streams_conf_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);

    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°2" << std::endl;
    TestWait(); TestSend();

    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_size(streams, &streams_recv_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(streams_conf_size, streams_recv_size);

    while(ed247_stream_list_next(streams, &stream) == ED247_STATUS_SUCCESS && stream != nullptr){
        ASSERT_EQ(ed247_stream_get_info(stream, &info), ED247_STATUS_SUCCESS);
        if(ed247_stream_get_assistant(stream, &assistant) == ED247_STATUS_SUCCESS){
            ASSERT_EQ(ed247_stream_get_signals(stream, &signals),ED247_STATUS_SUCCESS);
            for(uint32_t i = 0 ; i < info->sample_max_number ; i++){
                ASSERT_EQ(ed247_stream_assistant_pop_sample(assistant, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
                while(ed247_signal_list_next(signals, &signal) == ED247_STATUS_SUCCESS && signal != nullptr){
                    ASSERT_EQ(ed247_stream_assistant_read_signal(assistant, signal, &recv_sample, &recv_size), ED247_STATUS_SUCCESS);
                    ASSERT_EQ(ed247_signal_allocate_sample(signal, &sample, &size), ED247_STATUS_SUCCESS);
                    samples.push_back(sample);
                    fill_payload(sample, size, (uint8_t)i);
                    ASSERT_EQ(size, recv_size);
                    ASSERT_EQ(memcmp(sample, recv_sample, size), 0);
                    free(sample);
                }
            }
            ed247_signal_list_free(signals);
        }else{
            for(uint32_t i = 0 ; i < info->sample_max_number ; i++){
                ASSERT_EQ(ed247_stream_pop_sample(stream, &recv_sample, &recv_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
                ASSERT_EQ(ed247_stream_allocate_sample(stream, &sample, &size), ED247_STATUS_SUCCESS);
                fill_payload(sample, size, (uint8_t)i);
                ASSERT_EQ(size, recv_size);
                ASSERT_EQ(memcmp(sample, recv_sample, size), 0);
                free(sample);
            }
        }
    }
    ed247_stream_list_free(streams);

    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°3" << std::endl;
    TestWait(); TestSend();

    // Unload
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);

}

static std::vector<TestParams> config_files;

INSTANTIATE_TEST_CASE_P(FT_EXCHANGE_EXTERNAL, ExchangeContext,
    ::testing::ValuesIn(config_files));

/*************
 * Functions *
 *************/

int main(int argc, char **argv)
{
    config_files.push_back({TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, std::string(CONFIG_PATH)+"/ft_exchange_external/"+argv[1]});
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
