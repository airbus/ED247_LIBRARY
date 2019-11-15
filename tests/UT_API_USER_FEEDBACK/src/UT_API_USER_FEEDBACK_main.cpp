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

#include <ed247_logs.h>
#include <ed247_xml.h>
#include <ed247_test.h>

#include <string>

#include <gtest/gtest.h>
#include <gmock/gmock.h>


/**
    This test checks the warning and error messages returned to the user when the library is not correctly used.
**/

using namespace ed247;

class RobustnessTests : public ::testing::Test
{

};

TEST(RobustnessTests, DuplicateChannelUID)
{
    ed247_context_t context;
    
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
    ASSERT_EQ(ed247_load(CONFIG_PATH"/ut_api_user_feedback/DuplicateUID.xml", NULL, &context), ED247_STATUS_FAILURE);
    
    // TODO: insert brackets instead of dots when the compilers are regex compatibles
    const uint32_t* value = test::count_matching_lines_in_file(logfile,
            "Stream .A429StreamInput. uses an UID already registered in Channel .Channel0.");
    ASSERT_NE(value, (const uint32_t*)NULL);
    ASSERT_EQ(*value, (uint32_t)1);
};

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    
    return RUN_ALL_TESTS();
}
