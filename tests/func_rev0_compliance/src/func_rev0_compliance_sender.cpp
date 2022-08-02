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
TEST_P(StreamContext, BackupRecv)
{
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    std::string str;
    size_t sample_size;
    const void *sample;
    const ed247_timestamp_t *data_timestamp;
    const ed247_timestamp_t *recv_timestamp;
    const ed247_sample_details_t *sample_details;

    ASSERT_EQ(ed247_find_streams(_context, "Stream0", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);

    // Checkpoint
    SAY_SELF("Checkpoint");
    TEST_SYNC();

    for(unsigned i = 0 ; i < 10 ; i++){
    
        ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);

        str = strize() << std::setw(100) << std::setfill('0') << i;
        ASSERT_EQ(ed247_stream_pop_sample(stream, &sample, &sample_size, &data_timestamp, &recv_timestamp, &sample_details, NULL), ED247_STATUS_SUCCESS);
        ASSERT_EQ(sample_details->component_identifier, 1);
        ASSERT_EQ(sample_details->sequence_number, (size_t)i);

        // Checkpoint
        SAY_SELF("Checkpoint n~" << i);
        TEST_SYNC();
    }

    // Checkpoint
    SAY_SELF("Checkpoint");
    TEST_SYNC();

}

TEST_P(StreamContext, BackupSend)
{
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    std::string str;
    size_t sample_size;
    void *sample;
    ed247_timestamp_t data_timestamp;

    ASSERT_EQ(ed247_find_streams(_context, "Stream1", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);

    ASSERT_EQ(ed247_stream_allocate_sample(stream, &sample, &sample_size), ED247_STATUS_SUCCESS);

    // Checkpoint
    SAY_SELF("Checkpoint");
    TEST_SYNC();

    for(unsigned i = 0 ; i < 10 ; i++){

        str = strize() << std::setw(100) << std::setfill('0') << i;
        // Update frame
        data_timestamp.epoch_s = (uint32_t)i;
        data_timestamp.offset_ns = (uint32_t)i;
        memcpy((char*)sample, str.c_str(), 100);

        // Push & send
        ASSERT_EQ(ed247_stream_push_sample(stream, sample, 100, &data_timestamp, NULL), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

        // Checkpoint
        SAY_SELF("Checkpoint n~" << i);
        TEST_SYNC();
        
    }

    // Checkpoint
    SAY_SELF("Checkpoint");
    TEST_SYNC();

}

std::vector<TestParams> files;

INSTANTIATE_TEST_CASE_P(func_rev0_compliance, StreamContext,
    ::testing::ValuesIn(files));

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

    files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_rev0_compliance_main.xml"});

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
