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

/**
    This test checks the warning and error messages returned to the user when the library is not correctly used.
**/

class RobustnessTests : public ::testing::Test
{

};

TEST(RobustnessTests, DuplicateChannelUID)
{
    ed247_context_t context;
    constexpr const char* logfile = "./ed247.logs";
    ed247_set_log(ED247_LOG_LEVEL_MIN, logfile);

    std::string filepath = config_path+"/ecic_unit_api_user_feedback.xml";
    ASSERT_EQ(ed247_load_file(filepath.c_str(), &context), ED247_STATUS_FAILURE);

    // TODO: insert brackets instead of dots when the compilers are regex compatibles
    const uint32_t* value = tests_tools::count_matching_lines_in_file(logfile,
            ".*Stream .A429StreamInput. uses an UID already registered in Channel .Channel0.");
    ASSERT_NE(value, (const uint32_t*)NULL);
    ASSERT_GE(*value, (uint32_t)1);
};

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
