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
#include "ed247_internals.h"

ed247_timestamp_t timestamp1 = {123, 456};
ed247_timestamp_t timestamp2 = {456, 789};

ed247_status_t get_time_test1(ed247_time_sample_t time_sample, void *user_data)
{
    return libed247_update_time(time_sample, timestamp1.epoch_s, timestamp1.offset_ns);
}
ed247_status_t get_time_test2(ed247_time_sample_t time_sample, void *user_data)
{
    return libed247_update_time(time_sample, timestamp2.epoch_s, timestamp2.offset_ns);
}

TEST(TimeStamp, ManageHandlers)
{
    ed247::SimulationTimeHandler single = ed247::SimulationTimeHandler::get();
	ASSERT_EQ(single.is_valid(), false);
	
	single.set_handler(get_time_test1, NULL);
	ASSERT_EQ(single.is_valid(), true);
    ed247_timestamp_t timestamp;
    single.update_timestamp(timestamp);
	ASSERT_EQ(timestamp.epoch_s, timestamp1.epoch_s);
	ASSERT_EQ(timestamp.offset_ns, timestamp1.offset_ns);
	
	single.set_handler(get_time_test2, NULL);
	ASSERT_EQ(single.is_valid(), true);
    single.update_timestamp(timestamp);
	ASSERT_EQ(timestamp.epoch_s, timestamp2.epoch_s);
	ASSERT_EQ(timestamp.offset_ns, timestamp2.offset_ns);
	
	single.set_handler(nullptr, NULL);
	ASSERT_EQ(single.is_valid(), false);
}

TEST(TimeStamp, DefaultTimeHandler)
{
	ed247::SimulationTimeHandler single = ed247::SimulationTimeHandler::get();
	ASSERT_EQ(single.is_valid(), false);
	
	single.set_handler(libed247_set_simulation_time_ns, NULL);
	ASSERT_EQ(single.is_valid(), true);
    ed247_timestamp_t timestamp1, timestamp2;
    single.update_timestamp(timestamp1);
    single.update_timestamp(timestamp2);
	ASSERT_LE(timestamp1.epoch_s*10E9+timestamp1.offset_ns, timestamp2.epoch_s*10E9+timestamp2.offset_ns);

    single.update_timestamp(timestamp1);
    single.update_timestamp(timestamp2);
	ASSERT_LE(timestamp1.epoch_s*10E9+timestamp1.offset_ns, timestamp2.epoch_s*10E9+timestamp2.offset_ns);
	
    single.update_timestamp(timestamp1);
    single.update_timestamp(timestamp2);
	ASSERT_LE(timestamp1.epoch_s*10E9+timestamp1.offset_ns, timestamp2.epoch_s*10E9+timestamp2.offset_ns);
	
	single.set_handler(nullptr, NULL);
	ASSERT_EQ(single.is_valid(), false);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
