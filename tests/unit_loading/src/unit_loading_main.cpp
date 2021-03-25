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

#include "test_context.h"

#include <ed247_logs.h>
#include <ed247_xml.h>

#include <string>

/********
 * Test *
 ********/

std::string config_path = "../config";

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

        std::string filepath = GetParam();
        root = std::dynamic_pointer_cast<Root>(load_filepath(filepath));
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

std::vector<std::string> configuration_files;

INSTANTIATE_TEST_CASE_P(LoadingTests, LoadingContext,
    ::testing::ValuesIn(configuration_files));

int main(int argc, char **argv)
{
    if(argc >=1)
        config_path = argv[1];
    else
        config_path = "../config";

    std::cout << "Configuration path: " << config_path << std::endl;

    configuration_files.push_back(config_path+"/ecic_unit_loading_a429.xml");
    configuration_files.push_back(config_path+"/ecic_unit_loading_a664.xml");
    configuration_files.push_back(config_path+"/ecic_unit_loading_a825.xml");
    configuration_files.push_back(config_path+"/ecic_unit_loading_serial.xml");
    configuration_files.push_back(config_path+"/ecic_unit_loading_dis.xml");
    configuration_files.push_back(config_path+"/ecic_unit_loading_ana.xml");
    configuration_files.push_back(config_path+"/ecic_unit_loading_nad.xml");
    configuration_files.push_back(config_path+"/ecic_unit_loading_vnad.xml");

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}