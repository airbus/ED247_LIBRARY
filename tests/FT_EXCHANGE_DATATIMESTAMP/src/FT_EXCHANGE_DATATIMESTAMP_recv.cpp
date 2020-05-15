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
/******************************************************************************
This test file globally checks communication for all type of signals and every
configuration. Stream/Signals, Unicast/Multicast and all protocols are checked.
Although multicast is tested, all test cases only involve 2 components: a
sender and a receiver. This file codes for the receiver application.
Further common details of the test sequence are documented in the sender
application.
******************************************************************************/

class StreamContext : public TestContext {};

/******************************************************************************
This test case also specifically checks the behavior of the get_simulation_time
******************************************************************************/
TEST_P(StreamContext, SingleFrame)
{
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    const ed247_stream_info_t *stream_info;
    const void *sample;
    size_t sample_size;
    std::string str_send, str_recv;
    std::ostringstream oss;
    const ed247_timestamp_t* timestamp;

    
    // Checkpoint n°1
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°1" << std::endl;
    TestWait(); TestSend();

    memhooks_section_start();

    // Recv a single frame
    ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS);

    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_pop_sample(stream, &sample, &sample_size, &timestamp, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
    
    ASSERT_TRUE(memhooks_section_stop());
    
    // Extract and check content of payload
    oss.str("");
    oss << std::setw(stream_info->sample_max_size_bytes) << std::setfill('0') << 1;
    str_send = oss.str();
    str_recv = std::string((char*)sample, stream_info->sample_max_size_bytes);
    ASSERT_EQ(str_send, str_recv);
    // Check the received timestamp is the expected one
    ASSERT_EQ(1234567, timestamp->epoch_s);
    ASSERT_EQ(8910, timestamp->offset_ns);

    // Unload
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
}

std::vector<TestParams> stream_files = {
    {TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, std::string(CONFIG_PATH"/ft_exchange_datatimestamp/a429_2.xml")}
    };

INSTANTIATE_TEST_CASE_P(FT_EXCHANGE_STREAMS, StreamContext,
    ::testing::ValuesIn(stream_files));
	
/*************
 * Functions *
 *************/

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
