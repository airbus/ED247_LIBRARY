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
This test checks the api functions that manipulate channels
******************************************************************************/
class UtApiChannel : public ::testing::Test{};

TEST(UtApiChannel, ChannelsManipulation)
{
    ed247_context_t context;
    ed247_channel_list_t channel_list;
    ed247_channel_t channel;
    ed247_channel_t channel_test;

    std::string filepath = config_path+"/ecic_unit_api_channels.xml";
    ASSERT_EQ(ed247_load_file(filepath.c_str(), &context), ED247_STATUS_SUCCESS);

    // Get the channel list, check invalid calls
    ASSERT_EQ(ed247_find_channels (NULL, ".*", &channel_list), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_list_free(channel_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_channels (context, ".*", NULL), ED247_STATUS_FAILURE);
	ASSERT_EQ(ed247_find_channels (context, ".*[", &channel_list), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_list_free(channel_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_find_channels (context, NULL, &channel_list), ED247_STATUS_SUCCESS);

    // Get channel list size, chek invalid calls
    uint32_t size;
    ASSERT_EQ(ed247_channel_list_size(NULL, &size), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_list_size(channel_list, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_list_size(channel_list, &size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(size, (uint32_t)4);

    // Retrieve the first channel, check invalid calls
    ASSERT_EQ(ed247_channel_list_next(NULL, &channel), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_list_next(channel_list, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);

    // Check user_data
    void *user_data = nullptr;
    ASSERT_EQ(ed247_channel_get_user_data(NULL, &user_data), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_get_user_data(channel, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_get_user_data(channel, &user_data), ED247_STATUS_SUCCESS);
    ASSERT_EQ(user_data, nullptr);
    void *user_data_set = malloc(sizeof(uint8_t));
    *(uint8_t*)user_data_set = 12;
    ASSERT_EQ(ed247_channel_set_user_data(channel, user_data_set), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_get_user_data(channel, &user_data), ED247_STATUS_SUCCESS);
    ASSERT_EQ(*(uint8_t*)user_data_set, 12);
    ASSERT_EQ(ed247_channel_set_user_data(NULL, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_set_user_data(channel, NULL), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_get_user_data(channel, &user_data), ED247_STATUS_SUCCESS);
    ASSERT_EQ(user_data, nullptr);
    free(user_data_set);
    ASSERT_EQ(ed247_channel_list_free(channel_list), ED247_STATUS_SUCCESS);

    // Validate content
    ASSERT_EQ(ed247_find_channels (context, NULL, &channel_list), ED247_STATUS_SUCCESS);
    std::map<std::string, uint32_t> channel_found;
    for(int i = 0; i < 4; i++) {
      ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
      ASSERT_NE(channel, nullptr);
      std::string name = ed247_channel_get_name(channel);
      if (name == "DefaultValues") {
        // Check the content of retrieved information, this channel shall use plenty of default values
        ASSERT_TRUE(strcmp(ed247_channel_get_comment(channel), "") == 0);
        ASSERT_EQ(ed247_channel_get_frame_standard_revision(channel), ED247_STANDARD_ED247A);
        ASSERT_EQ(ed247_get_channel(context, "DefaultValues", &channel_test), ED247_STATUS_SUCCESS);
        ASSERT_EQ(channel, channel_test);
      } else if (name == "DummyChannel1" || name == "DummyChannel2") {
        // Ok
      } else if (name == "FilledChannel") {
        ASSERT_TRUE(strcmp(ed247_channel_get_comment(channel), "This is a test") == 0);
        ASSERT_EQ(ed247_channel_get_frame_standard_revision(channel), ED247_STANDARD_ED247A);
      } else {
        ASSERT_TRUE(false);
      }
    }
    for (auto& pair: channel_found) ASSERT_EQ(pair.second, 1);    // we found each channel once

    // Try to get the next channel while there is no other
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(channel, (ed247_channel_t)NULL);

    // Check the list restarts from start
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_NE(channel, nullptr);
    ASSERT_EQ(ed247_channel_list_free(channel_list), ED247_STATUS_SUCCESS);


    // Check the find routine with other kinds of requests
    ASSERT_EQ(ed247_find_channels (context, "", &channel_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_list_free(channel_list), ED247_STATUS_SUCCESS);

    // Make a more selective selection
    ASSERT_EQ(ed247_find_channels (context, ".*Dummy.*", &channel_list), ED247_STATUS_SUCCESS);
    channel_found.clear();
    for(int i = 0; i < 2; i++) {
      ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
      ASSERT_NE(channel, nullptr);
      std::string name = ed247_channel_get_name(channel);
      ASSERT_TRUE(name == "DummyChannel1" || name == "DummyChannel2");
    }
    for (auto& pair: channel_found) ASSERT_EQ(pair.second, 1);    // we found each channel once

    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(channel, (ed247_channel_t)NULL);
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_NE(channel, nullptr);
    ASSERT_EQ(ed247_channel_list_free(channel_list), ED247_STATUS_SUCCESS);

    // Select a single channel
    ASSERT_EQ(ed247_find_channels (context, "FilledChannel", &channel_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(strcmp(ed247_channel_get_name(channel), "FilledChannel") == 0);
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(channel, (ed247_channel_t)NULL);
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(strcmp(ed247_channel_get_name(channel), "FilledChannel") == 0);
    ASSERT_EQ(ed247_channel_list_free(channel_list), ED247_STATUS_SUCCESS);

    // Make a too selective request
    ASSERT_EQ(ed247_find_channels (context, "ChannelThatDoesNotExist", &channel_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(channel, (ed247_channel_t)NULL);
    ASSERT_EQ(ed247_channel_list_free(channel_list), ED247_STATUS_SUCCESS);

    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);

}

TEST(UtApiChannel,GetChannelList)
{
    ed247_context_t context;
    ed247_channel_list_t channel_list;
    ed247_channel_t channel;

    std::string filepath = config_path+"/ecic_unit_api_channels.xml";
    ASSERT_EQ(ed247_load_file(filepath.c_str(), &context), ED247_STATUS_SUCCESS);

    // First tests validate the parsing of the ecic file
    ASSERT_EQ(ed247_get_channel_list(NULL, &channel_list), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_list_free(channel_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_get_channel_list(context, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_get_channel_list(context, &channel_list), ED247_STATUS_SUCCESS);

    // Retrieve the first channel, check invalid calls
    ASSERT_EQ(ed247_channel_list_next(NULL, &channel), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_list_next(channel_list, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);

    // Retrieve The channel information of the first channel, check invalid calls
    ASSERT_EQ(ed247_channel_list_free(channel_list), ED247_STATUS_SUCCESS);

    ASSERT_EQ(ed247_get_channel_list(context, &channel_list), ED247_STATUS_SUCCESS);
    std::map<std::string, uint32_t> channel_found;
    for(int i = 0; i < 4; i++) {
      ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
      ASSERT_NE(channel, nullptr);
      std::string name = ed247_channel_get_name(channel);
      ASSERT_TRUE(name == "DefaultValues" || name == "DummyChannel1" || name == "DummyChannel2" || name == "FilledChannel");
    }
    for (auto& pair: channel_found) ASSERT_EQ(pair.second, 1);    // we found each channel once
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(channel, nullptr);
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
    return RUN_ALL_TESTS();
}
