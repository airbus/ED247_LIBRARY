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

class UtApiSignals : public ::testing::Test{};

/******************************************************************************
This first test case checks loading of the streams is correctly performed.
The basic getter for streams is partially validated as well as stream list
manipulation function.
******************************************************************************/
TEST(UtApiSignals, CheckSignalLoading)
{
    ed247_context_t context;
    ed247_signal_list_t signal_list;
    ed247_signal_t signal, signal2;
    uint32_t size;

    std::string filepath = config_path+"/ecic_unit_api_signals.xml";
    ASSERT_EQ(ed247_load_file(filepath.c_str(), &context), ED247_STATUS_SUCCESS);

    // First tests validate the parsing of the ecic file
    ASSERT_EQ(ed247_find_signals(NULL, NULL, &signal_list), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_signal_list_free(signal_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_signals(context, NULL, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_find_signals(context, ".*[", &signal_list), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_signal_list_free(signal_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_signals(context, NULL, &signal_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_signal_list_free(signal_list), ED247_STATUS_SUCCESS);


    // Get a single signal, check invalid calls
    ASSERT_EQ(ed247_get_signal(NULL, "SignalDisMin", &signal), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_get_signal(context, NULL, &signal), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_get_signal(context, "", &signal), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_get_signal(context, "SignalDisMin", NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_get_signal(context, "SignalDisMin", &signal), ED247_STATUS_SUCCESS);

    ASSERT_STREQ(ed247_signal_get_name(signal), "SignalDisMin");
    ASSERT_STREQ(ed247_signal_get_comment(signal), "");
    ASSERT_STREQ(ed247_signal_get_icd(signal), "");
    ASSERT_EQ(ed247_signal_get_type(signal), ED247_SIGNAL_TYPE_DISCRETE);
    ASSERT_EQ(ed247_signal_get_byte_offset(signal), (uint32_t)0);

    // Check user_data
    void *user_data = nullptr;
    ASSERT_EQ(ed247_signal_get_user_data(NULL, &user_data), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_signal_get_user_data(signal, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_signal_get_user_data(signal, &user_data), ED247_STATUS_SUCCESS);
    ASSERT_EQ(user_data, nullptr);
    void *user_data_set = malloc(sizeof(uint8_t));
    *(uint8_t*)user_data_set = 12;
    ASSERT_EQ(ed247_signal_set_user_data(signal, user_data_set), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_signal_get_user_data(signal, &user_data), ED247_STATUS_SUCCESS);
    ASSERT_EQ(*(uint8_t*)user_data_set, 12);
    ASSERT_EQ(ed247_signal_set_user_data(NULL, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_signal_set_user_data(signal, NULL), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_signal_get_user_data(signal, &user_data), ED247_STATUS_SUCCESS);
    ASSERT_EQ(user_data, nullptr);
    free(user_data_set);

    // Get signal list
    ASSERT_EQ(ed247_find_signals(context, NULL, &signal_list), ED247_STATUS_SUCCESS);

    // Check size
    ASSERT_EQ(ed247_signal_list_size(NULL, &size), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_signal_list_size(signal_list, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_signal_list_size(signal_list, &size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(size, (uint32_t)8);

    // Check all signals are presents
    for(int i = 0; i < 8; i++) {
      ASSERT_EQ(ed247_signal_list_next(signal_list, &signal2), ED247_STATUS_SUCCESS);
      ASSERT_NE(ed247_signal_get_name(signal2), nullptr);
      std::string signal_name = ed247_signal_get_name(signal2);
      bool found = false;

      if (signal_name == "SignalDisMin") {
        found = true;
        ASSERT_STREQ(ed247_signal_get_comment(signal2), "");
        ASSERT_STREQ(ed247_signal_get_icd(signal2), "");
        ASSERT_EQ(ed247_signal_get_type(signal2), ED247_SIGNAL_TYPE_DISCRETE);
        ASSERT_EQ(ed247_signal_get_byte_offset(signal2), (uint32_t)0);
        ASSERT_EQ(signal, signal2);   // signal shall be the same as the one we found by get method
      }
      if (signal_name == "SignalDisMax") {
        found = true;
        ASSERT_STREQ(ed247_signal_get_comment(signal2), "is");
        ASSERT_STREQ(ed247_signal_get_icd(signal2), "This");
        ASSERT_EQ(ed247_signal_get_type(signal2), ED247_SIGNAL_TYPE_DISCRETE);
        ASSERT_EQ(ed247_signal_get_byte_offset(signal2), (uint32_t)1);
      }
      if (signal_name == "SignalAnaMin") {
        found = true;
        ASSERT_STREQ(ed247_signal_get_comment(signal2), "");
        ASSERT_STREQ(ed247_signal_get_icd(signal2), "");
        ASSERT_EQ(ed247_signal_get_type(signal2), ED247_SIGNAL_TYPE_ANALOG);
        ASSERT_EQ(ed247_signal_get_byte_offset(signal2), (uint32_t)0);
        ASSERT_STREQ(ed247_signal_analogue_get_electrical_unit(signal2), "");
      }
      if (signal_name == "SignalAnaMax") {
        found = true;
        ASSERT_STREQ(ed247_signal_get_comment(signal2), "very");
        ASSERT_STREQ(ed247_signal_get_icd(signal2), "a");
        ASSERT_EQ(ed247_signal_get_type(signal2), ED247_SIGNAL_TYPE_ANALOG);
        ASSERT_EQ(ed247_signal_get_byte_offset(signal2), (uint32_t)4);
        ASSERT_STREQ(ed247_signal_analogue_get_electrical_unit(signal2), "mV");
      }
      if (signal_name == "SignalNADmin") {
        found = true;
        ASSERT_STREQ(ed247_signal_get_comment(signal2), "");
        ASSERT_STREQ(ed247_signal_get_icd(signal2), "");
        ASSERT_EQ(ed247_signal_get_type(signal2), ED247_SIGNAL_TYPE_NAD);
        ASSERT_EQ(ed247_signal_get_byte_offset(signal2), (uint32_t)0);
        ASSERT_EQ(ed247_signal_nad_get_type(signal2), ED247_NAD_TYPE_UINT16);
        ASSERT_EQ(ed247_signal_nad_get_dimensions_count(signal2), (uint32_t)1);
        ASSERT_TRUE(ed247_signal_nad_get_dimension(signal, 0) == (uint32_t)1);
        ASSERT_STREQ(ed247_signal_nad_get_unit(signal2), "");
      }
      if (signal_name == "SignalNADmax") {
        found = true;
        ASSERT_STREQ(ed247_signal_get_comment(signal2), "tiny");
        ASSERT_STREQ(ed247_signal_get_icd(signal2), "simple");
        ASSERT_EQ(ed247_signal_get_type(signal2), ED247_SIGNAL_TYPE_NAD);
        ASSERT_EQ(ed247_signal_get_byte_offset(signal2), (uint32_t)2);
        ASSERT_EQ(ed247_signal_nad_get_type(signal2), ED247_NAD_TYPE_INT64);
        ASSERT_EQ(ed247_signal_nad_get_dimensions_count(signal2), (uint32_t)2);
        ASSERT_EQ(ed247_signal_nad_get_dimension(signal2, 0), (uint32_t)10);
        ASSERT_EQ(ed247_signal_nad_get_dimension(signal2, 1), (uint32_t)20);
        ASSERT_STREQ(ed247_signal_nad_get_unit(signal2), "pixels");
      }
      if (signal_name == "SignalVNADmax") {
        found = true;
        ASSERT_STREQ(ed247_signal_get_comment(signal2), "test");
        ASSERT_STREQ(ed247_signal_get_icd(signal2), "little");
        ASSERT_EQ(ed247_signal_get_type(signal2), ED247_SIGNAL_TYPE_VNAD);
        ASSERT_EQ(ed247_signal_nad_get_type(signal2), ED247_NAD_TYPE_FLOAT64);
        ASSERT_EQ(ed247_signal_vnad_get_max_number(signal2), (uint32_t)7);
        ASSERT_EQ(ed247_signal_vnad_get_position(signal2), (uint32_t)1);
        ASSERT_STREQ(ed247_signal_nad_get_unit(signal2), "Mpc");
      }
      if (signal_name == "SignalVNADmin") {
        found = true;
        ASSERT_STREQ(ed247_signal_get_comment(signal2), "");
        ASSERT_STREQ(ed247_signal_get_icd(signal2), "");
        ASSERT_EQ(ed247_signal_get_type(signal2), ED247_SIGNAL_TYPE_VNAD);
        ASSERT_EQ(ed247_signal_nad_get_type(signal2), ED247_NAD_TYPE_FLOAT32);
        ASSERT_EQ(ed247_signal_vnad_get_max_number(signal2), (uint32_t)11);
        ASSERT_EQ(ed247_signal_vnad_get_position(signal2), (uint32_t)2);
        ASSERT_STREQ(ed247_signal_nad_get_unit(signal2), "");
      }

      ASSERT_TRUE(found);
    }

    // Check the list is starting again from the beginning when the end is reached
    ASSERT_EQ(ed247_signal_list_next(signal_list, &signal), ED247_STATUS_SUCCESS);
    ASSERT_EQ(signal, nullptr);
    ASSERT_EQ(ed247_signal_list_next(signal_list, &signal), ED247_STATUS_SUCCESS);
    ASSERT_NE(signal, nullptr);

    ASSERT_EQ(ed247_signal_list_free(signal_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
}

TEST(UtApiSignals, CheckOtherMethods)
{
    ed247_context_t context;
    ed247_stream_list_t stream_list;
    ed247_stream_t stream;
    ed247_signal_list_t signal_list;
    ed247_signal_t signal, signal_test;

    std::string filepath = config_path+"/ecic_unit_api_signals.xml";
    ASSERT_EQ(ed247_load_file(filepath.c_str(), &context), ED247_STATUS_SUCCESS);

    // Get the signals from Stream1
    ASSERT_EQ(ed247_find_streams(context, "Stream1", &stream_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_signal_list(NULL, &signal_list), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_signal_list_free(signal_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_get_signal_list(stream, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_get_signal_list(stream, &signal_list), ED247_STATUS_SUCCESS);

    ASSERT_EQ(ed247_signal_list_next(signal_list, &signal), ED247_STATUS_SUCCESS);
    ASSERT_STREQ(ed247_signal_get_name(signal), "SignalDisMin");

    ASSERT_EQ(ed247_signal_list_next(signal_list, &signal), ED247_STATUS_SUCCESS);
    ASSERT_STREQ(ed247_signal_get_name(signal), "SignalDisMax");

    ASSERT_EQ(ed247_signal_list_free(signal_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);

    // Check the presence of the signals from Stream2
    ASSERT_EQ(ed247_find_streams(context, "Stream2", &stream_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_find_signals(NULL, NULL, &signal_list), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_signal_list_free(signal_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_find_signals(stream, NULL, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_find_signals(stream, ".*[)", &signal_list), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_signal_list_free(signal_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_find_signals(stream, NULL, &signal_list), ED247_STATUS_SUCCESS);

    ASSERT_EQ(ed247_signal_list_next(signal_list, &signal), ED247_STATUS_SUCCESS);
    ASSERT_STREQ(ed247_signal_get_name(signal), "SignalAnaMin");

    ASSERT_EQ(ed247_stream_get_signal(NULL, "SignalAnaMin", &signal_test), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_get_signal(stream, NULL, &signal_test), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_get_signal(stream, "", &signal_test), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_get_signal(stream, "SignalAnaMin", NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_get_signal(stream, "SignalAnaMin", &signal_test), ED247_STATUS_SUCCESS);
    ASSERT_EQ(signal, signal_test);

    ASSERT_EQ(ed247_signal_list_next(signal_list, &signal), ED247_STATUS_SUCCESS);
    ASSERT_STREQ(ed247_signal_get_name(signal), "SignalAnaMax");

    // Check the list is starting again from the beginning when the end is reached
    ASSERT_EQ(ed247_signal_list_next(signal_list, &signal), ED247_STATUS_SUCCESS);
    ASSERT_EQ(signal, (ed247_signal_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signal_list, &signal), ED247_STATUS_SUCCESS);
    ASSERT_STREQ(ed247_signal_get_name(signal), "SignalAnaMin");

    // Remove this list
    ASSERT_EQ(ed247_signal_list_free(signal_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);

    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
}

TEST(UtApiSignals, DetectSignalsInStream)
{
    ed247_context_t context;
    ed247_stream_list_t stream_list;
    ed247_stream_t stream;
    uint8_t yes_no;

    std::string filepath = config_path+"/ecic_unit_api_signals.xml";
    ASSERT_EQ(ed247_load_file(filepath.c_str(), &context), ED247_STATUS_SUCCESS);

    // Get the streams and request if it contains signals for each of them
    // The first checks verify invalid calls
    ASSERT_EQ(ed247_find_streams(context, NULL, &stream_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
    ASSERT_STREQ(ed247_stream_get_name(stream), "StreamA429");
    ASSERT_EQ(ed247_stream_has_signals(NULL, &yes_no), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_has_signals(stream, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_has_signals(stream, &yes_no), ED247_STATUS_SUCCESS);
    ASSERT_EQ(yes_no, (uint8_t)0);

    ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
    ASSERT_STREQ(ed247_stream_get_name(stream), "StreamA825");
    ASSERT_EQ(ed247_stream_has_signals(stream, &yes_no), ED247_STATUS_SUCCESS);
    ASSERT_EQ(yes_no, (uint8_t)0);

    ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
    ASSERT_STREQ(ed247_stream_get_name(stream), "StreamA664");
    ASSERT_EQ(ed247_stream_has_signals(stream, &yes_no), ED247_STATUS_SUCCESS);
    ASSERT_EQ(yes_no, (uint8_t)0);

    ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
    ASSERT_STREQ(ed247_stream_get_name(stream), "StreamSERIAL");
    ASSERT_EQ(ed247_stream_has_signals(stream, &yes_no), ED247_STATUS_SUCCESS);
    ASSERT_EQ(yes_no, (uint8_t)0);

    ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
    ASSERT_STREQ(ed247_stream_get_name(stream), "Stream1");
    ASSERT_EQ(ed247_stream_has_signals(stream, &yes_no), ED247_STATUS_SUCCESS);
    ASSERT_EQ(yes_no, (uint8_t)1);

    ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
    ASSERT_STREQ(ed247_stream_get_name(stream), "Stream2");
    ASSERT_EQ(ed247_stream_has_signals(stream, &yes_no), ED247_STATUS_SUCCESS);
    ASSERT_EQ(yes_no, (uint8_t)1);

    ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
    ASSERT_STREQ(ed247_stream_get_name(stream), "Stream3");
    ASSERT_EQ(ed247_stream_has_signals(stream, &yes_no), ED247_STATUS_SUCCESS);
    ASSERT_EQ(yes_no, (uint8_t)1);

    ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
    ASSERT_STREQ(ed247_stream_get_name(stream), "Stream4");
    ASSERT_EQ(ed247_stream_has_signals(stream, &yes_no), ED247_STATUS_SUCCESS);
    ASSERT_EQ(yes_no, (uint8_t)1);


    ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);

    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
}


TEST(UtApiSignals, DuplicateSignalName)
{
    ed247_context_t context;
    std::string filepath = config_path+"/ecic_unit_api_duplicate_signals.xml";
    ASSERT_EQ(ed247_load_file(filepath.c_str(), &context), ED247_STATUS_FAILURE);
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
