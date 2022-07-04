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
#include "functional_test.h"
#define TEST_ACTOR_ID TEST_ACTOR1_ID

/********
 * Test *
 ********/

std::string config_path = "../config";

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

    // Checkpoint n~1
    // For this checkpoint the last byte is filled with 1
    // A single sample is sent on one of the streams of the channel
    SAY_SELF("Checkpoint n~1");
    TEST_SYNC();

    // Send
    str_send = strize() << std::setw(stream_info[0]->sample_max_size_bytes) << std::setfill('0') << 1;
    memcpy(sample[0], str_send.c_str(), stream_info[0]->sample_max_size_bytes);
    malloc_count_start();
    const ed247_stream_info_t *stream_info_tmp;
    ASSERT_EQ(ed247_stream_get_info(stream[0], &stream_info_tmp), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_push_sample(stream[0], sample[0], sample_size[0], NULL, NULL), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_send_pushed_samples(NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
    ASSERT_EQ(malloc_count_stop(), 0);

    // Check limit cases
    ASSERT_EQ(ed247_stream_push_sample(NULL, sample[0], sample_size[0], NULL, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_push_sample(stream[0], NULL, sample_size[0], NULL, NULL), ED247_STATUS_FAILURE);

    // Checkpoint n~2
    // Send the maximum number of samples of one stream of the channel
    // Samples are filled with 0s except for the last byte that is
    // filled with the number of the sample
    SAY_SELF("Checkpoint n~2");
    TEST_SYNC();

    // Send
    for(unsigned i = 0 ; i < stream_info[0]->sample_max_number ; i++){
        str_send = strize() << std::setw(stream_info[0]->sample_max_size_bytes) << std::setfill('0') << i;
        memcpy(sample[0], str_send.c_str(), stream_info[0]->sample_max_size_bytes);
        malloc_count_start();
        ASSERT_EQ(ed247_stream_push_sample(stream[0], sample[0], sample_size[0], NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_EQ(malloc_count_stop(), 0);
    }
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

    // Checkpoint n~3
    // For this checkpoint the last byte is filled with the number of the sample for both streams
    // Saturate the frame/channel with all available occurrence of samples from Stream0 and Stream1
    SAY_SELF("Checkpoint n~3");
    TEST_SYNC();

    for(unsigned j = 0 ; j < 2 ; j++){
        for(unsigned i = 0 ; i < stream_info[j]->sample_max_number ; i++){
            str_send = strize() << std::setw(stream_info[j]->sample_max_size_bytes) << std::setfill('0') << i;
            memcpy(sample[j], str_send.c_str(), stream_info[j]->sample_max_size_bytes);
            malloc_count_start();
            ASSERT_EQ(ed247_stream_push_sample(stream[j], sample[j], sample_size[j], NULL, NULL), ED247_STATUS_SUCCESS);
            ASSERT_EQ(malloc_count_stop(), 0);
        }
    }
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

    // Checkpoint n~4
    SAY_SELF("Checkpoint n~4");
    TEST_SYNC();

    for(unsigned j = 0 ; j < 2 ; j++){
        SAY_SELF("Samples data array size [" << stream_info[j]->sample_max_number*stream_info[j]->sample_max_size_bytes << "]");
        samples[j] = malloc(stream_info[j]->sample_max_number*stream_info[j]->sample_max_size_bytes);
        SAY_SELF("Samples data sizes [" << stream_info[j]->sample_max_number*sizeof(size_t) << "]");
        samples_size[j] = malloc(stream_info[j]->sample_max_number*sizeof(size_t));
        for(unsigned i = 0 ; i < stream_info[j]->sample_max_number ; i++){
            str_send = strize() << std::setw(stream_info[j]->sample_max_size_bytes) << std::setfill('0') << i;
            SAY_SELF("+ Append [" << str_send << "]");
            memcpy((char*)samples[j]+i*stream_info[j]->sample_max_size_bytes, str_send.c_str(), stream_info[j]->sample_max_size_bytes);
            ((size_t*)samples_size[j])[i] = stream_info[j]->sample_max_size_bytes;
        }
        SAY_SELF("Push samples");
        malloc_count_start();
        ASSERT_EQ(ed247_stream_push_samples(stream[j], samples[j], (size_t*)samples_size[j], stream_info[j]->sample_max_number, NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_EQ(malloc_count_stop(), 0);
        // Check limit cases
        ASSERT_EQ(ed247_stream_push_samples(NULL, samples[j], (size_t*)samples_size[j], stream_info[j]->sample_max_number, NULL, NULL), ED247_STATUS_FAILURE);
        ASSERT_EQ(ed247_stream_push_samples(stream[j], NULL, (size_t*)samples_size[j], stream_info[j]->sample_max_number, NULL, NULL), ED247_STATUS_FAILURE);
        ASSERT_EQ(ed247_stream_push_samples(stream[j], samples[j], NULL, stream_info[j]->sample_max_number, NULL, NULL), ED247_STATUS_FAILURE);
    }
    SAY_SELF("Send pushed samples");
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

    // Checkpoint n~5.1
    SAY_SELF("Checkpoint n~5.1");
    TEST_SYNC();

    // Receiver is setting the callbacks
    
    // Checkpoint n~5.2
    SAY_SELF("Checkpoint n~5.2");
    TEST_SYNC();

    for(unsigned j = 0 ; j < 2 ; j++){
        SAY_SELF("Push samples " << j);
        malloc_count_start();
        ASSERT_EQ(ed247_stream_push_samples(stream[j], samples[j], (size_t*)samples_size[j], stream_info[j]->sample_max_number, NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_EQ(malloc_count_stop(), 0);
    }
    SAY_SELF("Send pushed samples");
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

    // Receiver is setting the callbacks
    
    // Checkpoint n~6.1
    SAY_SELF("Checkpoint n~6.1");
    TEST_SYNC();

    // Receiver is setting callbacks
    
    // Checkpoint n~6.2
    SAY_SELF("Checkpoint n~6.2");
    TEST_SYNC();

    for(unsigned j = 0 ; j < 2 ; j++){
        SAY_SELF("Push samples " << j);
        malloc_count_start();
        ASSERT_EQ(ed247_stream_push_samples(stream[j], samples[j], (size_t*)samples_size[j], stream_info[j]->sample_max_number, NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_EQ(malloc_count_stop(), 0);
    }
    SAY_SELF("Send pushed samples");
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
    
    // Checkpoint n~7.1
    SAY_SELF("Checkpoint n~7.1");
    TEST_SYNC();

    // Receiver is setting callbacks
    
    // Checkpoint n~7.2
    SAY_SELF("Checkpoint n~7.2");
    TEST_SYNC();

    for(unsigned j = 0 ; j < 2 ; j++){
        SAY_SELF("Push samples " << j);
        malloc_count_start();
        ASSERT_EQ(ed247_stream_push_samples(stream[j], samples[j], (size_t*)samples_size[j], stream_info[j]->sample_max_number, NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_EQ(malloc_count_stop(), 0);
    }
    SAY_SELF("Send pushed samples");
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
    
    // Checkpoint n~8
    SAY_SELF("Checkpoint n~8");
    TEST_SYNC();

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

    // Stream
    ASSERT_EQ(ed247_find_streams(_context, "Stream0", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);

    // Sample
    ASSERT_EQ(ed247_stream_allocate_sample(stream, &sample, &sample_size), ED247_STATUS_SUCCESS);

    // Checkpoint n~1
    // For this checkpoint the last byte is filled with 1
    SAY_SELF("Checkpoint n~1");
    TEST_SYNC();

    // Send
    str_send = strize() << std::setw(stream_info->sample_max_size_bytes) << std::setfill('0') << 1;
    memcpy(sample, str_send.c_str(), stream_info->sample_max_size_bytes);
    malloc_count_start();
    ASSERT_EQ(ed247_stream_push_sample(stream, sample, sample_size, NULL, NULL), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
    ASSERT_EQ(malloc_count_stop(), 0);

    // Checkpoint n~2
    // Send the maximum number of samples of one Stream0
    // Samples are filled with 0s except for the last byte that is
    // filled with the number of the sample
    SAY_SELF("Checkpoint n~2");
    TEST_SYNC();

    // Send
    for(unsigned i = 0 ; i < stream_info->sample_max_number ; i++){
        str_send = strize() << std::setw(stream_info->sample_max_size_bytes) << std::setfill('0') << i;
        memcpy(sample, str_send.c_str(), stream_info->sample_max_size_bytes);
        ASSERT_EQ(ed247_stream_push_sample(stream, sample, sample_size, NULL, NULL), ED247_STATUS_SUCCESS);
    }
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

    // Checkpoint n~3
    // This test case is exactly the same as the previous one
    SAY_SELF("Checkpoint n~3");
    TEST_SYNC();

    for(unsigned i = 0 ; i < stream_info->sample_max_number ; i++){
        str_send = strize() << std::setw(stream_info->sample_max_size_bytes) << std::setfill('0') << i;
        memcpy(sample, str_send.c_str(), stream_info->sample_max_size_bytes);
        ASSERT_EQ(ed247_stream_push_sample(stream, sample, sample_size, NULL, NULL), ED247_STATUS_SUCCESS);
    }
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

    // Checkpoint n~4
    SAY_SELF("Checkpoint n~4");
    TEST_SYNC();

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

    // Stream
    ASSERT_EQ(ed247_find_streams(_context, "Stream0", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);

    // Sample
    ASSERT_EQ(ed247_stream_allocate_sample(stream, &sample, &sample_size), ED247_STATUS_SUCCESS);

    // Checkpoint n~1
    // Fill the maximum amount of samples for Stream0 and send them at once.
    SAY_SELF("Checkpoint n~1");
    TEST_SYNC();

    // Send
    for(unsigned i = 0 ; i < stream_info->sample_max_number ; i++){
        str_send = strize() << std::setw(stream_info->sample_max_size_bytes) << std::setfill('0') << i;
        memcpy(sample, str_send.c_str(), stream_info->sample_max_size_bytes);
        SAY_SELF("push/send sample " << i);
        ASSERT_EQ(ed247_stream_push_sample(stream, sample, sample_size, NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
        if (i == 0) {
          SAY_SELF("Checkpoint n~1.1");
          TEST_SYNC();
        }
    }

    // Checkpoint n~2
    SAY_SELF("Checkpoint n~2");
    TEST_SYNC();

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

    // Stream
    ASSERT_EQ(ed247_find_streams(_context, "Stream0", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);

    // Allocate samples
    size_t s = 0;
    void *tmp_sample;
    size_t tmp_sample_size;
    ASSERT_EQ(ed247_stream_find_signals(stream, ".*", &signals), ED247_STATUS_SUCCESS);
    while(ed247_signal_list_next(signals, &signal) == ED247_STATUS_SUCCESS && signal != nullptr){
        ASSERT_EQ(ed247_signal_get_info(signal, &signal_info), ED247_STATUS_SUCCESS);
        SAY_SELF("Create sample for signal [" << std::string(signal_info->name) << "] ...");
        ASSERT_EQ(ed247_signal_allocate_sample(signal, &samples[s], &sizes[s]), ED247_STATUS_SUCCESS);
        // Check limit cases
        ASSERT_EQ(ed247_signal_allocate_sample(NULL, &tmp_sample, &tmp_sample_size), ED247_STATUS_FAILURE);
        ASSERT_EQ(ed247_signal_allocate_sample(signal, NULL, &tmp_sample_size), ED247_STATUS_FAILURE);
        ASSERT_EQ(ed247_signal_allocate_sample(signal, &tmp_sample, NULL), ED247_STATUS_FAILURE);
        s++;
    }

    // Checkpoint n~1
    // Fill the all signals in the sample before sending
    SAY_SELF("Checkpoint n~1");
    TEST_SYNC();

    // Send
    s = 0;
    ASSERT_EQ(ed247_stream_get_assistant(stream, &assistant), ED247_STATUS_SUCCESS);
    malloc_count_start();
    ASSERT_EQ(ed247_stream_get_signal_list(stream, &signals), ED247_STATUS_SUCCESS);
    ASSERT_EQ(malloc_count_stop(), 0);
    while(ed247_signal_list_next(signals, &signal) == ED247_STATUS_SUCCESS && signal != nullptr){
        ASSERT_EQ(ed247_signal_get_info(signal, &signal_info), ED247_STATUS_SUCCESS);
        SAY_SELF("Writing [" << std::string(signal_info->name) << "] ...");
        if(signal_info->type == ED247_SIGNAL_TYPE_DISCRETE || signal_info->type == ED247_SIGNAL_TYPE_NAD){
          str_send = strize() << std::setw(sizes[s]) << std::setfill('0') << std::string(signal_info->name).substr(6,1);
        }else{
          str_send = strize() << std::setw(sizes[s]) << std::setfill('0') << std::string(signal_info->name).substr(6,2);
        }
        memcpy(samples[s], str_send.c_str(), sizes[s]);
        malloc_count_start();
        ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, signal, samples[s], sizes[s]), ED247_STATUS_SUCCESS);
        ASSERT_EQ(malloc_count_stop(), 0);
        s++;
    }
    malloc_count_start();
    ASSERT_EQ(ed247_stream_assistant_push_sample(assistant, NULL, NULL), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
    ASSERT_EQ(malloc_count_stop(), 0);

    // Checkpoint n~2
    SAY_SELF("Checkpoint n~2");
    TEST_SYNC();

    // Unload
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
}

std::vector<TestParams> stream_files;

INSTANTIATE_TEST_CASE_P(Stream, StreamContext,
    ::testing::ValuesIn(stream_files));

std::vector<TestParams> simple_stream_files;

INSTANTIATE_TEST_CASE_P(SimpleStream, SimpleStreamContext,
    ::testing::ValuesIn(simple_stream_files));

std::vector<TestParams> signal_files ;

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

    tests_tools::display_ed247_lib_infos();
    SAY("Configuration path: " << config_path);

    stream_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_a429_uc_main.xml"});
    stream_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_a429_mc_main.xml"});
    stream_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_a664_mc_main.xml"});
    stream_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_a825_uc_main.xml"});
    stream_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_a825_mc_main.xml"});
    stream_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_serial_uc_main.xml"});
    stream_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_serial_mc_main.xml"});
    
    simple_stream_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_a429_uc_main_simple.xml"});
    simple_stream_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_a429_mc_main_simple.xml"});
    
    signal_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_dis_mc_main.xml"});
    signal_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_ana_mc_main.xml"});
    signal_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_nad_mc_main.xml"});
    signal_files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_exchange_vnad_mc_main.xml"});

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
