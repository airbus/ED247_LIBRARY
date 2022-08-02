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
    ed247_stream_list_t stream_list;
    ed247_stream_t stream;
    ed247_stream_t stream_test;
    ed247_signal_list_t signal_list;
    ed247_signal_t signal, signal_test;
    const ed247_signal_info_t* signal_info;
    
    std::string filepath = config_path+"/ecic_unit_api_signals.xml";
    ASSERT_EQ(ed247_load_file(filepath.c_str(), &context), ED247_STATUS_SUCCESS);

    // Find stream
    ASSERT_EQ(ed247_find_streams(context, "Stream1", &stream_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);
    
    // First tests validate the parsing of the ecic file
    ASSERT_EQ(ed247_find_signals(NULL, NULL, &signal_list), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_signal_list_free(signal_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_signals(context, NULL, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_find_signals(context, ".*[", &signal_list), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_signal_list_free(signal_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_signals(context, NULL, &signal_list), ED247_STATUS_SUCCESS);

    // Check size
    size_t size;
    ASSERT_EQ(ed247_signal_list_size(NULL, &size), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_signal_list_size(signal_list, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_signal_list_size(signal_list, &size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(size, (size_t)8);
    
    // Check the retrieved content, perform invalid calls to verify robustness
    ASSERT_EQ(ed247_signal_list_next(signal_list, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_signal_list_next(NULL, &signal), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_signal_list_next(signal_list, &signal), ED247_STATUS_SUCCESS);

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

    // Get a single channel, check invalid calls
    ASSERT_EQ(ed247_get_signal(NULL, "SignalDisMin", &signal_test), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_get_signal(context, NULL, &signal_test), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_get_signal(context, "", &signal_test), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_get_signal(context, "SignalDisMin", NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_get_signal(context, "SignalDisMin", &signal_test), ED247_STATUS_SUCCESS);
    ASSERT_EQ(signal, signal_test);
    
    // Get the signal info and check the values, perform unvalid calls to verify robustness
    ASSERT_EQ(ed247_signal_get_info(NULL, &signal_info), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_signal_get_info(signal, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_signal_get_info(signal, &signal_info), ED247_STATUS_SUCCESS);
    
    ASSERT_TRUE(signal_info->name != NULL && strcmp(signal_info->name, "SignalDisMin") == 0);
    ASSERT_TRUE(signal_info->comment != NULL && strcmp(signal_info->comment, "") == 0);
    ASSERT_TRUE(signal_info->icd != NULL && strcmp(signal_info->icd, "") == 0);
    ASSERT_EQ(signal_info->type, ED247_SIGNAL_TYPE_DISCRETE);
    ASSERT_EQ(signal_info->info.dis.byte_offset, (uint32_t)0);

    ASSERT_EQ(ed247_signal_get_stream(NULL, &stream_test), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_signal_get_stream(signal, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_signal_get_stream(signal, &stream_test), ED247_STATUS_SUCCESS);
    ASSERT_EQ(stream, stream_test);
    
    ASSERT_EQ(ed247_signal_list_next(signal_list, &signal), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_signal_get_info(signal, &signal_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(signal_info->name != NULL && strcmp(signal_info->name, "SignalDisMax") == 0);
    ASSERT_TRUE(signal_info->comment != NULL && strcmp(signal_info->comment, "is") == 0);
    ASSERT_TRUE(signal_info->icd != NULL && strcmp(signal_info->icd, "This") == 0);
    ASSERT_EQ(signal_info->type, ED247_SIGNAL_TYPE_DISCRETE);
    ASSERT_EQ(signal_info->info.dis.byte_offset, (uint32_t)1);
    
    ASSERT_EQ(ed247_signal_list_next(signal_list, &signal), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_signal_get_info(signal, &signal_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(signal_info->name != NULL && strcmp(signal_info->name, "SignalAnaMin") == 0);
    ASSERT_TRUE(signal_info->comment != NULL && strcmp(signal_info->comment, "") == 0);
    ASSERT_TRUE(signal_info->icd != NULL && strcmp(signal_info->icd, "") == 0);
    ASSERT_EQ(signal_info->type, ED247_SIGNAL_TYPE_ANALOG);
    ASSERT_EQ(signal_info->info.ana.byte_offset, (uint32_t)0);
    ASSERT_TRUE(signal_info->info.ana.electrical_unit != NULL && strcmp(signal_info->info.ana.electrical_unit, "") == 0);
    
    ASSERT_EQ(ed247_signal_list_next(signal_list, &signal), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_signal_get_info(signal, &signal_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(signal_info->name != NULL && strcmp(signal_info->name, "SignalAnaMax") == 0);
    ASSERT_TRUE(signal_info->comment != NULL && strcmp(signal_info->comment, "very") == 0);
    ASSERT_TRUE(signal_info->icd != NULL && strcmp(signal_info->icd, "a") == 0);
    ASSERT_EQ(signal_info->type, ED247_SIGNAL_TYPE_ANALOG);
    ASSERT_EQ(signal_info->info.ana.byte_offset, (uint32_t)4);
    ASSERT_TRUE(signal_info->info.ana.electrical_unit != NULL && strcmp(signal_info->info.ana.electrical_unit, "mV") == 0);
    
    ASSERT_EQ(ed247_signal_list_next(signal_list, &signal), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_signal_get_info(signal, &signal_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(signal_info->name != NULL && strcmp(signal_info->name, "SignalNADmin") == 0);
    ASSERT_TRUE(signal_info->comment != NULL && strcmp(signal_info->comment, "") == 0);
    ASSERT_TRUE(signal_info->icd != NULL && strcmp(signal_info->icd, "") == 0);
    ASSERT_EQ(signal_info->type, ED247_SIGNAL_TYPE_NAD);
    ASSERT_EQ(signal_info->info.nad.byte_offset, (uint32_t)0);
    ASSERT_EQ(signal_info->info.nad.nad_type, ED247_NAD_TYPE_UINT16);
    ASSERT_EQ(signal_info->info.nad.dimensions_count, (uint32_t)1);
    ASSERT_TRUE(signal_info->info.nad.dimensions[0] == (uint32_t)1);
    ASSERT_TRUE(signal_info->info.nad.unit != NULL && strcmp(signal_info->info.nad.unit, "") == 0);
    
    ASSERT_EQ(ed247_signal_list_next(signal_list, &signal), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_signal_get_info(signal, &signal_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(signal_info->name != NULL && strcmp(signal_info->name, "SignalNADmax") == 0);
    ASSERT_TRUE(signal_info->comment != NULL && strcmp(signal_info->comment, "tiny") == 0);
    ASSERT_TRUE(signal_info->icd != NULL && strcmp(signal_info->icd, "simple") == 0);
    ASSERT_EQ(signal_info->type, ED247_SIGNAL_TYPE_NAD);
    ASSERT_EQ(signal_info->info.nad.byte_offset, (uint32_t)2);
    ASSERT_EQ(signal_info->info.nad.nad_type, ED247_NAD_TYPE_INT64);
    ASSERT_EQ(signal_info->info.nad.dimensions_count, (uint32_t)2);
    ASSERT_EQ(signal_info->info.nad.dimensions[0], (uint32_t)10);
    ASSERT_EQ(signal_info->info.nad.dimensions[1], (uint32_t)20);
    ASSERT_TRUE(signal_info->info.nad.unit != NULL && strcmp(signal_info->info.nad.unit, "pixels") == 0);
    
    ASSERT_EQ(ed247_signal_list_next(signal_list, &signal), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_signal_get_info(signal, &signal_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(signal_info->name != NULL && strcmp(signal_info->name, "SignalVNADmax") == 0);
    ASSERT_TRUE(signal_info->comment != NULL && strcmp(signal_info->comment, "test") == 0);
    ASSERT_TRUE(signal_info->icd != NULL && strcmp(signal_info->icd, "little") == 0);
    ASSERT_EQ(signal_info->type, ED247_SIGNAL_TYPE_VNAD);
    ASSERT_EQ(signal_info->info.vnad.nad_type, ED247_NAD_TYPE_FLOAT64);
    ASSERT_EQ(signal_info->info.vnad.max_length, (uint32_t)7);
    ASSERT_EQ(signal_info->info.vnad.position, (uint32_t)1);
    ASSERT_TRUE(signal_info->info.vnad.unit != NULL && strcmp(signal_info->info.vnad.unit, "Mpc") == 0);
    
    ASSERT_EQ(ed247_signal_list_next(signal_list, &signal), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_signal_get_info(signal, &signal_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(signal_info->name != NULL && strcmp(signal_info->name, "SignalVNADmin") == 0);
    ASSERT_TRUE(signal_info->comment != NULL && strcmp(signal_info->comment, "") == 0);
    ASSERT_TRUE(signal_info->icd != NULL && strcmp(signal_info->icd, "") == 0);
    ASSERT_EQ(signal_info->type, ED247_SIGNAL_TYPE_VNAD);
    ASSERT_EQ(signal_info->info.vnad.nad_type, ED247_NAD_TYPE_FLOAT32);
    ASSERT_EQ(signal_info->info.vnad.max_length, (uint32_t)11);
    ASSERT_EQ(signal_info->info.vnad.position, (uint32_t)2);
    ASSERT_TRUE(signal_info->info.vnad.unit != NULL && strcmp(signal_info->info.nad.unit, "") == 0);
    
    // Check the list is starting again from the beginning when the end is reached
    ASSERT_EQ(ed247_signal_list_next(signal_list, &signal), ED247_STATUS_SUCCESS);
    ASSERT_EQ(signal, (ed247_signal_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signal_list, &signal), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_signal_get_info(signal, &signal_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(signal_info->name != NULL && strcmp(signal_info->name, "SignalDisMin") == 0);
    
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
    const ed247_signal_info_t* signal_info;
    
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
    ASSERT_EQ(ed247_signal_get_info(signal, &signal_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(signal_info->name != NULL && strcmp(signal_info->name, "SignalDisMin") == 0);
    
    ASSERT_EQ(ed247_signal_list_next(signal_list, &signal), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_signal_get_info(signal, &signal_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(signal_info->name != NULL && strcmp(signal_info->name, "SignalDisMax") == 0);
    
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
    ASSERT_EQ(ed247_signal_get_info(signal, &signal_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(signal_info->name != NULL && strcmp(signal_info->name, "SignalAnaMin") == 0);

    ASSERT_EQ(ed247_stream_get_signal(NULL, "SignalAnaMin", &signal_test), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_get_signal(stream, NULL, &signal_test), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_get_signal(stream, "", &signal_test), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_get_signal(stream, "SignalAnaMin", NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_get_signal(stream, "SignalAnaMin", &signal_test), ED247_STATUS_SUCCESS);
    ASSERT_EQ(signal, signal_test);
    
    ASSERT_EQ(ed247_signal_list_next(signal_list, &signal), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_signal_get_info(signal, &signal_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(signal_info->name != NULL && strcmp(signal_info->name, "SignalAnaMax") == 0);
    
    // Check the list is starting again from the beginning when the end is reached
    ASSERT_EQ(ed247_signal_list_next(signal_list, &signal), ED247_STATUS_SUCCESS);
    ASSERT_EQ(signal, (ed247_signal_t)NULL);
    ASSERT_EQ(ed247_signal_list_next(signal_list, &signal), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_signal_get_info(signal, &signal_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(signal_info->name != NULL && strcmp(signal_info->name, "SignalAnaMin") == 0);
    
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
    ASSERT_TRUE(strcmp(ed247_stream_get_name(stream), "StreamA429") == 0);
    ASSERT_EQ(ed247_stream_has_signals(NULL, &yes_no), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_has_signals(stream, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_stream_has_signals(stream, &yes_no), ED247_STATUS_SUCCESS);
    ASSERT_EQ(yes_no, (uint8_t)0);

    ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(strcmp(ed247_stream_get_name(stream), "StreamA825") == 0);
    ASSERT_EQ(ed247_stream_has_signals(stream, &yes_no), ED247_STATUS_SUCCESS);
    ASSERT_EQ(yes_no, (uint8_t)0);  
    
    ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(strcmp(ed247_stream_get_name(stream), "StreamA664") == 0);
    ASSERT_EQ(ed247_stream_has_signals(stream, &yes_no), ED247_STATUS_SUCCESS);
    ASSERT_EQ(yes_no, (uint8_t)0);

    ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(strcmp(ed247_stream_get_name(stream), "StreamSERIAL") == 0);
    ASSERT_EQ(ed247_stream_has_signals(stream, &yes_no), ED247_STATUS_SUCCESS);
    ASSERT_EQ(yes_no, (uint8_t)0);  
    
    ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(strcmp(ed247_stream_get_name(stream), "Stream1") == 0);
    ASSERT_EQ(ed247_stream_has_signals(stream, &yes_no), ED247_STATUS_SUCCESS);
    ASSERT_EQ(yes_no, (uint8_t)1);
    
    ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(strcmp(ed247_stream_get_name(stream), "Stream2") == 0);
    ASSERT_EQ(ed247_stream_has_signals(stream, &yes_no), ED247_STATUS_SUCCESS);
    ASSERT_EQ(yes_no, (uint8_t)1);
    
    ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(strcmp(ed247_stream_get_name(stream), "Stream3") == 0);
    ASSERT_EQ(ed247_stream_has_signals(stream, &yes_no), ED247_STATUS_SUCCESS);
    ASSERT_EQ(yes_no, (uint8_t)1);
    
    ASSERT_EQ(ed247_stream_list_next(stream_list, &stream), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(strcmp(ed247_stream_get_name(stream), "Stream4") == 0);
    ASSERT_EQ(ed247_stream_has_signals(stream, &yes_no), ED247_STATUS_SUCCESS);
    ASSERT_EQ(yes_no, (uint8_t)1);
    
    
    ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);
    
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
