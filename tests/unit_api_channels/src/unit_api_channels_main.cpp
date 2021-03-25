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

#include "ed247.h"
#include "sync_entity.h"

#include <stdio.h>
#include <fstream>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include "gtest/gtest.h"

#include <memory>

/***********
 * Defines *
 ***********/

/********
 * Test *
 ********/

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
    const ed247_channel_info_t* channel_info;
    
    std::string filepath = config_path+"/ecic_unit_api_channels.xml";
    ASSERT_EQ(ed247_load(filepath.c_str(), NULL, &context), ED247_STATUS_SUCCESS);
        
    // Get the channel list, check invalid calls
    ASSERT_EQ(ed247_find_channels (NULL, ".*", &channel_list), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_find_channels (context, ".*", NULL), ED247_STATUS_FAILURE);
	ASSERT_EQ(ed247_find_channels (context, ".*[", &channel_list), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_find_channels (context, NULL, &channel_list), ED247_STATUS_SUCCESS);

    // Get channel list size, chek invalid calls
    size_t size;
    ASSERT_EQ(ed247_channel_list_size(NULL, &size), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_list_size(channel_list, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_list_size(channel_list, &size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(size, (size_t)4);
    
    // Retrieve the first channel, check invalid calls
    ASSERT_EQ(ed247_channel_list_next(NULL, &channel), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_list_next(channel_list, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    
    // Retrieve The channel information of the first channel, check invalid calls
    ASSERT_EQ(ed247_channel_get_info(NULL, &channel_info), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_get_info(channel, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_get_info(channel, &channel_info), ED247_STATUS_SUCCESS);
    
    // Check the content of retrieved information, this channel shall use plenty of default values
    ASSERT_TRUE(channel_info->name != NULL && strcmp(channel_info->name, "DefaultValues") == 0);
    ASSERT_TRUE(channel_info->comment != NULL && strcmp(channel_info->comment, "") == 0);
    ASSERT_EQ(channel_info->frame_format.standard_revision, ED247_STANDARD_ED247A);

    // Get a single channel, check invalid calls
    ASSERT_EQ(ed247_get_channel(NULL, "DefaultValues", &channel_test), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_get_channel(context, NULL, &channel_test), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_get_channel(context, "", &channel_test), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_get_channel(context, "DefaultValues", NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_get_channel(context, "DefaultValues", &channel_test), ED247_STATUS_SUCCESS);
    ASSERT_EQ(channel, channel_test);
    
    // Get the next channel, get information and and confirm the name and values of the header
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_get_info(channel, &channel_info), ED247_STATUS_SUCCESS);
    ASSERT_FALSE(strcmp(channel_info->name, "DummyChannel1"));
    
    // Get the next channel, get information and and confirm the name and values of the header
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_get_info(channel, &channel_info), ED247_STATUS_SUCCESS);
    ASSERT_FALSE(strcmp(channel_info->name, "DummyChannel2"));
    
    // Get the next channel, get information and check the provided content
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_get_info(channel, &channel_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(channel_info->name != NULL && strcmp(channel_info->name, "FilledChannel") == 0);
    ASSERT_TRUE(channel_info->comment != NULL && strcmp(channel_info->comment, "This is a test") == 0);
    ASSERT_EQ(channel_info->frame_format.standard_revision, ED247_STANDARD_ED247A);
    
    // Try to get the next channel while there is no other
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(channel, (ed247_channel_t)NULL);
    
    // Check the list restarts from start
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_get_info(channel, &channel_info), ED247_STATUS_SUCCESS);
    ASSERT_FALSE(strcmp(channel_info->name, "DefaultValues"));
    
    // Free the list, check invalid calls
    ASSERT_EQ(ed247_channel_list_free(NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_list_free(channel_list), ED247_STATUS_SUCCESS);
    
    // Check the find routine with other kinds of requests
    ASSERT_EQ(ed247_find_channels (context, "", &channel_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    
    // Make a more selective selection
    ASSERT_EQ(ed247_find_channels (context, ".*Dummy.*", &channel_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_get_info(channel, &channel_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(channel_info->name != NULL && strcmp(channel_info->name, "DummyChannel1") == 0);
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_get_info(channel, &channel_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(channel_info->name != NULL && strcmp(channel_info->name, "DummyChannel2") == 0);
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(channel, (ed247_channel_t)NULL);
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_get_info(channel, &channel_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(channel_info->name != NULL && strcmp(channel_info->name, "DummyChannel1") == 0);
    
    // Select a single channel
    ASSERT_EQ(ed247_find_channels (context, "FilledChannel", &channel_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_get_info(channel, &channel_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(channel_info->name != NULL && strcmp(channel_info->name, "FilledChannel") == 0);
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(channel, (ed247_channel_t)NULL);
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_get_info(channel, &channel_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(channel_info->name != NULL && strcmp(channel_info->name, "FilledChannel") == 0);
    
    // Make a too selective request
    ASSERT_EQ(ed247_find_channels (context, "ChannelThatDoesNotExist", &channel_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(channel, (ed247_channel_t)NULL);
    
    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
    
}

TEST(UtApiChannel,GetChannelList)
{
    ed247_context_t context;
    ed247_channel_list_t channel_list;
    ed247_channel_t channel;
    const ed247_channel_info_t* channel_info;

    std::string filepath = config_path+"/ecic_unit_api_channels.xml";
    ASSERT_EQ(ed247_load(filepath.c_str(), NULL, &context), ED247_STATUS_SUCCESS);

    // First tests validate the parsing of the ecic file
    ASSERT_EQ(ed247_component_get_channels(NULL, &channel_list), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_component_get_channels(context, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_component_get_channels(context, &channel_list), ED247_STATUS_SUCCESS);

    // Retrieve the first channel, check invalid calls
    ASSERT_EQ(ed247_channel_list_next(NULL, &channel), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_list_next(channel_list, NULL), ED247_STATUS_FAILURE);

    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    
    // Retrieve The channel information of the first channel, check invalid calls
    ASSERT_EQ(ed247_channel_get_info(NULL, &channel_info), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_get_info(channel, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_channel_get_info(channel, &channel_info), ED247_STATUS_SUCCESS);
    
    
    ASSERT_EQ(ed247_component_get_channels(context, &channel_list), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_get_info(channel, &channel_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(channel_info->name != NULL && strcmp(channel_info->name, "DefaultValues") == 0);
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_get_info(channel, &channel_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(channel_info->name != NULL && strcmp(channel_info->name, "DummyChannel1") == 0);
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_get_info(channel, &channel_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(channel_info->name != NULL && strcmp(channel_info->name, "DummyChannel2") == 0);
    ASSERT_EQ(ed247_channel_list_next(channel_list, &channel), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_get_info(channel, &channel_info), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(channel_info->name != NULL && strcmp(channel_info->name, "FilledChannel") == 0);
}

int main(int argc, char **argv)
{
    if(argc >=1)
        config_path = argv[1];
    else
        config_path = "../config";

    std::cout << "Configuration path: " << config_path << std::endl;

    ::testing::InitGoogleTest(&argc, argv);
    // ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}