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
#include "test_context.h"

#include <stdio.h>
#include <fstream>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include "gtest/gtest.h"

#include <memory>

/***********
 * Defines *
 ***********/

/********
 * Test *
 ********/

std::string config_path = "../config";

/******************************************************************************
Sequence checking the libed247_errors
******************************************************************************/
class UtApiMisc : public ::testing::Test {};

TEST(UtApiMisc, LastError)
{
    ed247_context_t context;
    const char* errors;

    // Check there is no error returned
    errors = libed247_errors();
    ASSERT_STREQ(errors, "No error");

    // Load config and test again
    std::string filepath = config_path+"/ecic_unit_api_misc.xml";
    ASSERT_EQ(ed247_load(filepath.c_str(), NULL, &context), ED247_STATUS_SUCCESS);
    errors = libed247_errors();
    ASSERT_STREQ(errors, "No error");

    // Check error
    ASSERT_EQ(ed247_get_runtime_metrics(context, NULL), ED247_STATUS_FAILURE);
    errors = libed247_errors();
    std::cout << "====" << std::endl;
    std::cout << errors << std::endl;
    std::cout << "====" << std::endl;
    ASSERT_STREQ(errors, "Invalid metrics pointer\n");

    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
}

/******************************************************************************
Sequence checking the information getter
******************************************************************************/
TEST(UtApiMisc, InfoGetter)
{
    ed247_context_t context = nullptr;
    const ed247_component_info_t* infos = nullptr;
    ed247_component_info_t expected_info_values = {
        "", // Version
        ED247_COMPONENT_TYPE_VIRTUAL, // Type
        "ComponentWithoutOptions", // Name
        "", // Comment
        ED247_STANDARD_ED247A, // Standard Revision
        0,
        { // File Producer
            "", // Identifier
            "" // Comment
        }
    };
    // First 2 calls are not functional, just check they do not make any crash in application
    ASSERT_EQ(ed247_component_get_info(NULL, &infos), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_component_get_info(context, NULL), ED247_STATUS_FAILURE);

    std::string filepath = config_path+"/ecic_unit_api_misc_mini.xml";
    ASSERT_EQ(ed247_load(filepath.c_str(), NULL, &context), ED247_STATUS_SUCCESS);
    memhooks_section_start();
    ASSERT_EQ(ed247_component_get_info(NULL, &infos), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_component_get_info(context, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_component_get_info(context, &infos), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(memhooks_section_stop());
    
    // Check content information
    ASSERT_EQ(infos->component_type, expected_info_values.component_type);
    ASSERT_TRUE(infos->name != NULL && strcmp(infos->name, expected_info_values.name) == 0);
    ASSERT_TRUE(infos->comment != NULL && strcmp(infos->comment, expected_info_values.comment) == 0);
    ASSERT_EQ(infos->standard_revision, expected_info_values.standard_revision);
    ASSERT_TRUE(infos->component_version != NULL && strcmp(infos->component_version, expected_info_values.component_version) == 0);
    ASSERT_TRUE(infos->file_producer.identifier != NULL && strcmp(infos->file_producer.identifier, expected_info_values.file_producer.identifier) == 0 );
    ASSERT_TRUE(infos->file_producer.comment != NULL && strcmp(infos->file_producer.comment, expected_info_values.file_producer.comment) == 0);
    
    // Load a second configuration with optional fields filled
    expected_info_values = {
        "X.Y", // Version
        ED247_COMPONENT_TYPE_BRIDGE, // Type
        "ComponentWithAllOptions", // Name
        "Toutes les options du header sont renseignées", // Comment
        ED247_STANDARD_ED247A, // Standard Revision
        0,
        { // File Producer
            "User", // Identifier
            "COMMENT" // Comment
        }
    };
    
    filepath = config_path+"/ecic_unit_api_misc.xml";
    ASSERT_EQ(ed247_load(filepath.c_str(), NULL, &context), ED247_STATUS_SUCCESS);
    memhooks_section_start();
    ASSERT_EQ(ed247_component_get_info(context, &infos), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_component_get_info(NULL, &infos), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_component_get_info(context, NULL), ED247_STATUS_FAILURE);
    memhooks_section_stop();
    
    // Check content information
    ASSERT_EQ(infos->component_type, expected_info_values.component_type);
    ASSERT_FALSE(strcmp(infos->name, expected_info_values.name));
    ASSERT_FALSE(strcmp(infos->comment, expected_info_values.comment));
    ASSERT_EQ(infos->standard_revision, expected_info_values.standard_revision);
    ASSERT_FALSE(strcmp(infos->component_version, expected_info_values.component_version));
    ASSERT_FALSE(strcmp(infos->file_producer.identifier, expected_info_values.file_producer.identifier));
    ASSERT_FALSE(strcmp(infos->file_producer.comment, expected_info_values.file_producer.comment));

    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE__INVALID), (size_t)0);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_INT8), (size_t)1);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_INT16), (size_t)2);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_INT32), (size_t)4);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_INT64), (size_t)8);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_UINT8), (size_t)1);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_UINT16), (size_t)2);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_UINT32), (size_t)4);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_UINT64), (size_t)8);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_FLOAT32), (size_t)4);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_FLOAT64), (size_t)8);
    
    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
}

/******************************************************************************
Check the library identification routines
******************************************************************************/
TEST(UtApiMisc, IdentificationGetters)
{
    const char* value = NULL;
    memhooks_section_start();
    value = ed247_get_implementation_name();
    memhooks_section_stop();
    ASSERT_STREQ(value, "ED247_LIBRARY");
    
    memhooks_section_start();
    value = ed247_get_implementation_version();
    memhooks_section_stop();
    ASSERT_STREQ(value, "1.1.0");
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