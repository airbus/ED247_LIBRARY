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

std::string config_path = "../config";

class FrameContext : public ::testing::TestWithParam<std::string>{};

TEST_P(FrameContext, Main)
{
    ed247_context_t context;
    ed247_channel_list_t channels_output;
    ed247_channel_list_t channels_input;
    ed247_channel_t channel_output;
    const ed247_channel_info_t *channel_info;
    ed247_channel_t channel_input;
    ed247_stream_list_t streams;
    ed247_stream_t stream_output;
    const ed247_stream_info_t *stream_output_info;
    ed247_stream_t stream_input;
    const ed247_stream_info_t *stream_input_info;
    void *sample_data;
    const void *csample_data;
    size_t sample_size;
    bool full, empty;
    ed247_frame_list_t frames;
    const ed247_frame_t *frame;

    std::string filepath = GetParam();

    ASSERT_EQ(ed247_load_file(filepath.c_str(),&context), ED247_STATUS_SUCCESS);

    // Retrieve channel identifier
    ASSERT_EQ(ed247_find_channels(context, "Channel0", &channels_output), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_list_next(channels_output, &channel_output), ED247_STATUS_SUCCESS);
    ed247_channel_get_info(channel_output, &channel_info);
    SAY("Name: " << channel_info->name);
    ASSERT_EQ(ed247_channel_list_free(channels_output), ED247_STATUS_SUCCESS);

    ASSERT_EQ(ed247_find_channels(context, "Channel1", &channels_input), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_channel_list_next(channels_input, &channel_input), ED247_STATUS_SUCCESS);
    ed247_channel_get_info(channel_output, &channel_info);
    SAY("Name: " << channel_info->name);
    ASSERT_EQ(ed247_channel_list_free(channels_input), ED247_STATUS_SUCCESS);

    // Retrieve streams
    ASSERT_EQ(ed247_channel_find_streams(channel_output, "Stream.*", &streams), ED247_STATUS_SUCCESS);
    while(ed247_stream_list_next(streams, &stream_output) == ED247_STATUS_SUCCESS && stream_output){
        ASSERT_EQ(ed247_stream_get_info(stream_output, &stream_output_info), ED247_STATUS_SUCCESS);
        // Allocate sample & push them
        ASSERT_EQ(ed247_stream_allocate_sample(stream_output, &sample_data, &sample_size), ED247_STATUS_SUCCESS);
        for(uint16_t i = 0 ; i < stream_output_info->sample_max_number ; i++){
            std::string sample_str = strize() << std::setw(sample_size) << std::setfill('0') << i;
            memcpy(sample_data, sample_str.c_str(), sample_size);
            malloc_count_start();
            ASSERT_EQ(ed247_stream_push_sample(stream_output, sample_data, sample_size, nullptr, &full), ED247_STATUS_SUCCESS);
            ASSERT_EQ(malloc_count_stop(), 0);
            if((uint16_t)i < (stream_output_info->sample_max_number-1))
                ASSERT_FALSE(full);
            else
                ASSERT_TRUE(full);
        }
    }

    // Encode frame
    malloc_count_start();
    ASSERT_EQ(ed247_frame_encode(context, &frames), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_frame_list_next(frames, &frame), ED247_STATUS_SUCCESS);

    ASSERT_EQ(frame->channel, channel_output);
    ASSERT_EQ(malloc_count_stop(), 0);
    // Check limit cases
    ASSERT_EQ(ed247_frame_list_next(NULL, &frame), ED247_STATUS_FAILURE);
    size_t size;
    ASSERT_EQ(ed247_frame_list_next(frames, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_frame_list_size(NULL, &size), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_frame_list_size(frames, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_frame_list_size(frames, &size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(size, (size_t)2);
    ASSERT_EQ(ed247_frame_encode(NULL, &frames), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_frame_encode(context, NULL), ED247_STATUS_FAILURE);

    size_t frame_index = 0;
    while(ed247_stream_list_next(streams, &stream_output) == ED247_STATUS_SUCCESS && stream_output){
        ASSERT_EQ(ed247_stream_get_info(stream_output, &stream_output_info), ED247_STATUS_SUCCESS);
        ed247_uid_t sid = ntohs(*(ed247_uid_t*)((char*)frame->data+frame_index));
        ASSERT_EQ(sid, stream_output_info->uid);
        frame_index += sizeof(ed247_uid_t);
        size_t ssize = ntohs(*(uint16_t*)((char*)frame->data+frame_index));
        if(stream_output_info->type == ED247_STREAM_TYPE_A664){
            if(stream_output_info->sample_max_number > 1){ // Warning: This assumes the MessageSize is disabled only when SampleMaxNumber FOR THIS EXAMPLE ONLY
                ASSERT_EQ(ssize, (sizeof(uint16_t)+stream_output_info->sample_max_size_bytes)*stream_output_info->sample_max_number);
            }else{
                ASSERT_EQ(ssize, stream_output_info->sample_max_size_bytes*stream_output_info->sample_max_number);
            }
        }else if(stream_output_info->type == ED247_STREAM_TYPE_A825){
            ASSERT_EQ(ssize, (sizeof(uint8_t)+stream_output_info->sample_max_size_bytes)*stream_output_info->sample_max_number);
        }else if(stream_output_info->type == ED247_STREAM_TYPE_SERIAL){
            ASSERT_EQ(ssize, (sizeof(uint16_t)+stream_output_info->sample_max_size_bytes)*stream_output_info->sample_max_number);
        }else{
            ASSERT_EQ(ssize, stream_output_info->sample_max_size_bytes*stream_output_info->sample_max_number);
        }
        frame_index += sizeof(uint16_t);
        for(uint16_t i = 0 ; i < stream_output_info->sample_max_number ; i++){
          std::string expected = strize() << std::setw(sample_size) << std::setfill('0') << i;
            if(stream_output_info->type == ED247_STREAM_TYPE_A664 && stream_output_info->sample_max_number > 1){
                    auto sample_size_tmp = (uint16_t)ntohs(*(uint16_t*)((char*)frame->data+frame_index));
                    frame_index += sizeof(uint16_t);
                    ASSERT_EQ(sample_size_tmp, sample_size);
            }else if(stream_output_info->type == ED247_STREAM_TYPE_A825){
                    auto sample_size_tmp = *(uint8_t*)((char*)frame->data+frame_index);
                    frame_index += sizeof(uint8_t);
                    ASSERT_EQ(sample_size_tmp, sample_size);
            }else if(stream_output_info->type == ED247_STREAM_TYPE_SERIAL){
                    auto sample_size_tmp = *(uint16_t*)((char*)frame->data+frame_index);
                    frame_index += sizeof(uint16_t);
                    ASSERT_EQ(sample_size_tmp, sample_size);
            }
            std::string strsample((char*)frame->data+frame_index, stream_output_info->sample_max_size_bytes);
            ASSERT_EQ(strsample, expected);
            frame_index += stream_output_info->sample_max_size_bytes;
        }
    }
    ASSERT_EQ(ed247_frame_list_free(frames), ED247_STATUS_SUCCESS);
    // Check limit case
    ASSERT_EQ(ed247_frame_list_free(NULL), ED247_STATUS_FAILURE);

    // Decode frame
    malloc_count_start();
    ASSERT_EQ(ed247_frame_decode(channel_input, frame->data, frame->size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_frame_decode(NULL, frame->data, frame->size), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_frame_decode(channel_input, NULL, frame->size), ED247_STATUS_FAILURE);
    ASSERT_EQ(malloc_count_stop(), 0);

    // Pop stream samples
    ASSERT_EQ(ed247_channel_find_streams(channel_input, "Stream.*", &streams), ED247_STATUS_SUCCESS);
    while(ed247_stream_list_next(streams, &stream_input) == ED247_STATUS_SUCCESS && stream_input){
        ASSERT_EQ(ed247_stream_get_info(stream_input, &stream_input_info), ED247_STATUS_SUCCESS);
        int i = 0;
        do{
            std::string sample_str = strize() << std::setw(sample_size) << std::setfill('0') << i++;
            malloc_count_start();
            ASSERT_EQ(ed247_stream_pop_sample(stream_input, &csample_data, &sample_size, nullptr, nullptr, nullptr, &empty), ED247_STATUS_SUCCESS);
            ASSERT_EQ(malloc_count_stop(), 0);
            ASSERT_EQ(memcmp(csample_data, sample_str.c_str(), sample_size), 0);
        }while(!empty);
    }

    // Unload
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
}

std::vector<std::string> configuration_files;

INSTANTIATE_TEST_CASE_P(FrameTests, FrameContext,
    ::testing::ValuesIn(configuration_files));

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

    configuration_files.push_back(config_path+"/ecic_func_frame_a429.xml");
    configuration_files.push_back(config_path+"/ecic_func_frame_a664.xml");
    configuration_files.push_back(config_path+"/ecic_func_frame_a825.xml");
    configuration_files.push_back(config_path+"/ecic_func_frame_serial.xml");

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
