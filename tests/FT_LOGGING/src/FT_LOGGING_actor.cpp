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

#include <stdio.h>
#include <fstream>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include "gtest/gtest.h"
#include "gmock/gmock.h"

/***********
 * Defines *
 ***********/

/********
 * Test *
 ********/

/******************************************************************************
Test the log routines and customization of output.
Execute several time the same sequence (load + unload of a simple ECIC).
Each time the configuration tells to print more output.
Check on the out file that output is growing.
******************************************************************************/
class LogConfigurationTest : public ::testing::Test{};

TEST(LogConfigurationTest, Loading)
{
    ed247_context_t context;
    const char* filename = CONFIG_PATH"/ft_logging/ecic_no_optional_desc.xml";

#ifdef _MSC_VER
    char * logfile;
    size_t len;
    _dupenv_s(&logfile, &len, "ED247_LOG_FILEPATH");
    ASSERT_NE(logfile, (char*) NULL);
#else
    // Do not even think about removing "std::" before getenv,
    // if unspecified the appropriate getenv will not be used !!!!
    const char* logfile = std::getenv("ED247_LOG_FILEPATH");
    ASSERT_NE(logfile, (const char*) NULL);
#endif    
    // Because printing the stacktraces generates more dynamic allocations they have to be deactivated
#ifdef __linux__
    setenv("MEMHOOKS_LEVEL", "1", 1);
#else
    _putenv_s("MEMHOOKS_LEVEL","1");
#endif
    
    // Ensure the temporary log file does not exist or is empty when starting the sequence.
    const uint32_t* retrieve_ptr = test::count_matching_lines_in_file(logfile, ".*");
    if (retrieve_ptr != NULL)
    {
#ifdef _MSC_VER
        _unlink(logfile);
#else
        unlink(logfile);
#endif
        retrieve_ptr = test::count_matching_lines_in_file(logfile, ".*");
    }
    ASSERT_EQ(retrieve_ptr, (const uint32_t*)NULL);
    
    
    // Run the sequence of load / Unload with increasing traces
    // ed247_set_log_level(ED247_LOG_LEVEL_ERROR);
    // ed247_load(filename, NULL, &context);
    // ed247_unload(context);
    // retrieve_ptr = test::count_matching_lines_in_file(logfile, ".*");
    // ASSERT_NE(retrieve_ptr, (const uint32_t*)NULL);
    // const uint32_t size_log_none = *retrieve_ptr;
    
    // ed247_set_log_level(ED247_LOG_LEVEL_ERROR);
    // ed247_load(filename, NULL, &context);
    // ed247_unload(context);
    // retrieve_ptr = test::count_matching_lines_in_file(logfile, ".*");
    // ASSERT_NE(retrieve_ptr, (const uint32_t*)NULL);
    // const uint32_t size_log_error = *retrieve_ptr;
    
    ed247_set_log_level(ED247_LOG_LEVEL_WARNING);
    ed247_load(filename, NULL, &context);
    ed247_unload(context);
    retrieve_ptr = test::count_matching_lines_in_file(logfile, ".*");
    ASSERT_NE(retrieve_ptr, (const uint32_t*)NULL);
    const uint32_t size_log_warning = *retrieve_ptr;
    
    ed247_set_log_level(ED247_LOG_LEVEL_INFO);
    ed247_load(filename, NULL, &context);
    ed247_unload(context);
    retrieve_ptr = test::count_matching_lines_in_file(logfile, ".*");
    ASSERT_NE(retrieve_ptr, (const uint32_t*)NULL);
    const uint32_t size_log_info = *retrieve_ptr;
    
    ed247_set_log_level(ED247_LOG_LEVEL_DEBUG);
    ed247_load(filename, NULL, &context);
    ed247_unload(context);
    retrieve_ptr = test::count_matching_lines_in_file(logfile, ".*");
    ASSERT_NE(retrieve_ptr, (const uint32_t*)NULL);
    const uint32_t size_log_debug = *retrieve_ptr;
    
    ed247_set_log_level(ED247_LOG_LEVEL_TEST);
    ed247_load(filename, NULL, &context);
    ed247_unload(context);
    retrieve_ptr = test::count_matching_lines_in_file(logfile, ".*");
    ASSERT_NE(retrieve_ptr, (const uint32_t*)NULL);
    const uint32_t size_log_test = *retrieve_ptr;
    
    // Check such log sizes are consistents
    // ASSERT_EQ(size_log_none, (uint32_t)0);
    // ASSERT_GT(size_log_error, size_log_none);
    // ASSERT_GT(size_log_warning, size_log_error);
    ASSERT_GT(size_log_warning, (uint32_t)0);
    ASSERT_GT(size_log_info, size_log_warning);
    ASSERT_GT(size_log_debug, size_log_info);
    // The last one is only greater on equal because there
    // shall not be specific test traces in the library
    ASSERT_GE(size_log_test, size_log_debug);
    
    // To end this test, verify the robustness of the logger set_level
    ed247_log_level_t level = ED247_LOG_LEVEL_ERROR;
    ed247_set_log_level((ed247_log_level_t)((int)ED247_LOG_LEVEL_TEST+1));
    ed247_get_log_level(&level);
    ASSERT_EQ(level, ED247_LOG_LEVEL_TEST);
    ed247_set_log_level((ed247_log_level_t)((int)ED247_LOG_LEVEL_ERROR-1));
    ed247_get_log_level(&level);
    ASSERT_EQ(level, ED247_LOG_LEVEL_ERROR);
    
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}