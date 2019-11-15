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

#ifdef __linux
    #include <arpa/inet.h>
#elif _WIN32
    #include <winsock2.h>
#endif

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
TEST_P(StreamContext, BackupRecv)
{
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    std::ostringstream oss;
    std::string str;
    size_t sample_size;
    const void *sample;
    const ed247_timestamp_t *data_timestamp;
    const ed247_timestamp_t *recv_timestamp;
    const ed247_sample_info_t *info;

    ASSERT_EQ(ed247_find_streams(_context, "Stream0", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);

    // Checkpoint
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint" << std::endl;
    TestSend(); TestWait();

    for(unsigned i = 0 ; i < 10 ; i++){
    
        ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);

        oss.str("");
        oss << std::setw(100) << std::setfill('0') << i;
        str = oss.str();
        ASSERT_EQ(ed247_stream_pop_sample(stream, &sample, &sample_size, &data_timestamp, &recv_timestamp, &info, NULL), ED247_STATUS_SUCCESS);
        ASSERT_EQ(info->component_identifier, 1);
        ASSERT_EQ(info->sequence_number, (size_t)i);

        // Checkpoint
        std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°" << i << std::endl;
        TestSend(); TestWait();
    }

    // Checkpoint
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint" << std::endl;
    TestSend(); TestWait();

}

TEST_P(StreamContext, BackupSend)
{
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    std::ostringstream oss;
    std::string str;
    size_t sample_size;
    void *sample;
    ed247_timestamp_t data_timestamp;

    ASSERT_EQ(ed247_find_streams(_context, "Stream1", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);

    ASSERT_EQ(ed247_stream_allocate_sample(stream, &sample, &sample_size), ED247_STATUS_SUCCESS);

    // Checkpoint
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint" << std::endl;
    TestSend(); TestWait();

    for(unsigned i = 0 ; i < 10 ; i++){

        oss.str("");
        oss << std::setw(100) << std::setfill('0') << i;
        str = oss.str();
        // Update frame
        data_timestamp.epoch_s = (uint32_t)i;
        data_timestamp.offset_ns = (uint32_t)i;
        memcpy((char*)sample, str.c_str(), 100);

        // Push & send
        ASSERT_EQ(ed247_stream_push_sample(stream, sample, 100, &data_timestamp, NULL), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

        // Checkpoint
        std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°" << i << std::endl;
        TestSend(); TestWait();
        
    }

    // Checkpoint
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint" << std::endl;
    TestSend(); TestWait();

}

std::vector<TestParams> files = {
    {TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, std::string(CONFIG_PATH"/ft_backup/a664_mc_1.xml")}
    };

INSTANTIATE_TEST_CASE_P(FT_BACKUP, StreamContext,
    ::testing::ValuesIn(files));

/*************
 * Functions *
 *************/

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
