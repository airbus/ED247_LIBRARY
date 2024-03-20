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

std::string config_path = "../config";

/******************************************************************************
Test robustness on load and unload sequences. The cases are the following:
- Context address not provided for load/unload
- ECIC file path not provided
- ECIC file path is erroneous
- ECIC file path is miss formatted
- Unload after an error in loading procedure
******************************************************************************/
class LoadRobustness : public ::testing::Test{};

const char* ecic_content = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<ED247ComponentInstanceConfiguration ComponentType=\"Virtual\" Name=\"VirtualComponent\" Comment=\"\" StandardRevision=\"A\" Identifier=\"0\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"ED247A_ECIC.xsd\">\n"
"	<Channels>\n"
"		<MultiChannel Name=\"Channel0\" Comment=\"\">\n"
"           <FrameFormat StandardRevision=\"A\"/>\n"
"            <ComInterface>\n"
"                <UDP_Sockets>\n"
"			        <UDP_Socket DstIP=\"127.0.0.1\" DstPort=\"2589\"/>\n"
"                </UDP_Sockets>\n"
"            </ComInterface>\n"
"			<Streams>\n"
"				<A429_Stream UID=\"0\" Name=\"A429StreamOutput\" Direction=\"Out\" Comment=\"\" ICD=\"File.xml:BUS45\">\n"
"                    <DataTimestamp Enable=\"No\" SampleDataTimestampOffset=\"No\"/>\n"
"                    <Errors Enable=\"No\"/>\n"
"				</A429_Stream>\n"
"				<A429_Stream UID=\"1\" Name=\"A429StreamInput\" Direction=\"Out\" Comment=\"\" ICD=\"File.xml:BUS45\">\n"
"                    <DataTimestamp Enable=\"No\" SampleDataTimestampOffset=\"No\"/>\n"
"                    <Errors Enable=\"No\"/>\n"
"                </A429_Stream>\n"
"			</Streams>\n"
"		</MultiChannel>\n"
"	</Channels>\n"
"</ED247ComponentInstanceConfiguration>";

TEST(RobustnessLoad, Loading)
{
    ed247_context_t context;
    // Context address not provided, unload after error checked
    ASSERT_EQ(ed247_load_file((config_path+"/ecic_func_load_all_a429.xml").c_str(), NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_unload(NULL), ED247_STATUS_FAILURE);

    // ECIC file path not provided, unload after error checked
    ASSERT_EQ(ed247_load_file(NULL, &context), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_unload(context), ED247_STATUS_FAILURE);

    // ECIC file path is erroneous, unload after error checked
    ASSERT_EQ(ed247_load_file("NotExistingFile.xml", &context), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_unload(context), ED247_STATUS_FAILURE);

    // ECIC file path is miss formatted
    ASSERT_EQ(ed247_load_file((config_path+"/ecic_func_load_all_missformatted.xml").c_str(), &context), ED247_STATUS_FAILURE);

    // Check the errors did not mess up the loading capabilities
    ASSERT_EQ(ed247_load_file((config_path+"/ecic_func_load_all_a429.xml").c_str(), &context), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);

    // ECIC file with duplicated stream
    ASSERT_EQ(ed247_load_file((config_path+"/ecic_func_load_all_stream_duplicated.xml").c_str(), &context), ED247_STATUS_FAILURE);

    // ECIC file with duplicated channel
    ASSERT_EQ(ed247_load_file((config_path+"/ecic_func_load_all_channel_duplicated.xml").c_str(), &context), ED247_STATUS_FAILURE);

    // Check load by providing ECIC content
    ASSERT_EQ(ed247_load_content(ecic_content, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_load_content(NULL, &context), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_load_content(ecic_content, &context), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);

    // Check load by content fails if wrong ECIC content is provided
    ASSERT_EQ(ed247_load_content("I am a wrong configuration file content !", &context), ED247_STATUS_FAILURE);

    // ECIC ETH file FrameSize disabled but sample max number is > 1, unload after error checked
    ASSERT_EQ(ed247_load_file((config_path+"/ecic_func_load_all_eth.xml").c_str(), &context), ED247_STATUS_FAILURE);

    //Sample Max Size Bytes UDP check
    ASSERT_EQ(ed247_load_file((config_path+"/ecic_func_load_all_sample_size.xml").c_str(), &context), ED247_STATUS_FAILURE);

}

TEST(NoInputs, Wait)
{
    ed247_context_t context;
    // Context address not provided, unload after error checked
    ASSERT_EQ(ed247_load_file((config_path+"/ecic_func_load_all_no_inputs.xml").c_str(), &context), ED247_STATUS_SUCCESS);

    ed247_stream_list_t streams;
    ASSERT_EQ(ed247_wait_frame(context, &streams, 1000), ED247_STATUS_TIMEOUT);
    ASSERT_EQ(ed247_wait_during(context, &streams, 1000), ED247_STATUS_NODATA);

    ASSERT_EQ(ed247_unload(NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
}


/******************************************************************************
Test functional load/unload procedure and check the malloc count is working fine
******************************************************************************/
class LoadContext : public ::testing::TestWithParam<std::string>{};

TEST_P(LoadContext, Loading)
{
    ed247_context_t ed247_context;

    std::string filepath = GetParam();

    ASSERT_EQ(ed247_load_file(filepath.c_str(),&ed247_context), ED247_STATUS_SUCCESS);

    ASSERT_EQ(ed247_unload(ed247_context), ED247_STATUS_SUCCESS);

    // Test mem hooks
    malloc_count_start();
    void *test = malloc(100000);
    free(test);
#ifdef __linux__
    ASSERT_EQ(malloc_count_stop(), 1);
#else
    ASSERT_EQ(malloc_count_stop(), 0);
#endif
}

std::vector<std::string> configuration_files;

INSTANTIATE_TEST_CASE_P(func_load_all, LoadContext,
    ::testing::ValuesIn(configuration_files));

/*************
 * Functions *
 *************/

int main(int argc, char **argv)
{
    // Parsing argument manage by Google Test framework
    ::testing::InitGoogleTest(&argc, argv);

    if(argc > 1)
        config_path = argv[1];
    else
        config_path = "../config";

    tests_tools::display_ed247_lib_infos();
    SAY("Configuration path: " << config_path);

    configuration_files.push_back(config_path+"/ecic_func_load_all_a429.xml");

    ::testing::InitGoogleTest(&argc, argv);
  // ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
