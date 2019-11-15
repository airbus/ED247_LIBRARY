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

using namespace ed247::xml;

class LoadingContext : public ::testing::TestWithParam<std::string>
{

};

TEST_P(LoadingContext, Loading)
{
    std::shared_ptr<Root> root;
    try{
        std::ostringstream oss;
        oss << "Load content of [" << GetParam() << "]";
        RecordProperty("description",oss.str());

        std::string filepath{CONFIG_PATH"/ut_loading/"};
        filepath += GetParam();
        root = std::dynamic_pointer_cast<Root>(load(filepath));
    }
    catch(std::exception & e){
        LOG_ERROR() << "Error: " << e.what() << LOG_END;
        ASSERT_TRUE(false);
    }
    catch(...){
        LOG_ERROR() << "Error" << LOG_END;
        ASSERT_TRUE(false);
    }
};

const std::vector<std::string> configuration_files = {
    std::string("a429.xml"),
    std::string("a664.xml"),
    std::string("a825.xml"),
    std::string("serial.xml"),
    std::string("dis.xml"),
    std::string("ana.xml"),
    std::string("nad.xml"),
    std::string("vnad.xml")
};

INSTANTIATE_TEST_CASE_P(LoadingTests, LoadingContext,
    ::testing::ValuesIn(configuration_files));

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}