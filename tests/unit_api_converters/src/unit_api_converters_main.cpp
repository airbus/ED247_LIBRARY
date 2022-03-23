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

/******************************************************************************
This test aims to explicitly test the conversion functions provided by the API
******************************************************************************/
 
/******************************************************************************
Sequence checking the status conversions
There is no revert conversion from string to status because such parameters
are not supposed to appear in any ECIC configuration file
******************************************************************************/
class FromStatusType : public ::testing::TestWithParam<std::pair <ed247_status_t, const char*> > {};

TEST_P(FromStatusType, StatusConverter)
{
    malloc_count_start();
    ASSERT_EQ(strcmp(ed247_status_string(GetParam().first), GetParam().second), 0);
    ASSERT_EQ(malloc_count_stop(), 0);
}

std::vector<std::pair<ed247_status_t, const char*> > from_status_associations = {
    {ED247_STATUS_SUCCESS, "SUCCESS"},
    {ED247_STATUS_FAILURE, "FAILURE"},
    {ED247_STATUS_TIMEOUT, "TIMEOUT"},
    {ED247_STATUS_NODATA, "NODATA"},
    {(const ed247_status_t)5, "Unknown"}
};

INSTANTIATE_TEST_CASE_P(FromStatusConverters, FromStatusType,
    ::testing::ValuesIn(from_status_associations));

/******************************************************************************
Sequence checking the standard version conversions
******************************************************************************/
class FromStandardType : public ::testing::TestWithParam<std::pair <ed247_standard_t, const char*> > {};

TEST_P(FromStandardType, FromStandardConverter)
{
    malloc_count_start();
    ASSERT_EQ(strcmp(ed247_standard_string(GetParam().first), GetParam().second), 0);
    ASSERT_EQ(malloc_count_stop(), 0);
}

std::vector<std::pair<ed247_standard_t, const char*> > from_standard_associations = {
    {ED247_STANDARD__INVALID, "Unknown"},
    {ED247_STANDARD_ED247, "-"},
    {ED247_STANDARD_ED247A, "A"},
    {ED247_STANDARD__COUNT, "Unknown"}
};

INSTANTIATE_TEST_CASE_P(FromStandardConverters, FromStandardType,
    ::testing::ValuesIn(from_standard_associations));

class ToStandardType : public ::testing::TestWithParam<std::pair <const char*, ed247_standard_t> > {};

TEST_P(ToStandardType, ToStandardConverter)
{
    malloc_count_start();
    ASSERT_EQ(ed247_standard_from_string(GetParam().first), GetParam().second);
    ASSERT_EQ(malloc_count_stop(), 0);
}

std::vector<std::pair<const char*, ed247_standard_t> > to_standard_associations = {
    {"Invalid", ED247_STANDARD__INVALID},
    {"-", ED247_STANDARD_ED247},
    {"A", ED247_STANDARD_ED247A, },
    {"B", ED247_STANDARD__INVALID},
};

INSTANTIATE_TEST_CASE_P(ToStandardConverters, ToStandardType,
    ::testing::ValuesIn(to_standard_associations));

/******************************************************************************
Sequence checking the direction (incomming or outgoing) conversions
******************************************************************************/
class FromDirectionType : public ::testing::TestWithParam<std::pair <ed247_direction_t, const char*> > {};

TEST_P(FromDirectionType, FromDirectionConverter)
{
    malloc_count_start();
    ASSERT_EQ(strcmp(ed247_direction_string(GetParam().first), GetParam().second), 0);
    ASSERT_EQ(malloc_count_stop(), 0);
}

std::vector<std::pair<ed247_direction_t, const char*> > from_direction_associations = {
    {ED247_DIRECTION__INVALID, "Unknown"},
    {ED247_DIRECTION_IN, "In"},
    {ED247_DIRECTION_OUT, "Out"},
    {ED247_DIRECTION_INOUT, "InOut"}
};

INSTANTIATE_TEST_CASE_P(FromDirectionConverters, FromDirectionType,
    ::testing::ValuesIn(from_direction_associations));

class ToDirectionType : public ::testing::TestWithParam<std::pair <const char*, ed247_direction_t> > {};

TEST_P(ToDirectionType, ToDirectionConverter)
{
    malloc_count_start();
    ASSERT_EQ(ed247_direction_from_string(GetParam().first), GetParam().second);
    ASSERT_EQ(malloc_count_stop(), 0);
}

std::vector<std::pair<const char*, ed247_direction_t> > to_direction_associations = {
    {"In", ED247_DIRECTION_IN},
    {"Out", ED247_DIRECTION_OUT},
    {"InOut", ED247_DIRECTION_INOUT},
    {"Other", ED247_DIRECTION__INVALID}
};

INSTANTIATE_TEST_CASE_P(ToDirectionConverters, ToDirectionType,
    ::testing::ValuesIn(to_direction_associations));

/******************************************************************************
Sequence checking the YES/NO conversions
******************************************************************************/
class FromYesNoType : public ::testing::TestWithParam<std::pair <ed247_yesno_t, const char*> > {};

TEST_P(FromYesNoType, FromYesNoConverter)
{
    malloc_count_start();
    ASSERT_EQ(strcmp(ed247_yesno_string(GetParam().first), GetParam().second), 0);
    ASSERT_EQ(malloc_count_stop(), 0);
}

std::vector<std::pair<ed247_yesno_t, const char*> > from_yesno_associations = {
    {ED247_YESNO_NO, "No"},
    {ED247_YESNO_YES, "Yes"},
    {ED247_YESNO__INVALID, "Unknown"}
};

INSTANTIATE_TEST_CASE_P(FromYesNoConverter, FromYesNoType,
    ::testing::ValuesIn(from_yesno_associations));

class ToYesNoType : public ::testing::TestWithParam<std::pair <const char*, ed247_yesno_t> > {};

TEST_P(ToYesNoType, ToYesNoConverter)
{
    malloc_count_start();
    ASSERT_EQ(ed247_yesno_from_string(GetParam().first), GetParam().second);
    ASSERT_EQ(malloc_count_stop(), 0);
}

std::vector<std::pair<const char*, ed247_yesno_t> > to_yesno_associations = {
    {"Yes", ED247_YESNO_YES},
    {"No", ED247_YESNO_NO},
    {"Other", ED247_YESNO__INVALID},
    {" YES", ED247_YESNO__INVALID},
    {"NO ", ED247_YESNO__INVALID},
    {" 0 ", ED247_YESNO__INVALID},
    {"Other", ED247_YESNO__INVALID},
};

INSTANTIATE_TEST_CASE_P(ToYesNoConverters, ToYesNoType,
    ::testing::ValuesIn(to_yesno_associations));

/******************************************************************************
Sequence checking the component types conversions
******************************************************************************/
class FromComponentType : public ::testing::TestWithParam<std::pair <ed247_component_type_t, const char*> > {};

TEST_P(FromComponentType, FromComponentConverter)
{
    malloc_count_start();
    ASSERT_EQ(strcmp(ed247_component_type_string(GetParam().first), GetParam().second), 0);
    ASSERT_EQ(malloc_count_stop(), 0);
}

std::vector<std::pair<ed247_component_type_t, const char*> > from_component_associations = {
    {ED247_COMPONENT_TYPE__INVALID, "Unknown"},
    {ED247_COMPONENT_TYPE_VIRTUAL, "Virtual"},
    {ED247_COMPONENT_TYPE_BRIDGE, "Bridge"},
    {ED247_COMPONENT_TYPE__COUNT, "Unknown"}
};

INSTANTIATE_TEST_CASE_P(FromComponentConverter, FromComponentType,
    ::testing::ValuesIn(from_component_associations));

class ToComponentType : public ::testing::TestWithParam<std::pair <const char*, ed247_component_type_t> > {};

TEST_P(ToComponentType, ToComponentConverter)
{
    malloc_count_start();
    ASSERT_EQ(ed247_component_type_from_string(GetParam().first), GetParam().second);
    ASSERT_EQ(malloc_count_stop(), 0);
}

std::vector<std::pair<const char*, ed247_component_type_t> > to_component_associations = {
    {"Virtual", ED247_COMPONENT_TYPE_VIRTUAL},
    {"Bridge", ED247_COMPONENT_TYPE_BRIDGE},
    {"virtual", ED247_COMPONENT_TYPE__INVALID},
    {"hybrid", ED247_COMPONENT_TYPE__INVALID},
    {"Real", ED247_COMPONENT_TYPE__INVALID},
    {"Other", ED247_COMPONENT_TYPE__INVALID},
    {"Test", ED247_COMPONENT_TYPE__INVALID},
    {"Unknown", ED247_COMPONENT_TYPE__INVALID}
};

INSTANTIATE_TEST_CASE_P(ToComponentConverters, ToComponentType,
    ::testing::ValuesIn(to_component_associations));

/******************************************************************************
Sequence checking the component types conversions
******************************************************************************/
class FromStreamType : public ::testing::TestWithParam<std::pair <ed247_stream_type_t, const char*> > {};

TEST_P(FromStreamType, FromStreamConverter)
{
    malloc_count_start();
    ASSERT_EQ(strcmp(ed247_stream_type_string(GetParam().first), GetParam().second), 0);
    ASSERT_EQ(malloc_count_stop(), 0);
}

std::vector<std::pair<ed247_stream_type_t, const char*> > from_stream_associations = {
    {ED247_STREAM_TYPE__INVALID, "Unknown"},
    {ED247_STREAM_TYPE_A664, "A664"},
    {ED247_STREAM_TYPE_A429, "A429"},
    {ED247_STREAM_TYPE_A825, "A825"},
    {ED247_STREAM_TYPE_M1553, "M1553"},
    {ED247_STREAM_TYPE_SERIAL, "SERIAL"},
    {ED247_STREAM_TYPE_AUDIO, "AUDIO"},
    {ED247_STREAM_TYPE_VIDEO, "VIDEO"},
    {ED247_STREAM_TYPE_ETHERNET, "ETHERNET"},
    {ED247_STREAM_TYPE_ANALOG, "ANALOG"},
    {ED247_STREAM_TYPE_DISCRETE, "DISCRETE"},
    {ED247_STREAM_TYPE_NAD, "NAD"},
    {ED247_STREAM_TYPE_VNAD, "VNAD"},
    {ED247_STREAM_TYPE__COUNT, "Unknown"}
};

INSTANTIATE_TEST_CASE_P(FromStreamConverter, FromStreamType,
    ::testing::ValuesIn(from_stream_associations));

class ToStreamType : public ::testing::TestWithParam<std::pair <const char*, ed247_stream_type_t> > {};

TEST_P(ToStreamType, ToStreamConverter)
{
    malloc_count_start();
    ASSERT_EQ(ed247_stream_type_from_string(GetParam().first), GetParam().second);
    ASSERT_EQ(malloc_count_stop(), 0);
}

std::vector<std::pair<const char*, ed247_stream_type_t> > to_stream_associations = {
    {"A664", ED247_STREAM_TYPE_A664},
    {"A429", ED247_STREAM_TYPE_A429},
    {"A825", ED247_STREAM_TYPE_A825},
    {"M1553", ED247_STREAM_TYPE_M1553},
    {"SERIAL", ED247_STREAM_TYPE_SERIAL},
    {"AUDIO", ED247_STREAM_TYPE_AUDIO},
    {"VIDEO", ED247_STREAM_TYPE_VIDEO},
    {"ETHERNET", ED247_STREAM_TYPE_ETHERNET},
    {"ANALOG", ED247_STREAM_TYPE_ANALOG},
    {"DISCRETE", ED247_STREAM_TYPE_DISCRETE},
    {"NAD", ED247_STREAM_TYPE_NAD},
    {"VNAD", ED247_STREAM_TYPE_VNAD},
    {"Other", ED247_STREAM_TYPE__INVALID}
};

INSTANTIATE_TEST_CASE_P(ToStreamConverters, ToStreamType,
    ::testing::ValuesIn(to_stream_associations));

/******************************************************************************
Sequence checking the signal types conversions
******************************************************************************/
class FromSignalType : public ::testing::TestWithParam<std::pair <ed247_signal_type_t, const char*> > {};

TEST_P(FromSignalType, FromSignalConverter)
{
    malloc_count_start();
    ASSERT_EQ(strcmp(ed247_signal_type_string(GetParam().first), GetParam().second), 0);
    ASSERT_EQ(malloc_count_stop(), 0);
}

std::vector<std::pair<ed247_signal_type_t, const char*> > from_signal_associations = {
    {ED247_SIGNAL_TYPE__INVALID, "Unknown"},
    {ED247_SIGNAL_TYPE_ANALOG, "ANALOG"},
    {ED247_SIGNAL_TYPE_DISCRETE, "DISCRETE"},
    {ED247_SIGNAL_TYPE_NAD, "NAD"},
    {ED247_SIGNAL_TYPE_VNAD, "VNAD"}
};

INSTANTIATE_TEST_CASE_P(FromSignalConverter, FromSignalType,
    ::testing::ValuesIn(from_signal_associations));

class ToSignalType : public ::testing::TestWithParam<std::pair <const char*, ed247_signal_type_t> > {};

TEST_P(ToSignalType, ToSignalConverter)
{
    malloc_count_start();
    ASSERT_EQ(ed247_signal_type_from_string(GetParam().first), GetParam().second);
    ASSERT_EQ(malloc_count_stop(), 0);
}

std::vector<std::pair<const char*, ed247_signal_type_t> > to_signal_associations = {
    {"ANALOG", ED247_SIGNAL_TYPE_ANALOG},
    {"DISCRETE", ED247_SIGNAL_TYPE_DISCRETE},
    {"NAD", ED247_SIGNAL_TYPE_NAD},
    {"VNAD", ED247_SIGNAL_TYPE_VNAD},
    {"Unknown", ED247_SIGNAL_TYPE__INVALID}
};

INSTANTIATE_TEST_CASE_P(ToSignalConverters, ToSignalType,
    ::testing::ValuesIn(to_signal_associations));

/******************************************************************************
Sequence checking the NAD types conversions
******************************************************************************/
class FromNADType : public ::testing::TestWithParam<std::pair <ed247_nad_type_t, const char*> > {};

TEST_P(FromNADType, FromNADConverter)
{
    malloc_count_start();
    ASSERT_EQ(strcmp(ed247_nad_type_string(GetParam().first), GetParam().second), 0);
    ASSERT_EQ(malloc_count_stop(), 0);
}

std::vector<std::pair<ed247_nad_type_t, const char*> > from_nad_associations = {
    {ED247_NAD_TYPE__INVALID, "Unknown"},
    {ED247_NAD_TYPE_INT8, "int8"},
    {ED247_NAD_TYPE_INT16, "int16"},
    {ED247_NAD_TYPE_INT32, "int32"},
    {ED247_NAD_TYPE_INT64, "int64"},
    {ED247_NAD_TYPE_UINT8, "uint8"},
    {ED247_NAD_TYPE_UINT16, "uint16"},
    {ED247_NAD_TYPE_UINT32, "uint32"},
    {ED247_NAD_TYPE_UINT64, "uint64"},
    {ED247_NAD_TYPE_FLOAT32, "float32"},
    {ED247_NAD_TYPE_FLOAT64, "float64"},
    {ED247_NAD_TYPE__COUNT, "Unknown"}
};

INSTANTIATE_TEST_CASE_P(FromNADConverter, FromNADType,
    ::testing::ValuesIn(from_nad_associations));

class ToNADType : public ::testing::TestWithParam<std::pair <const char*, ed247_nad_type_t> > {};

TEST_P(ToNADType, ToNADConverter)
{
    malloc_count_start();
    ASSERT_EQ(ed247_nad_type_from_string(GetParam().first), GetParam().second);
    ASSERT_EQ(malloc_count_stop(), 0);
}

std::vector<std::pair<const char*, ed247_nad_type_t> > to_nad_associations = {
    {"int8", ED247_NAD_TYPE_INT8},
    {"int16", ED247_NAD_TYPE_INT16},
    {"int32", ED247_NAD_TYPE_INT32},
    {"int64", ED247_NAD_TYPE_INT64},
    {"uint8", ED247_NAD_TYPE_UINT8},
    {"uint16", ED247_NAD_TYPE_UINT16},
    {"uint32", ED247_NAD_TYPE_UINT32},
    {"uint64", ED247_NAD_TYPE_UINT64},
    {"float32", ED247_NAD_TYPE_FLOAT32},
    {"float64", ED247_NAD_TYPE_FLOAT64},
    {"Unknown", ED247_NAD_TYPE__INVALID},
    {"Other", ED247_NAD_TYPE__INVALID}
};

INSTANTIATE_TEST_CASE_P(ToNADConverters, ToNADType,
    ::testing::ValuesIn(to_nad_associations));
    
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
