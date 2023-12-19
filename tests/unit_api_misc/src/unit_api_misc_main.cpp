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

#include "single_actor_test.h"


std::string config_path = "../config";

class UtApiMisc : public ::testing::Test {};

/******************************************************************************
Sequence checking the information getter
******************************************************************************/
TEST(UtApiMisc, InfoGetter)
{
    ed247_context_t context = nullptr;

    // Check global information for an unfiled ECIC
    std::string filepath = config_path+"/ecic_unit_api_misc_mini.xml";
    ASSERT_EQ(ed247_load_file(filepath.c_str(), &context), ED247_STATUS_SUCCESS);

    ASSERT_TRUE(strcmp(ed247_component_get_version(context), "") == 0);
    ASSERT_EQ(ed247_component_get_type(context), ED247_COMPONENT_TYPE_VIRTUAL);
    ASSERT_TRUE(strcmp(ed247_component_get_name(context), "ComponentWithoutOptions") == 0);
    ASSERT_TRUE(strcmp(ed247_component_get_comment(context), "") == 0);
    ASSERT_EQ(ed247_component_get_identifier(context), 0);
    ASSERT_EQ(ed247_component_get_standard_revision(context), ED247_STANDARD_ED247A);

    ASSERT_TRUE(strcmp(ed247_file_producer_get_identifier(context), "") == 0);
    ASSERT_TRUE(strcmp(ed247_file_producer_get_comment(context), "") == 0);
    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);

    // Load a second configuration with optional fields filled
    filepath = config_path+"/ecic_unit_api_misc.xml";
    ASSERT_EQ(ed247_load_file(filepath.c_str(), &context), ED247_STATUS_SUCCESS);

    ASSERT_TRUE(strcmp(ed247_component_get_version(context), "X.Y") == 0);
    ASSERT_EQ(ed247_component_get_type(context), ED247_COMPONENT_TYPE_BRIDGE);
    ASSERT_TRUE(strcmp(ed247_component_get_name(context), "ComponentWithAllOptions") == 0);
    ASSERT_TRUE(strcmp(ed247_component_get_comment(context), "Toutes les options du header sont renseignées") == 0);
    ASSERT_EQ(ed247_component_get_identifier(context), 0);
    ASSERT_EQ(ed247_component_get_standard_revision(context), ED247_STANDARD_ED247A);

    ASSERT_TRUE(strcmp(ed247_file_producer_get_identifier(context), "User") == 0);
    ASSERT_TRUE(strcmp(ed247_file_producer_get_comment(context), "COMMENT") == 0);


    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE__INVALID), (uint32_t)0);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_INT8), (uint32_t)1);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_INT16), (uint32_t)2);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_INT32), (uint32_t)4);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_INT64), (uint32_t)8);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_UINT8), (uint32_t)1);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_UINT16), (uint32_t)2);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_UINT32), (uint32_t)4);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_UINT64), (uint32_t)8);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_FLOAT32), (uint32_t)4);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_FLOAT64), (uint32_t)8);

    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
}

/******************************************************************************
Check the library identification routines
******************************************************************************/
TEST(UtApiMisc, IdentificationGetters)
{
    const char* value = NULL;
    value = ed247_get_implementation_name();
    ASSERT_STREQ(value, "ED247_LIBRARY");

    value = ed247_get_implementation_version();
    ASSERT_STREQ(value, TEST_PRODUCT_VERSION);
}

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
