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

#include <stdio.h>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

/***********
 * Defines *
 ***********/

/********
 * Test *
 ********/

/******************************************************************************
Test robustness on load and unload sequences. The cases are the following:
- Context address not provided for load/unload
- ECIC file path not provided
- ECIC file path is erroneous
- ECIC file path is miss formatted
- Unload after an error in loading procedure
******************************************************************************/
class LoadRobustness : public ::testing::Test{};

TEST(RobustnessLoad, Loading)
{
    ed247_context_t context;
    // Context address not provided, unload after error checked
    ASSERT_EQ(ed247_load(CONFIG_PATH"/ft_load_all/a429.xml", NULL, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_unload(NULL), ED247_STATUS_FAILURE);
    
    // ECIC file path not provided, unload after error checked
    ASSERT_EQ(ed247_load(NULL, NULL, &context), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_unload(context), ED247_STATUS_FAILURE);
    
    // ECIC file path is erroneous, unload after error checked
    ASSERT_EQ(ed247_load("NotExistingFile.xml", NULL, &context), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_unload(context), ED247_STATUS_FAILURE);
    
    // ECIC file path is miss formatted
    ASSERT_EQ(ed247_load(CONFIG_PATH"/ft_load_all/missformatted.xml", NULL, &context), ED247_STATUS_FAILURE);
    
    // Check the errors did not mess up the loading capabilities
    ASSERT_EQ(ed247_load(CONFIG_PATH"/ft_load_all/a429.xml", NULL, &context), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
    
    // ECIC file with duplicated stream
    ASSERT_EQ(ed247_load(CONFIG_PATH"/ft_load_all/stream_duplicated.xml", NULL, &context), ED247_STATUS_FAILURE);

    // ECIC file with duplicated channel
    ASSERT_EQ(ed247_load(CONFIG_PATH"/ft_load_all/stream_duplicated.xml", NULL, &context), ED247_STATUS_FAILURE);
}


/******************************************************************************
Test functional load/unload procedure and check the memhooks are working fine
******************************************************************************/
class LoadContext : public ::testing::TestWithParam<std::string>{};

TEST_P(LoadContext, Loading)
{
    ed247_context_t ed247_context;

    std::string filepath = GetParam();

    ASSERT_EQ(ed247_load(filepath.c_str(),NULL,&ed247_context), ED247_STATUS_SUCCESS);

    ASSERT_EQ(ed247_unload(ed247_context), ED247_STATUS_SUCCESS);

    // Test mem hooks

    memhooks_section_start();

    void *test = malloc(100000);
    free(test);

#ifdef __linux__
    ASSERT_FALSE(memhooks_section_stop());
#else
    ASSERT_TRUE(memhooks_section_stop());
#endif
}

std::vector<std::string> configuration_files = {
    std::string(CONFIG_PATH"/ft_load_all/a429.xml")
    };

INSTANTIATE_TEST_CASE_P(LoadingTests, LoadContext,
    ::testing::ValuesIn(configuration_files));

/*************
 * Functions *
 *************/

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}