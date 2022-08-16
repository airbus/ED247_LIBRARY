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
#include "ed247_xml.h"

std::string config_path = "../config";

class LoadingContext : public ::testing::TestWithParam<std::string>
{

};

TEST_P(LoadingContext, Loading)
{
    std::unique_ptr<ed247::xml::Component> component;
    try{
        RecordProperty("description", strize() << "Load content of [" << GetParam() << "]");

        std::string filepath = GetParam();
        component = ed247::xml::load_filepath(filepath);
    }
    catch(std::exception & e){
        SAY("Error: " << e.what());
        ASSERT_TRUE(false);
    }
    catch(...){
        SAY("Error");
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

    SAY("Configuration path: " << config_path);

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
