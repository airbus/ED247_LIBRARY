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

#ifdef __unix__
# define env_set(var, value) setenv(var, value, 1)
# define env_rm(var) unsetenv(var)
static struct stat unused_stat_buffer;
# define file_exist(file) (stat(file, &unused_stat_buffer) == 0)
#else
# define env_set(var, value) _putenv_s(var, value)
# define env_rm(var) _putenv_s(var, "")
static struct _stat unused_stat_buffer;
# define file_exist(file) (_stat(file, &unused_stat_buffer) == 0)
# define unlink _unlink
#endif


namespace ed247 {

/******************************************************************************
Test the log routines and customization of output.
Execute several time the same sequence (load + unload of a simple ECIC).
Each time the configuration tells to print more output.
Check on the out file that output is growing.
******************************************************************************/
  class LogConfigurationTest : public ::testing::Test{};

  TEST(LogConfigurationTest, LoggingByEnv)
  {
    constexpr const char* log_filepath = "./ed247_test_log_by_env.log";
    const uint32_t* match_count = nullptr;
    ed247_log_level_t level = ED247_LOG_LEVEL_UNSET;
    std::string level_str;

    ed247::log::delete_logger();
    unlink(log_filepath);
    EXPECT_FALSE(file_exist(log_filepath));

    env_set("ED247_LOG_FILEPATH", log_filepath);

    level_str = strize() << (int)ED247_LOG_LEVEL_ERROR;
    env_set("ED247_LOG_LEVEL", level_str.c_str());
    ed247_set_log_level(ED247_LOG_LEVEL_CRAZY);
    EXPECT_TRUE(file_exist(log_filepath));
    match_count = tests_tools::count_matching_lines_in_file(log_filepath, ".*ERROR.*");
    EXPECT_NE(match_count, nullptr);
    if (match_count != nullptr) { EXPECT_EQ(*match_count, 0); } // ERROR level do not display any traces
    match_count = tests_tools::count_matching_lines_in_file(log_filepath, ".*CRAZY.*");
    EXPECT_NE(match_count, nullptr);
    if (match_count != nullptr) { EXPECT_EQ(*match_count, 0); }

    ed247::log::delete_logger();
    unlink(log_filepath);

    level_str = strize() << (int)ED247_LOG_LEVEL_WARNING;
    env_set("ED247_LOG_LEVEL", level_str.c_str());
    ed247_set_log_level(ED247_LOG_LEVEL_CRAZY);
    EXPECT_TRUE(file_exist(log_filepath));
    match_count = tests_tools::count_matching_lines_in_file(log_filepath, ".*WARNING.*");
    EXPECT_NE(match_count, nullptr);
    if (match_count != nullptr) { EXPECT_GT(*match_count, 0); }
    match_count = tests_tools::count_matching_lines_in_file(log_filepath, ".*CRAZY.*");
    EXPECT_NE(match_count, nullptr);
    if (match_count != nullptr) { EXPECT_EQ(*match_count, 0); }

    ed247::log::delete_logger();
    unlink(log_filepath);

    level_str = strize() << (int)ED247_LOG_LEVEL_INFO;
    env_set("ED247_LOG_LEVEL", level_str.c_str());
    ed247_set_log_level(ED247_LOG_LEVEL_CRAZY);
    EXPECT_TRUE(file_exist(log_filepath));
    match_count = tests_tools::count_matching_lines_in_file(log_filepath, ".*INFO.*");
    EXPECT_NE(match_count, nullptr);
    if (match_count != nullptr) { EXPECT_GT(*match_count, 0); }
    match_count = tests_tools::count_matching_lines_in_file(log_filepath, ".*CRAZY.*");
    EXPECT_NE(match_count, nullptr);
    if (match_count != nullptr) { EXPECT_EQ(*match_count, 0); }

    ed247::log::delete_logger();
    unlink(log_filepath);

    level_str = strize() << (int)ED247_LOG_LEVEL_DEBUG;
    env_set("ED247_LOG_LEVEL", level_str.c_str());
    ed247_set_log_level(ED247_LOG_LEVEL_CRAZY);
    EXPECT_TRUE(file_exist(log_filepath));
    match_count = tests_tools::count_matching_lines_in_file(log_filepath, ".*DEBUG.*");
    EXPECT_NE(match_count, nullptr);
    if (match_count != nullptr) { EXPECT_GT(*match_count, 0); }
    match_count = tests_tools::count_matching_lines_in_file(log_filepath, ".*CRAZY.*");
    EXPECT_NE(match_count, nullptr);
    if (match_count != nullptr) { EXPECT_EQ(*match_count, 0); }

    ed247::log::delete_logger();
    unlink(log_filepath);

    level_str = strize() << (int)ED247_LOG_LEVEL_CRAZY;
    env_set("ED247_LOG_LEVEL", level_str.c_str());
    ed247_set_log_level(ED247_LOG_LEVEL_INFO);
    EXPECT_TRUE(file_exist(log_filepath));
    match_count = tests_tools::count_matching_lines_in_file(log_filepath, ".*CRAZY.*");
    EXPECT_NE(match_count, nullptr);
    if (match_count != nullptr) { EXPECT_GT(*match_count, 0); }
    match_count = tests_tools::count_matching_lines_in_file(log_filepath, ".*INFO.*");
    EXPECT_NE(match_count, nullptr);
    if (match_count != nullptr) { EXPECT_EQ(*match_count, 0); }

    ed247::log::delete_logger();
    unlink(log_filepath);

    level_str = strize() << (int)(ED247_LOG_LEVEL_MAX) + 1;
    env_set("ED247_LOG_LEVEL", level_str.c_str());
    ed247_set_log_level(ED247_LOG_LEVEL_INFO);
    ed247_get_log_level(&level);
    EXPECT_EQ((int)level, (int)ED247_LOG_LEVEL_MAX);

    ed247::log::delete_logger();
    unlink(log_filepath);

    level_str = strize() << (int)(ED247_LOG_LEVEL_MIN) - 1;
    env_set("ED247_LOG_LEVEL", level_str.c_str());
    ed247_set_log_level(ED247_LOG_LEVEL_INFO);
    ed247_get_log_level(&level);
    EXPECT_EQ((int)level, (int)ED247_LOG_LEVEL_MIN);

    ed247::log::delete_logger();
    unlink(log_filepath);
  }

  TEST(LogConfigurationTest, LoggingByArgs)
  {
    constexpr const char* log_filepath = "./ed247_test_log_by_args.log";
    const uint32_t* match_count = nullptr;
    ed247_log_level_t level = ED247_LOG_LEVEL_UNSET;

    ed247::log::delete_logger();
    unlink(log_filepath);
    EXPECT_FALSE(file_exist(log_filepath));

    env_rm("ED247_LOG_FILEPATH");
    env_rm("ED247_LOG_LEVEL");

    ed247_set_log(ED247_LOG_LEVEL_ERROR, log_filepath);
    EXPECT_TRUE(file_exist(log_filepath));
    match_count = tests_tools::count_matching_lines_in_file(log_filepath, ".*");
    EXPECT_NE(match_count, nullptr);
    if (match_count != nullptr) { EXPECT_EQ(*match_count, 0); } // ERROR level do not display any traces
    match_count = tests_tools::count_matching_lines_in_file(log_filepath, ".*WARNING.*");
    EXPECT_NE(match_count, nullptr);
    if (match_count != nullptr) { EXPECT_EQ(*match_count, 0); }
    match_count = tests_tools::count_matching_lines_in_file(log_filepath, ".*INFO.*");
    EXPECT_NE(match_count, nullptr);
    if (match_count != nullptr) { EXPECT_EQ(*match_count, 0); }
    match_count = tests_tools::count_matching_lines_in_file(log_filepath, ".*DEBUG.*");
    EXPECT_NE(match_count, nullptr);
    if (match_count != nullptr) { EXPECT_EQ(*match_count, 0); }
    match_count = tests_tools::count_matching_lines_in_file(log_filepath, ".*CRAZY.*");
    EXPECT_NE(match_count, nullptr);
    if (match_count != nullptr) { EXPECT_EQ(*match_count, 0); }

    ed247_set_log(ED247_LOG_LEVEL_INFO, log_filepath);
    EXPECT_TRUE(file_exist(log_filepath));
    match_count = tests_tools::count_matching_lines_in_file(log_filepath, ".*ERROR.*");
    EXPECT_NE(match_count, nullptr);
    if (match_count != nullptr) { EXPECT_EQ(*match_count, 0); }
    match_count = tests_tools::count_matching_lines_in_file(log_filepath, ".*WARNING.*");
    EXPECT_NE(match_count, nullptr);
    if (match_count != nullptr) { EXPECT_EQ(*match_count, 0); }
    match_count = tests_tools::count_matching_lines_in_file(log_filepath, ".*INFO.*");
    EXPECT_NE(match_count, nullptr);
    if (match_count != nullptr) { EXPECT_GT(*match_count, 0); }
    match_count = tests_tools::count_matching_lines_in_file(log_filepath, ".*DEBUG.*");
    EXPECT_NE(match_count, nullptr);
    if (match_count != nullptr) { EXPECT_EQ(*match_count, 0); }
    match_count = tests_tools::count_matching_lines_in_file(log_filepath, ".*CRAZY.*");
    EXPECT_NE(match_count, nullptr);
    if (match_count != nullptr) { EXPECT_EQ(*match_count, 0); }

    ed247_set_log_level((ed247_log_level_t)(ED247_LOG_LEVEL_MAX + 10));
    ed247_get_log_level(&level);
    EXPECT_EQ((int)level, (int)ED247_LOG_LEVEL_MAX);

    ed247_set_log_level((ed247_log_level_t)(ED247_LOG_LEVEL_MIN - 1));
    ed247_get_log_level(&level);
    EXPECT_EQ((int)level, (int)ED247_LOG_LEVEL_MIN);

    ed247::log::delete_logger();
    unlink(log_filepath);
  }
}

int main(int argc, char **argv)
{
  if(argc >=1)
    config_path = argv[1];
  else
    config_path = "../config";

  tests_tools::display_ed247_lib_infos();
  SAY("Configuration path: " << config_path);

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
