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
This specific application covers the case when the log file cannot be writen
The loading procedure must still be successfull
******************************************************************************/
class LogConfigurationTest : public ::testing::Test{};

TEST(LogConfigurationTest, Loading)
{
    ed247_context_t context;
    const char* filename = CONFIG_PATH"/ft_logging/ecic_no_optional_desc.xml";

    // Set an unvalid path (empty) so that the file cannot be written
#ifdef __linux__
    setenv("ED247_LOG_FILEPATH", "", 1);
#else
    _putenv_s("ED247_LOG_FILEPATH", "");
#endif
    
    // Run the sequence of load
    ASSERT_EQ(ed247_load(filename, NULL, &context), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
}



int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}