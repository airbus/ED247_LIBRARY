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
#include "two_actors_test.h"
#define TEST_ACTOR_ID TEST_ACTOR2_ID

std::string config_path = "../config";

/******************************************************************************
This test file globally checks communication for all type of signals and every
configuration. Stream/Signals, Unicast/Multicast and all protocols are checked.
Although multicast is tested, all test cases only involve 2 components: a
sender and a receiver. This file codes for the receiver application.
Further common details of the test sequence are documented in the sender
application.
******************************************************************************/

class StreamContext : public TestContext {};
class SimpleStreamContext : public TestContext {};
class SignalContext : public TestContext {};

/******************************************************************************
This test case also specifically checks the behavior of the get_simulation_time
******************************************************************************/
TEST_P(StreamContext, BackupRecv)
{
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    std::string str;
    uint32_t sample_size;
    void *sample;
    uint32_t sample_index = 0;

    ASSERT_EQ(ed247_find_streams(_context, "Stream0", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ed247_stream_list_free(streams);

    sample_size = sizeof(uint16_t)+
        sizeof(uint16_t)+
        sizeof(uint32_t)+sizeof(uint32_t)+
        sizeof(uint32_t)+sizeof(uint32_t)+
        100;
    sample = malloc(sample_size);

    // Checkpoint
    SAY_SELF("Checkpoint");
    TEST_SYNC();

    for(unsigned i = 0 ; i < 10 ; i++){
        str = strize() << std::setw(100) << std::setfill('0') << i;
        // Update frame
        sample_index = 0;
        *(uint16_t*)((char*)sample+sample_index) = htons((uint16_t)1);
        sample_index += sizeof(uint16_t);
        *(uint16_t*)((char*)sample+sample_index) = htons((uint16_t)i);
        sample_index += sizeof(uint16_t);
        *(uint32_t*)((char*)sample+sample_index) = htons((uint32_t)i);
        sample_index += sizeof(uint32_t);
        *(uint32_t*)((char*)sample+sample_index) = htons((uint32_t)i);
        sample_index += sizeof(uint32_t);
        *(uint32_t*)((char*)sample+sample_index) = htons((uint32_t)i);
        sample_index += sizeof(uint32_t);
        *(uint32_t*)((char*)sample+sample_index) = htons((uint32_t)i);
        sample_index += sizeof(uint32_t);
        memcpy((char*)sample+sample_index, str.c_str(), 100);
        sample_index += 100;
        ASSERT_EQ(sample_index, sample_size);

        // Push & send
        ASSERT_EQ(ed247_stream_push_sample(stream, sample, sample_size, NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

        // Checkpoint
        SAY_SELF("Checkpoint n~" << i);
        TEST_SYNC();

    }

    free(sample);

    // Checkpoint
    SAY_SELF("Checkpoint");
    TEST_SYNC();

}

TEST_P(StreamContext, BakcupSend)
{
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    std::string str;
    uint32_t sample_size;
    const void *sample;
    uint32_t sample_index = 0;

    ASSERT_EQ(ed247_find_streams(_context, "Stream1", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ed247_stream_list_free(streams);

    // Checkpoint
    SAY_SELF("Checkpoint");
    TEST_SYNC();

    for(unsigned i = 0 ; i < 10 ; i++){

        ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);

        str = strize() << std::setw(100) << std::setfill('0') << i;
        ASSERT_EQ(ed247_stream_pop_sample(stream, &sample, &sample_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);

        // Check frame
        sample_index = 0;
        ASSERT_TRUE(ntohs(*(uint16_t*)((char*)sample+sample_index)) == (uint16_t)0);
        sample_index += sizeof(uint16_t);
        ASSERT_TRUE(ntohs(*(uint16_t*)((char*)sample+sample_index)) == (uint16_t)i);
        sample_index += sizeof(uint16_t);
        // ASSERT_EQ(*(uint32_t*)((char*)sample+sample_index), (uint32_t)i);
        sample_index += sizeof(uint32_t);
        // ASSERT_EQ(*(uint32_t*)((char*)sample+sample_index), (uint32_t)i);
        sample_index += sizeof(uint32_t);
        ASSERT_EQ(ntohl(*(uint32_t*)((char*)sample+sample_index)), (uint32_t)i);
        sample_index += sizeof(uint32_t);
        ASSERT_EQ(ntohl(*(uint32_t*)((char*)sample+sample_index)), (uint32_t)i);
        sample_index += sizeof(uint32_t);
        ASSERT_EQ(memcmp((char*)sample+sample_index, str.c_str(), 100), 0);

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

    files.push_back({TEST_ACTOR_ID, config_path+"/ecic_func_rev0_compliance_tester.xml"});

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
