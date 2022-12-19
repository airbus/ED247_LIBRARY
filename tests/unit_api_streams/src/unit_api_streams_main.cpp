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

#include "unitary_test.h"

std::string config_path = "../config";

/******************************************************************************
This test file checks the functions that manipulate channels are working correctly
******************************************************************************/

class UtApiStreams : public ::testing::Test{};

/******************************************************************************
This first test case checks loading of the streams is correctly performed.
The basic getter for streams is partially validated as well as stream list
manipulation function.
******************************************************************************/	
TEST(UtApiStreams, LoadStreams)
{
    ed247_context_t context;
    ed247_stream_list_t stream_list;
    ed247_stream_t stream;

    std::string filepath = config_path+"/ecic_unit_api_streams_single_channel.xml";
    SAY("Load ECIC " << filepath);
    ASSERT_EQ(ed247_load_file(filepath.c_str(), &context), ED247_STATUS_SUCCESS);


    // Check user_data
    ASSERT_EQ(ed247_get_stream(context, "Stream2", &stream), ED247_STATUS_SUCCESS);
    void *user_data = nullptr;
    ASSERT_EQ(ed247_stream_get_user_data(NULL, &user_data), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_get_user_data(stream, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_get_user_data(stream, &user_data), ED247_STATUS_SUCCESS);
    ASSERT_EQ(user_data, nullptr);
    void *user_data_set = malloc(sizeof(uint8_t));
    *(uint8_t*)user_data_set = 12;
    ASSERT_EQ(ed247_stream_set_user_data(stream, user_data_set), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_user_data(stream, &user_data), ED247_STATUS_SUCCESS);
    ASSERT_EQ(*(uint8_t*)user_data, 12);
    ASSERT_EQ(ed247_stream_set_user_data(NULL, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_set_user_data(stream, NULL), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_user_data(stream, &user_data), ED247_STATUS_SUCCESS);
    ASSERT_EQ(user_data, nullptr);
    free(user_data_set);

    // First tests validate the parsing of the ecic file
    ASSERT_EQ(ed247_get_stream_list(NULL, &stream_list), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_get_stream_list(context, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_get_stream_list(context, &stream_list), ED247_STATUS_SUCCESS);

    // Check stream list size
    uint32_t size;
    ASSERT_EQ(ed247_stream_list_size(stream_list, &size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(size, (uint32_t)16);

    // Check the retrieved content, perform unvalid calls to verify robustness
    ASSERT_EQ(ed247_stream_list_next(stream_list, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_list_next(NULL, &stream), ED247_STATUS_FAILURE);

    std::set<std::string> stream_found;
    while(true) {
      ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
      if (stream == nullptr) break;
      std::string name = ed247_stream_get_name(stream);
      stream_found.insert(name);

      if (name == "Stream2") {
        ASSERT_EQ(ed247_stream_get_direction(stream), ED247_DIRECTION_OUT);
        ASSERT_EQ(ed247_stream_get_type(stream), ED247_STREAM_TYPE_A429);
        ASSERT_STREQ(ed247_stream_get_comment(stream), "");
        ASSERT_STREQ(ed247_stream_get_icd(stream), "");
        ASSERT_EQ(ed247_stream_get_uid(stream), (ed247_uid_t)1);
        ASSERT_EQ(ed247_stream_get_sample_max_number(stream), (uint32_t)1);
        ASSERT_EQ(ed247_stream_get_sample_max_size_bytes(stream), (uint32_t)4);

      } else if (name == "Stream2full") {
        ASSERT_EQ(ed247_stream_get_direction(stream), ED247_DIRECTION_OUT);
        ASSERT_EQ(ed247_stream_get_type(stream), ED247_STREAM_TYPE_A429);
        ASSERT_STREQ(ed247_stream_get_comment(stream), "Optional for A429");
        ASSERT_STREQ(ed247_stream_get_icd(stream), "ICD for A429");
        ASSERT_EQ(ed247_stream_get_uid(stream), (ed247_uid_t)101);
        ASSERT_EQ(ed247_stream_get_sample_max_number(stream), (uint32_t)3);
        ASSERT_EQ(ed247_stream_get_sample_max_size_bytes(stream), (uint32_t)4);

      } else if (name == "Stream3") {
        ASSERT_EQ(ed247_stream_get_direction(stream), ED247_DIRECTION_OUT);
        ASSERT_EQ(ed247_stream_get_type(stream), ED247_STREAM_TYPE_A825);
        ASSERT_STREQ(ed247_stream_get_comment(stream), "");
        ASSERT_STREQ(ed247_stream_get_icd(stream), "");
        ASSERT_EQ(ed247_stream_get_uid(stream), (ed247_uid_t)2);
        ASSERT_EQ(ed247_stream_get_sample_max_number(stream), (uint32_t)1);
        ASSERT_EQ(ed247_stream_get_sample_max_size_bytes(stream), (uint32_t)69);

      } else if (name == "Stream3full") {
        ASSERT_EQ(ed247_stream_get_direction(stream), ED247_DIRECTION_OUT);
        ASSERT_EQ(ed247_stream_get_type(stream), ED247_STREAM_TYPE_A825);
        ASSERT_STREQ(ed247_stream_get_comment(stream), "A825 designates CAN");
        ASSERT_STREQ(ed247_stream_get_icd(stream), "ICD for A825");
        ASSERT_EQ(ed247_stream_get_uid(stream), (ed247_uid_t)102);
        ASSERT_EQ(ed247_stream_get_sample_max_number(stream), (uint32_t)17);
        ASSERT_EQ(ed247_stream_get_sample_max_size_bytes(stream), (uint32_t)69);

      } else if (name == "Stream1") {
        ASSERT_EQ(ed247_stream_get_direction(stream), ED247_DIRECTION_OUT);
        ASSERT_EQ(ed247_stream_get_type(stream), ED247_STREAM_TYPE_A664);
        ASSERT_STREQ(ed247_stream_get_comment(stream), "");
        ASSERT_STREQ(ed247_stream_get_icd(stream), "");
        ASSERT_EQ(ed247_stream_get_uid(stream), (ed247_uid_t)0);
        ASSERT_EQ(ed247_stream_get_sample_max_number(stream), (uint32_t)1);
        ASSERT_EQ(ed247_stream_get_sample_max_size_bytes(stream), (uint32_t)1471);

      } else if (name == "Stream1full") {
        ASSERT_EQ(ed247_stream_get_direction(stream), ED247_DIRECTION_OUT);
        ASSERT_EQ(ed247_stream_get_type(stream), ED247_STREAM_TYPE_A664);
        ASSERT_STREQ(ed247_stream_get_comment(stream), "Optional for A664");
        ASSERT_STREQ(ed247_stream_get_icd(stream), "Test for A664");
        ASSERT_EQ(ed247_stream_get_uid(stream), (ed247_uid_t)100);
        ASSERT_EQ(ed247_stream_get_sample_max_number(stream), (uint32_t)42);
        ASSERT_EQ(ed247_stream_get_sample_max_size_bytes(stream), (uint32_t)1471);

      } else if (name == "StreamSerialFull") {
        ASSERT_EQ(ed247_stream_get_direction(stream), ED247_DIRECTION_OUT);
        ASSERT_EQ(ed247_stream_get_type(stream), ED247_STREAM_TYPE_SERIAL);
        ASSERT_STREQ(ed247_stream_get_comment(stream), "SERIAL line");
        ASSERT_STREQ(ed247_stream_get_icd(stream), "ICD for SERIAL");
        ASSERT_EQ(ed247_stream_get_uid(stream), (ed247_uid_t)1002);
        ASSERT_EQ(ed247_stream_get_sample_max_number(stream), (uint32_t)27);
        ASSERT_EQ(ed247_stream_get_sample_max_size_bytes(stream), (uint32_t)123);

      } else if (name == "StreamSerial") {
        ASSERT_EQ(ed247_stream_get_direction(stream), ED247_DIRECTION_OUT);
        ASSERT_EQ(ed247_stream_get_type(stream), ED247_STREAM_TYPE_SERIAL);
        ASSERT_STREQ(ed247_stream_get_comment(stream), "");
        ASSERT_STREQ(ed247_stream_get_icd(stream), "");
        ASSERT_EQ(ed247_stream_get_uid(stream), (ed247_uid_t)1003);
        ASSERT_EQ(ed247_stream_get_sample_max_number(stream), (uint32_t)1);
        ASSERT_EQ(ed247_stream_get_sample_max_size_bytes(stream), (uint32_t)321);

      } else if (name == "Stream4") {
        ASSERT_EQ(ed247_stream_get_direction(stream), ED247_DIRECTION_OUT);
        ASSERT_EQ(ed247_stream_get_type(stream), ED247_STREAM_TYPE_DISCRETE);
        ASSERT_STREQ(ed247_stream_get_comment(stream), "");
        ASSERT_STREQ(ed247_stream_get_icd(stream), "");
        ASSERT_EQ(ed247_stream_get_uid(stream), (ed247_uid_t)3);
        ASSERT_EQ(ed247_stream_get_sample_max_number(stream), (uint32_t)1);
        ASSERT_EQ(ed247_stream_get_sample_max_size_bytes(stream), (uint32_t)4);
        ASSERT_EQ(ed247_stream_get_sampling_period_us(stream), (uint32_t)10000);

      } else if (name == "Stream4full") {
        ASSERT_EQ(ed247_stream_get_direction(stream), ED247_DIRECTION_OUT);
        ASSERT_EQ(ed247_stream_get_type(stream), ED247_STREAM_TYPE_DISCRETE);
        ASSERT_STREQ(ed247_stream_get_comment(stream), "Discrete input signals");
        ASSERT_STREQ(ed247_stream_get_icd(stream), "ICD for DSI");
        ASSERT_EQ(ed247_stream_get_uid(stream), (ed247_uid_t)103);
        ASSERT_EQ(ed247_stream_get_sample_max_number(stream), (uint32_t)14);
        ASSERT_EQ(ed247_stream_get_sample_max_size_bytes(stream), (uint32_t)4);
        ASSERT_EQ(ed247_stream_get_sampling_period_us(stream), (uint32_t)15000);

      } else if (name == "Stream5") {
        ASSERT_EQ(ed247_stream_get_direction(stream), ED247_DIRECTION_OUT);
        ASSERT_EQ(ed247_stream_get_type(stream), ED247_STREAM_TYPE_ANALOG);
        ASSERT_STREQ(ed247_stream_get_comment(stream), "");
        ASSERT_STREQ(ed247_stream_get_icd(stream), "");
        ASSERT_EQ(ed247_stream_get_uid(stream), (ed247_uid_t)4);
        ASSERT_EQ(ed247_stream_get_sample_max_number(stream), (uint32_t)1);
        ASSERT_EQ(ed247_stream_get_sample_max_size_bytes(stream), (uint32_t)16);
        ASSERT_EQ(ed247_stream_get_sampling_period_us(stream), (uint32_t)20000);

      } else if (name == "Stream5full") {
        ASSERT_EQ(ed247_stream_get_direction(stream), ED247_DIRECTION_OUT);
        ASSERT_EQ(ed247_stream_get_type(stream), ED247_STREAM_TYPE_ANALOG);
        ASSERT_STREQ(ed247_stream_get_comment(stream), "Analog input signals");
        ASSERT_STREQ(ed247_stream_get_icd(stream), "ICD for ANA");
        ASSERT_EQ(ed247_stream_get_uid(stream), (ed247_uid_t)104);
        ASSERT_EQ(ed247_stream_get_sample_max_number(stream), (uint32_t)36);
        ASSERT_EQ(ed247_stream_get_sample_max_size_bytes(stream), (uint32_t)16);
        ASSERT_EQ(ed247_stream_get_sampling_period_us(stream), (uint32_t)25000);

      } else if (name == "Stream6") {
        ASSERT_EQ(ed247_stream_get_direction(stream), ED247_DIRECTION_OUT);
        ASSERT_EQ(ed247_stream_get_type(stream), ED247_STREAM_TYPE_NAD);
        ASSERT_STREQ(ed247_stream_get_comment(stream), "");
        ASSERT_STREQ(ed247_stream_get_icd(stream), "");
        ASSERT_EQ(ed247_stream_get_uid(stream), (ed247_uid_t)5);
        ASSERT_EQ(ed247_stream_get_sample_max_number(stream), (uint32_t)1);
        ASSERT_EQ(ed247_stream_get_sample_max_size_bytes(stream), (uint32_t)6);
        ASSERT_EQ(ed247_stream_get_sampling_period_us(stream), (uint32_t)100000);

      } else if (name == "Stream6full") {
        ASSERT_EQ(ed247_stream_get_direction(stream), ED247_DIRECTION_OUT);
        ASSERT_EQ(ed247_stream_get_type(stream), ED247_STREAM_TYPE_NAD);
        ASSERT_STREQ(ed247_stream_get_comment(stream), "Non Avionic Data input signals");
        ASSERT_STREQ(ed247_stream_get_icd(stream), "ICD for NAD");
        ASSERT_EQ(ed247_stream_get_uid(stream), (ed247_uid_t)105);
        ASSERT_EQ(ed247_stream_get_sample_max_number(stream), (uint32_t)23);
        ASSERT_EQ(ed247_stream_get_sample_max_size_bytes(stream), (uint32_t)15);
        ASSERT_EQ(ed247_stream_get_sampling_period_us(stream), (uint32_t)110000);

      } else if (name == "Stream7") {
        ASSERT_EQ(ed247_stream_get_direction(stream), ED247_DIRECTION_OUT);
        ASSERT_EQ(ed247_stream_get_type(stream), ED247_STREAM_TYPE_VNAD);
        ASSERT_STREQ(ed247_stream_get_comment(stream), "");
        ASSERT_STREQ(ed247_stream_get_icd(stream), "");
        ASSERT_EQ(ed247_stream_get_uid(stream), (ed247_uid_t)6);
        ASSERT_EQ(ed247_stream_get_sample_max_number(stream), (uint32_t)1);
        ASSERT_EQ(ed247_stream_get_sample_max_size_bytes(stream), (uint32_t)12);
        ASSERT_EQ(ed247_stream_get_sampling_period_us(stream), (uint32_t)500000);

      } else if (name == "Stream7full") {
        ASSERT_EQ(ed247_stream_get_direction(stream), ED247_DIRECTION_OUT);
        ASSERT_EQ(ed247_stream_get_type(stream), ED247_STREAM_TYPE_VNAD);
        ASSERT_STREQ(ed247_stream_get_comment(stream), "Variable Non Avionic Data input signals");
        ASSERT_STREQ(ed247_stream_get_icd(stream), "ICD for VNAD");
        ASSERT_EQ(ed247_stream_get_uid(stream), (ed247_uid_t)106);
        ASSERT_EQ(ed247_stream_get_sample_max_number(stream), (uint32_t)12);
        ASSERT_EQ(ed247_stream_get_sample_max_size_bytes(stream), (uint32_t)13);
        ASSERT_EQ(ed247_stream_get_sampling_period_us(stream), (uint32_t)1000000);
      } else {
        SAY("Unkwonn stream " << name << "!");
        ASSERT_TRUE(false);
      }
    }
    ASSERT_EQ(stream_found.size(), 16);

    // Check the end of the list is reached and that on next request it will restart from the beginning
    ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
    ASSERT_NE(stream, nullptr);

    // Remove this list
    ASSERT_EQ(ed247_stream_list_free(NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);

    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
}

TEST(UtApiStreams, CheckFindStreamMethod)
{
    ed247_context_t context;
    ed247_channel_list_t channel_list;
    ed247_channel_t channel;
    ed247_stream_list_t stream_list;
    ed247_stream_t stream, stream_test;

    std::string filepath = config_path+"/ecic_unit_api_streams_multiple_channels.xml";
    SAY("Load ECIC " << filepath);
    ASSERT_EQ(ed247_load_file(filepath.c_str(), &context), ED247_STATUS_SUCCESS);

    ASSERT_EQ(ed247_find_channels(context, "MultipleStreamsChannel", &channel_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_list_free(channel_list), ED247_STATUS_SUCCESS);

    ASSERT_EQ(ed247_find_streams(NULL, NULL, &stream_list), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_find_streams(context, NULL, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_find_streams(context, ".*[", &stream_list), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_find_streams(context, NULL, &stream_list), ED247_STATUS_SUCCESS);

    // Get a single stream, check invalid calls
    ASSERT_EQ(ed247_get_stream(NULL, "Stream2", &stream_test), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_get_stream(context, NULL, &stream_test), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_get_stream(context, "", &stream_test), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_get_stream(context, "Stream2", NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_get_stream(context, "Stream2", &stream_test), ED247_STATUS_SUCCESS);

    std::set<std::string> stream_found;
    while(true) {
      ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
      if (stream == nullptr) break;
      std::string name = ed247_stream_get_name(stream);
      stream_found.insert(name);

      if (name != "Stream2" && name != "Stream3" && name != "Stream1" && name != "Stream8" && name != "Stream4" && name != "Stream5" &&
          name != "Stream6" && name != "Stream7" && name != "Stream12" && name != "Stream13" && name != "Stream11" && name != "Stream18" &&
          name != "Stream14" && name != "Stream15" && name != "Stream16" && name != "Stream17")
      {
        SAY("Unexpected stream " << name << "!");
        ASSERT_TRUE(false);
      }
    }
    ASSERT_EQ(stream_found.size(), 16);

    // Check the end of the list is reached and that on next request it will restart from the beginning
    ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
    ASSERT_NE(stream, nullptr);

    // Remove this list
    ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);

    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
}


TEST(UtApiStreams, CheckGetStreamFromContext)
{
    ed247_context_t context;
    ed247_channel_list_t channel_list;
    ed247_channel_t channel;
    ed247_stream_list_t stream_list;
    ed247_stream_t stream;

    std::string filepath = config_path+"/ecic_unit_api_streams_multiple_channels.xml";
    SAY("Load ECIC " << filepath);
    ASSERT_EQ(ed247_load_file(filepath.c_str(), &context), ED247_STATUS_SUCCESS);

    // Get the first channel and retrieve streams
    ASSERT_EQ(ed247_find_channels(context, "MultipleStreamsChannel", &channel_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_get_stream_list(NULL, &stream_list), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_get_stream_list(channel, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_get_stream_list(channel, &stream_list), ED247_STATUS_SUCCESS);
    std::map<std::string, int> stream_found;
    for (int i = 1; i < 9; i++) {
      ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
      ASSERT_NE(stream, nullptr);
      std::string name = ed247_stream_get_name(stream);
      ASSERT_TRUE(name == "Stream1" || name == "Stream2" || name == "Stream3" || name == "Stream4" ||
                  name == "Stream5" || name == "Stream6" || name == "Stream7" || name == "Stream8");
      stream_found[name] += 1;
    }
    for (auto stream_pair: stream_found) {
      ASSERT_EQ(stream_pair.second, 1);
    }

    // Check the end of the list is reached and that on next request it will restart from the beginning
    ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(stream, (ed247_stream_t)NULL);
    ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
    ASSERT_NE(stream, nullptr);

    // Remove this list
    ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_list_free(channel_list), ED247_STATUS_SUCCESS);

    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
}

TEST(UtApiStreams, CheckRegexStreamFromContext)
{
    ed247_context_t context;
    ed247_channel_list_t channel_list;
    ed247_channel_t channel;
    ed247_stream_list_t stream_list;
    ed247_stream_t stream, stream_test;

    std::string filepath = config_path+"/ecic_unit_api_streams_multiple_channels.xml";
    SAY("Load ECIC " << filepath);
    ASSERT_EQ(ed247_load_file(filepath.c_str(), &context), ED247_STATUS_SUCCESS);

    // Check channel list size
    ASSERT_EQ(ed247_find_channels(context, ".*", &channel_list), ED247_STATUS_SUCCESS);
    uint32_t size;
    ASSERT_EQ(ed247_channel_list_size(channel_list, &size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(size, (uint32_t)2);
    ASSERT_EQ(ed247_channel_list_free(channel_list), ED247_STATUS_SUCCESS);

    // Get a single channel and retrieve streams
    ASSERT_EQ(ed247_find_channels(context, "MultipleStreamsChannel2", &channel_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_find_streams(NULL, NULL, &stream_list), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_find_streams(channel, NULL, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_find_streams(channel, ".*[", &stream_list), ED247_STATUS_FAILURE);

    if(atoi("This case cannot be unrolled because gcc does not supports regex before version 4.9.X") != 0)
    {
        ASSERT_EQ(ed247_channel_find_streams(channel, "Stream1[1357]", &stream_list), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
        ASSERT_STREQ(ed247_stream_get_name(stream), "Stream11");

        ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
        ASSERT_STREQ(ed247_stream_get_name(stream), "Stream13");

        ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
        ASSERT_STREQ(ed247_stream_get_name(stream), "Stream15");

        ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
        ASSERT_STREQ(ed247_stream_get_name(stream), "Stream17");

        // Check the end of the list is reached and that on next request it will restart from the beginning
        ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
        ASSERT_EQ(stream, (ed247_stream_t)NULL);
        ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
        ASSERT_STREQ(ed247_stream_get_name(stream), "Stream11");
    }
    else
    {
        ASSERT_EQ(ed247_channel_find_streams(channel, "Stream11", &stream_list), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
        ASSERT_STREQ(ed247_stream_get_name(stream), "Stream11");


        // Get a single stream, check invalid calls
        ASSERT_EQ(ed247_channel_get_stream(NULL, "Stream11", &stream_test), ED247_STATUS_FAILURE);
        ASSERT_EQ(ed247_channel_get_stream(channel, NULL, &stream_test), ED247_STATUS_FAILURE);
        ASSERT_EQ(ed247_channel_get_stream(channel, "", &stream_test), ED247_STATUS_FAILURE);
        ASSERT_EQ(ed247_channel_get_stream(channel, "Stream11", NULL), ED247_STATUS_FAILURE);
        ASSERT_EQ(ed247_channel_get_stream(channel, "Stream11", &stream_test), ED247_STATUS_SUCCESS);
        ASSERT_EQ(stream, stream_test);

    }

    // Remove this list
    ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_list_free(channel_list), ED247_STATUS_SUCCESS);

    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
}

int main(int argc, char **argv)
{
    if(argc >=1)
        config_path = argv[1];
    else
        config_path = "../config";

    SAY("Configuration path: " << config_path);

    ::testing::InitGoogleTest(&argc, argv);
    // ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
