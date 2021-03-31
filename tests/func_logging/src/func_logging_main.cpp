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

/***********
 * Defines *
 ***********/

/********
 * Test *
 ********/

std::string config_path = "../config";

/******************************************************************************
Test the log routines and customization of output.
Execute several time the same sequence (load + unload of a simple ECIC).
Each time the configuration tells to print more output.
Check on the out file that output is growing.
******************************************************************************/
class LogConfigurationTest : public ::testing::Test{};

TEST(LogConfigurationTest, Logging)
{
    ed247_context_t context;
    std::string filename = config_path+"/ecic_func_logging.xml";  
    // Because printing the stacktraces generates more dynamic allocations they have to be deactivated
#ifdef __linux__
    setenv("MEMHOOKS_LEVEL", "1", 1);
    setenv("ED247_LOG_FILEPATH", "./ed247.logs", 1);
#else
    _putenv_s("MEMHOOKS_LEVEL","1");
    _putenv_s("ED247_LOG_FILEPATH","./ed247.logs");
#endif
    const char* logfile = std::getenv("ED247_LOG_FILEPATH");
    ASSERT_NE(logfile, (const char*) NULL);
    
    // Ensure the temporary log file does not exist or is empty when starting the sequence.
    const uint32_t* retrieve_ptr = synchro::count_matching_lines_in_file(logfile, ".*");
    if (retrieve_ptr != NULL)
    {
#ifdef _MSC_VER
        _unlink(logfile);
#else
        unlink(logfile);
#endif
        retrieve_ptr = synchro::count_matching_lines_in_file(logfile, ".*");
    }
    ASSERT_EQ(retrieve_ptr, (const uint32_t*)NULL);
    
    ed247_set_log_level(ED247_LOG_LEVEL_WARNING);
    ed247_load(filename.c_str(), NULL, &context);
    ed247_unload(context);
    retrieve_ptr = synchro::count_matching_lines_in_file(logfile, ".*");
    ASSERT_NE(retrieve_ptr, (const uint32_t*)NULL);
    const uint32_t size_log_warning = *retrieve_ptr;
    
    ed247_set_log_level(ED247_LOG_LEVEL_INFO);
    ed247_load(filename.c_str(), NULL, &context);
    ed247_unload(context);
    retrieve_ptr = synchro::count_matching_lines_in_file(logfile, ".*");
    ASSERT_NE(retrieve_ptr, (const uint32_t*)NULL);
    const uint32_t size_log_info = *retrieve_ptr;
    
    ed247_set_log_level(ED247_LOG_LEVEL_DEBUG);
    ed247_load(filename.c_str(), NULL, &context);
    ed247_unload(context);
    retrieve_ptr = synchro::count_matching_lines_in_file(logfile, ".*");
    ASSERT_NE(retrieve_ptr, (const uint32_t*)NULL);
    const uint32_t size_log_debug = *retrieve_ptr;
    
    // Check such log sizes are consistents
    ASSERT_GT(size_log_warning, (uint32_t)0);
    ASSERT_GT(size_log_info, size_log_warning);
    ASSERT_GT(size_log_debug, size_log_info);
    
    // To end this test, verify the robustness of the logger set_level
    ed247_log_level_t level = ED247_LOG_LEVEL_ERROR;
    ed247_set_log_level((ed247_log_level_t)((int)ED247_LOG_LEVEL_DEBUG+1));
    ed247_get_log_level(&level);
    ASSERT_EQ(level, ED247_LOG_LEVEL_DEBUG);
    ed247_set_log_level((ed247_log_level_t)((int)ED247_LOG_LEVEL_ERROR-1));
    ed247_get_log_level(&level);
    ASSERT_EQ(level, ED247_LOG_LEVEL_ERROR);
    
}

TEST(LogConfigurationTest, NoLogging)
{
    ed247_context_t context;
    std::string filename = config_path+"/ecic_func_logging.xml";

    // Set an unvalid path (empty) so that the file cannot be written
#ifdef __linux__
    setenv("ED247_LOG_FILEPATH", "", 1);
#else
    _putenv_s("ED247_LOG_FILEPATH", "");
#endif
    const char* logfile = std::getenv("ED247_LOG_FILEPATH");

    std::cout << "Check log file ..." << std::endl;

    // delete logging file if necessary
    const uint32_t* retrieve_ptr = synchro::count_matching_lines_in_file(logfile, ".*");
    if (retrieve_ptr != NULL)
    {
#ifdef _MSC_VER
        _unlink(logfile);
#else
        unlink(logfile);
#endif
        retrieve_ptr = synchro::count_matching_lines_in_file(logfile, ".*");
    }
    ASSERT_EQ(retrieve_ptr, (const uint32_t*)NULL);

    std::cout << "Loading ..." << std::endl;
    
    // Run the sequence of load
    ASSERT_EQ(ed247_load(filename.c_str(), NULL, &context), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);

    std::cout << "Unloading ..." << std::endl;

    // Check it has not been created
    retrieve_ptr = synchro::count_matching_lines_in_file(logfile, ".*");
    ASSERT_EQ(retrieve_ptr, (const uint32_t*)NULL);
}

TEST(LogConfigurationTest, LoggingByArgs)
{
    ed247_context_t context;
    std::string filename = config_path+"/ecic_func_logging.xml";

    // Set an unvalid path (empty) so that the file cannot be written

    std::cout << "Check log file ..." << std::endl;

    // delete logging file if necessary
    const char* logfile = "./ed247_by_config.logs";
    const uint32_t* retrieve_ptr = synchro::count_matching_lines_in_file(logfile, ".*");
    if (retrieve_ptr != NULL)
    {
#ifdef _MSC_VER
        _unlink(logfile);
#else
        unlink(logfile);
#endif
        retrieve_ptr = synchro::count_matching_lines_in_file(logfile, ".*");
    }
    ASSERT_EQ(retrieve_ptr, (const uint32_t*)NULL);

    std::cout << "Loading ..." << std::endl;
    
    // Run the sequence of load
    libed247_configuration_t configuration = LIBED247_CONFIGURATION_DEFAULT;
    configuration.log_filepath = logfile;
    configuration.log_level = ED247_LOG_LEVEL_DEBUG;
    ASSERT_EQ(ed247_load(filename.c_str(), &configuration, &context), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);

    std::cout << "Unloading ..." << std::endl;

    // Check it has not been created
    retrieve_ptr = synchro::count_matching_lines_in_file(logfile, ".*");
    ASSERT_GT(*retrieve_ptr, 0);
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