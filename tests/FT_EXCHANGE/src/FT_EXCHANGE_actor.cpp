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

#define TEST_ENTITY_SRC_ID 1
#define TEST_ENTITY_DST_ID 2

/********
 * Test *
 ********/
/******************************************************************************
This test file globally checks communication for all type of signals and every
configuration. Stream/Signals, Unicast/Multicast and all protocols are checked.
Although multicast is tested, all test cases only involve 2 components: a
sender and a receiver. This file codes for the sender application.
******************************************************************************/

class StreamContext : public TestContext {};
class SimpleStreamContext : public TestContext {};
class SignalContext : public TestContext {};

/******************************************************************************
This test checks the global communication between 2 ed247 components for every
stream protocol present in test vectors. 3 successive sequences are run:
Checkpoint 1 - Send a single data on a single stream
Checkpoint 2 - Send several data on a single stream
Checkpoint 3 - Send several data on a several streams
This is done for every protocol provided and each byte of each sample is filled
with 0s except for the last one that is filled with an other digit depending
on the Checkpoint. This test expects the ECIC files to define channels with
multiple streams which define Stream0 and Stream 1 among them. Provided
configurations are defined in vector stream_files
******************************************************************************/
TEST_P(StreamContext, SingleFrame)
{
    ed247_stream_list_t streams;
    ed247_stream_t stream[2];
    const ed247_stream_info_t *stream_info[2];
    void *sample[2];
    size_t sample_size[2];
    void *samples[2];
    void *samples_size[2];
    std::string str_send;
    std::ostringstream oss;

    // Stream
    ASSERT_EQ(ed247_find_streams(_context, "Stream0", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &(stream[0])), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_info(stream[0], &(stream_info[0])), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_streams(_context, "Stream1", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &(stream[1])), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_info(stream[1], &(stream_info[1])), ED247_STATUS_SUCCESS);

    // Sample
    ASSERT_EQ(ed247_stream_allocate_sample(stream[0], &(sample[0]), &sample_size[0]), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_allocate_sample(stream[1], &(sample[1]), &sample_size[1]), ED247_STATUS_SUCCESS);

    // Check limit cases
    void *tmp_sample;
    size_t tmp_sample_size;
    ASSERT_EQ(ed247_stream_allocate_sample(NULL, &tmp_sample, &tmp_sample_size), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_allocate_sample(stream[0], NULL, &tmp_sample_size), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_allocate_sample(stream[0], &tmp_sample, NULL), ED247_STATUS_FAILURE);

    // Checkpoint n°1
    // For this checkpoint the last byte is filled with 1
    // A single sample is sent on one of the streams of the channel
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°1" << std::endl;
    TestSend(); TestWait();

    // Send
    oss.str("");
    oss << std::setw(stream_info[0]->sample_max_size_bytes) << std::setfill('0') << 1;
    str_send = oss.str();
    memcpy(sample[0], str_send.c_str(), stream_info[0]->sample_max_size_bytes);
    memhooks_section_start();
    const ed247_stream_info_t *stream_info_tmp;
    ASSERT_EQ(ed247_stream_get_info(stream[0], &stream_info_tmp), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_push_sample(stream[0], sample[0], sample_size[0], NULL, NULL), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(memhooks_section_stop());

    // Check limit cases
    ASSERT_EQ(ed247_stream_push_sample(NULL, sample[0], sample_size[0], NULL, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_push_sample(stream[0], NULL, sample_size[0], NULL, NULL), ED247_STATUS_FAILURE);

    // Checkpoint n°2
    // Send the maximum number of samples of one stream of the channel
    // Samples are filled with 0s except for the last byte that is
    // filled with the number of the sample
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°2" << std::endl;
    TestSend(); TestWait();

    // Send
    for(unsigned i = 0 ; i < stream_info[0]->sample_max_number ; i++){
        oss.str("");
        oss << std::setw(stream_info[0]->sample_max_size_bytes) << std::setfill('0') << i;
        str_send = oss.str();
        memcpy(sample[0], str_send.c_str(), stream_info[0]->sample_max_size_bytes);
        memhooks_section_start();
        ASSERT_EQ(ed247_stream_push_sample(stream[0], sample[0], sample_size[0], NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_TRUE(memhooks_section_stop());
    }
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

    // Checkpoint n°3
    // For this checkpoint the last byte is filled with the number of the sample for both streams
    // Saturate the frame/channel with all available occurrence of samples from Stream0 and Stream1
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°3" << std::endl;
    TestSend(); TestWait();

    for(unsigned j = 0 ; j < 2 ; j++){
        for(unsigned i = 0 ; i < stream_info[j]->sample_max_number ; i++){
            oss.str("");
            oss << std::setw(stream_info[j]->sample_max_size_bytes) << std::setfill('0') << i;
            str_send = oss.str();
            memcpy(sample[j], str_send.c_str(), stream_info[j]->sample_max_size_bytes);
            memhooks_section_start();
            ASSERT_EQ(ed247_stream_push_sample(stream[j], sample[j], sample_size[j], NULL, NULL), ED247_STATUS_SUCCESS);
            ASSERT_TRUE(memhooks_section_stop());
        }
    }
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

    // Checkpoint n°4
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°4" << std::endl;
    TestSend(); TestWait();

    for(unsigned j = 0 ; j < 2 ; j++){
        std::cout << "# Samples data array size [" << stream_info[j]->sample_max_number*stream_info[j]->sample_max_size_bytes << "]" << std::endl;
        samples[j] = malloc(stream_info[j]->sample_max_number*stream_info[j]->sample_max_size_bytes);
        std::cout << "# Samples data sizes [" << stream_info[j]->sample_max_number*sizeof(size_t) << "]" << std::endl;
        samples_size[j] = malloc(stream_info[j]->sample_max_number*sizeof(size_t));
        for(unsigned i = 0 ; i < stream_info[j]->sample_max_number ; i++){
            oss.str("");
            oss << std::setw(stream_info[j]->sample_max_size_bytes) << std::setfill('0') << i;
            str_send = oss.str();
            std::cout << "## Append [" << str_send << "]" << std::endl;
            memcpy((char*)samples[j]+i*stream_info[j]->sample_max_size_bytes, str_send.c_str(), stream_info[j]->sample_max_size_bytes);
            ((size_t*)samples_size[j])[i] = stream_info[j]->sample_max_size_bytes;
        }
        std::cout << "# Push samples" << std::endl;
        memhooks_section_start();
        ASSERT_EQ(ed247_stream_push_samples(stream[j], samples[j], (size_t*)samples_size[j], stream_info[j]->sample_max_number, NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_TRUE(memhooks_section_stop());
        // Check limit cases
        ASSERT_EQ(ed247_stream_push_samples(NULL, samples[j], (size_t*)samples_size[j], stream_info[j]->sample_max_number, NULL, NULL), ED247_STATUS_FAILURE);
        ASSERT_EQ(ed247_stream_push_samples(stream[j], NULL, (size_t*)samples_size[j], stream_info[j]->sample_max_number, NULL, NULL), ED247_STATUS_FAILURE);
        ASSERT_EQ(ed247_stream_push_samples(stream[j], samples[j], NULL, stream_info[j]->sample_max_number, NULL, NULL), ED247_STATUS_FAILURE);
    }
    std::cout << "# Send pushed samples" << std::endl;
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
    std::cout << "# Send pushed samples OK" << std::endl;

    // Checkpoint n°5.1
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°5.1" << std::endl;
    TestSend(); TestWait();

    // Receiver is setting the callbacks
    
    // Checkpoint n°5.2
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°5.2" << std::endl;
    TestSend(); TestWait();

    for(unsigned j = 0 ; j < 2 ; j++){
        std::cout << "# Push samples" << std::endl;
        memhooks_section_start();
        ASSERT_EQ(ed247_stream_push_samples(stream[j], samples[j], (size_t*)samples_size[j], stream_info[j]->sample_max_number, NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_TRUE(memhooks_section_stop());
    }
    std::cout << "# Send pushed samples" << std::endl;
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
    std::cout << "# Send pushed samples OK" << std::endl;

    // Receiver is setting the callbacks
    
    // Checkpoint n°6.1
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°6.1" << std::endl;
    TestSend(); TestWait();

    // Receiver is setting callbacks
    
    // Checkpoint n°6.2
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°6.2" << std::endl;
    TestSend(); TestWait();

    for(unsigned j = 0 ; j < 2 ; j++){
        std::cout << "# Push samples" << std::endl;
        memhooks_section_start();
        ASSERT_EQ(ed247_stream_push_samples(stream[j], samples[j], (size_t*)samples_size[j], stream_info[j]->sample_max_number, NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_TRUE(memhooks_section_stop());
    }
    std::cout << "# Send pushed samples" << std::endl;
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
    std::cout << "# Send pushed samples OK" << std::endl;
    
    // Checkpoint n°7.1
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°7.1" << std::endl;
    TestSend(); TestWait();

    // Receiver is setting callbacks
    
    // Checkpoint n°7.2
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°7.2" << std::endl;
    TestSend(); TestWait();

    for(unsigned j = 0 ; j < 2 ; j++){
        std::cout << "# Push samples" << std::endl;
        memhooks_section_start();
        ASSERT_EQ(ed247_stream_push_samples(stream[j], samples[j], (size_t*)samples_size[j], stream_info[j]->sample_max_number, NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_TRUE(memhooks_section_stop());
    }
    std::cout << "# Send pushed samples" << std::endl;
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
    std::cout << "# Send pushed samples OK" << std::endl;
    
    // Checkpoint n°8
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°8" << std::endl;
    TestSend(); TestWait();

    // End
    TestSend();

    // Free memory
    for(unsigned j = 0 ; j < 2 ; j++){
        free(samples[j]);
        free(samples_size[j]);
    }

    // Unload
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
}

/******************************************************************************
This test checks the global communication between 2 ed247 actors for every
stream protocol present in test vectors. 3 successive sequences are run:
Checkpoint 1 - Send a single data on the single stream
Checkpoint 2 - Send several data on the single stream
Checkpoint 3 - Same as Checkpoint 2
This is done for every protocol provided and each byte of each sample is filled
with 0s except for the last one that is filled with an other digit depending
on the Checkpoint. This test expects the ECIC files to define channels with
a unique stream named Stream0. Provided configurations are defined in vector
simple_stream_files
******************************************************************************/
TEST_P(SimpleStreamContext, SingleFrame)
{
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    const ed247_stream_info_t *stream_info;
    void *sample;
    size_t sample_size;
    std::string str_send;
    std::ostringstream oss;

    // Stream
    ASSERT_EQ(ed247_find_streams(_context, "Stream0", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);

    // Sample
    ASSERT_EQ(ed247_stream_allocate_sample(stream, &sample, &sample_size), ED247_STATUS_SUCCESS);

    // Checkpoint n°1
    // For this checkpoint the last byte is filled with 1
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°1" << std::endl;
    TestSend(); TestWait();

    // Send
    oss.str("");
    oss << std::setw(stream_info->sample_max_size_bytes) << std::setfill('0') << 1;
    str_send = oss.str();
    memcpy(sample, str_send.c_str(), stream_info->sample_max_size_bytes);
    memhooks_section_start();
    ASSERT_EQ(ed247_stream_push_sample(stream, sample, sample_size, NULL, NULL), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(memhooks_section_stop());

    // Checkpoint n°2
    // Send the maximum number of samples of one Stream0
    // Samples are filled with 0s except for the last byte that is
    // filled with the number of the sample
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°2" << std::endl;
    TestSend(); TestWait();

    // Send
    for(unsigned i = 0 ; i < stream_info->sample_max_number ; i++){
        oss.str("");
        oss << std::setw(stream_info->sample_max_size_bytes) << std::setfill('0') << i;
        str_send = oss.str();
        memcpy(sample, str_send.c_str(), stream_info->sample_max_size_bytes);
        ASSERT_EQ(ed247_stream_push_sample(stream, sample, sample_size, NULL, NULL), ED247_STATUS_SUCCESS);
    }
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

    // Checkpoint n°3
    // This test case is exactly the same as the previous one
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°3" << std::endl;
    TestSend(); TestWait();

    for(unsigned i = 0 ; i < stream_info->sample_max_number ; i++){
        oss.str("");
        oss << std::setw(stream_info->sample_max_size_bytes) << std::setfill('0') << i;
        str_send = oss.str();
        memcpy(sample, str_send.c_str(), stream_info->sample_max_size_bytes);
        ASSERT_EQ(ed247_stream_push_sample(stream, sample, sample_size, NULL, NULL), ED247_STATUS_SUCCESS);
    }
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

    // Checkpoint n°4
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°4" << std::endl;
    TestSend(); TestWait();

    // End
    TestSend();

    // Unload
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
}

/******************************************************************************
This test checks the global communication between 2 ed247 actors for every
stream protocol present in test vectors. Only one checkpoint is run in this
test: Send several data on the single stream
This test case is equivalent to the 2nd checkpoint of the first test in file.
This is done for every protocol provided and each byte of each sample is filled
with 0s except for the last one that is filled with an other digit depending
on the Checkpoint. This test expects the ECIC files to define channels with
a at least a stream named Stream0. Provided configurations are defined in
vector stream_files
******************************************************************************/
TEST_P(StreamContext, MultipleFrame)
{
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    const ed247_stream_info_t *stream_info;
    void *sample;
    size_t sample_size;
    std::string str_send;
    std::ostringstream oss;

    // Stream
    ASSERT_EQ(ed247_find_streams(_context, "Stream0", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);

    // Sample
    ASSERT_EQ(ed247_stream_allocate_sample(stream, &sample, &sample_size), ED247_STATUS_SUCCESS);

    // Checkpoint n°1
    // Fill the maximum amount of samples for Stream0 and send them at once.
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°1" << std::endl;
    TestSend(); TestWait();

    // Send
    for(unsigned i = 0 ; i < stream_info->sample_max_number ; i++){
        oss.str("");
        oss << std::setw(stream_info->sample_max_size_bytes) << std::setfill('0') << i;
        str_send = oss.str();
        memcpy(sample, str_send.c_str(), stream_info->sample_max_size_bytes);
        ASSERT_EQ(ed247_stream_push_sample(stream, sample, sample_size, NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
    }

    // Checkpoint n°2
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°2" << std::endl;
    TestSend(); TestWait();

    // End
    TestSend();

    // Unload
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
}

/******************************************************************************
This test checks the global communication between 2 ed247 actors for signals
present in test vectors. Only one checkpoint is run in this test: Send several
signals on the Stream0.
This is done for every signal protocol provided and each byte of each signal is
filled with 0s except for the last one that is filled with a 1.
This test expects the ECIC files to define at least the Stream0 with signals
Signal00 and Signal01. Provided configurations are defined in
vector signal_files
******************************************************************************/
TEST_P(SignalContext, SingleFrame)
{
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    const ed247_stream_info_t *stream_info;
    ed247_stream_assistant_t assistant;
    ed247_signal_list_t signals;
    ed247_signal_t signal;
    const ed247_signal_info_t *signal_info;
    void *samples[2];
    size_t sizes[2];
    std::string str_send;
    std::ostringstream oss;

    // Stream
    ASSERT_EQ(ed247_find_streams(_context, "Stream0", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);

    // Allocate samples
    size_t s = 0;
    void *tmp_sample;
    size_t tmp_sample_size;
    ASSERT_EQ(ed247_find_stream_signals(stream, ".*", &signals), ED247_STATUS_SUCCESS);
    while(ed247_signal_list_next(signals, &signal) == ED247_STATUS_SUCCESS && signal != nullptr){
        ASSERT_EQ(ed247_signal_get_info(signal, &signal_info), ED247_STATUS_SUCCESS);
        std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Create sample for signal [" << std::string(signal_info->name) << "] ..." << std::endl;
        ASSERT_EQ(ed247_signal_allocate_sample(signal, &samples[s], &sizes[s]), ED247_STATUS_SUCCESS);
        // Check limit cases
        ASSERT_EQ(ed247_signal_allocate_sample(NULL, &tmp_sample, &tmp_sample_size), ED247_STATUS_FAILURE);
        ASSERT_EQ(ed247_signal_allocate_sample(signal, NULL, &tmp_sample_size), ED247_STATUS_FAILURE);
        ASSERT_EQ(ed247_signal_allocate_sample(signal, &tmp_sample, NULL), ED247_STATUS_FAILURE);
        s++;
    }

    // Checkpoint n°1
    // Fill the all signals in the sample before sending
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°1" << std::endl;
    TestSend(); TestWait();

    // Send
    s = 0;
    ASSERT_EQ(ed247_stream_get_assistant(stream, &assistant), ED247_STATUS_SUCCESS);
    memhooks_section_start();
    ASSERT_EQ(ed247_stream_get_signals(stream, &signals), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(memhooks_section_stop());
    while(ed247_signal_list_next(signals, &signal) == ED247_STATUS_SUCCESS && signal != nullptr){
        ASSERT_EQ(ed247_signal_get_info(signal, &signal_info), ED247_STATUS_SUCCESS);
        std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Writing [" << std::string(signal_info->name) << "] ..." << std::endl;
        oss.str("");
        if(signal_info->type == ED247_SIGNAL_TYPE_DISCRETE || signal_info->type == ED247_SIGNAL_TYPE_NAD){
            oss << std::setw(sizes[s]) << std::setfill('0') << std::string(signal_info->name).substr(6,1);
        }else{
            oss << std::setw(sizes[s]) << std::setfill('0') << std::string(signal_info->name).substr(6,2);
        }
        str_send = oss.str();
        memcpy(samples[s], str_send.c_str(), sizes[s]);
        memhooks_section_start();
        ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, signal, samples[s], sizes[s]), ED247_STATUS_SUCCESS);
        ASSERT_TRUE(memhooks_section_stop());
        s++;
    }
    memhooks_section_start();
    ASSERT_EQ(ed247_stream_assistant_push_sample(assistant, NULL, NULL), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(memhooks_section_stop());

    // Checkpoint n°2
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°2" << std::endl;
    TestSend(); TestWait();

    // End
    TestSend();

    // Unload
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
}

std::vector<TestParams> stream_files = {
    {TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, std::string(CONFIG_PATH"/ft_exchange/a429_uc_1.xml")},
    {TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, std::string(CONFIG_PATH"/ft_exchange/a429_mc_1.xml")},
    {TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, std::string(CONFIG_PATH"/ft_exchange/a664_mc_1.xml")},
    {TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, std::string(CONFIG_PATH"/ft_exchange/a825_uc_1.xml")},
    {TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, std::string(CONFIG_PATH"/ft_exchange/a825_mc_1.xml")},
    {TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, std::string(CONFIG_PATH"/ft_exchange/serial_uc_1.xml")},
    {TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, std::string(CONFIG_PATH"/ft_exchange/serial_mc_1.xml")}
    };

INSTANTIATE_TEST_CASE_P(FT_EXCHANGE_STREAMS, StreamContext,
    ::testing::ValuesIn(stream_files));

std::vector<TestParams> simple_stream_files = {
    {TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, std::string(CONFIG_PATH"/ft_exchange/a429_uc_1_simple.xml")},
    {TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, std::string(CONFIG_PATH"/ft_exchange/a429_mc_1_simple.xml")}
    };

INSTANTIATE_TEST_CASE_P(FT_EXCHANGE_SIMPLE_STREAMS, SimpleStreamContext,
    ::testing::ValuesIn(simple_stream_files));

std::vector<TestParams> signal_files = {
    {TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, std::string(CONFIG_PATH"/ft_exchange/dis_mc_1.xml")},
    {TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, std::string(CONFIG_PATH"/ft_exchange/ana_mc_1.xml")},
    {TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, std::string(CONFIG_PATH"/ft_exchange/nad_mc_1.xml")},
    {TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, std::string(CONFIG_PATH"/ft_exchange/vnad_mc_1.xml")}
    };

INSTANTIATE_TEST_CASE_P(FT_EXCHANGE_SIGNALS, SignalContext,
    ::testing::ValuesIn(signal_files));

/*************
 * Functions *
 *************/

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
