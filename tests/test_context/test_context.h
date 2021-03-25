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

#ifndef _ED247_TEST_H_
#define _ED247_TEST_H_

#include "ed247.h"
#include "sync_entity.h"

#include "gtest/gtest.h"

#include <ostream>
#include <iostream>
#include <sstream>

#ifdef __linux__
    #define TEST_EXPORT __attribute__ ((visibility ("default")))
#elif _WIN32
    #ifdef TEST_EXPORTS
        #define  TEST_EXPORT __declspec(dllexport)
    #else
        #define  TEST_EXPORT __declspec(dllimport)
    #endif
#endif

struct TestParams {
    uint32_t src_id;
    uint32_t dst_id;
    std::string filepath;
};

class TestContext : public ::testing::TestWithParam<TestParams>
{
    protected:

        // Per-test-suite set-up
        // Called before the first test in this test suite
        // Can be omitted if not needed
        TestContext()
        {
            // Establish connexion with tester
            synchro::Entity::init();
            _actor = nullptr;
        }

        // Per-test-suite tear-down
        // Called after the last test in this test suite
        // Can be omitted if not needed
        ~TestContext()
        {
            // Close connexion with tester
            if(_actor)
                delete _actor;
        }

        void SetUp() override
        {
            std::cout << "# CONTEXT [" << GetParam().src_id << "] SETUP" << std::endl;

            synchro::sleep_us(10000);

            if(!_actor)
                _actor = new synchro::Entity(GetParam().src_id);

            ASSERT_NE(_actor, nullptr);

            synchro::sleep_us(10000);

            if(GetParam().src_id == 1){
                TestSend(); TestWait();
            }else{
                TestWait(); TestSend();
            }

            ASSERT_EQ(ed247_load(GetParam().filepath.c_str(),NULL,&_context), ED247_STATUS_SUCCESS);
        }

        void TearDown() override
        {
            std::cout << "# CONTEXT [" << GetParam().src_id << "] TEAR DOWN" << std::endl;

            if(GetParam().src_id == 1){
                TestSend(); //TestWait();
            }else{
                TestWait(); TestSend();
            }

            ASSERT_EQ(ed247_unload(_context), ED247_STATUS_SUCCESS);
        }

        void TestSend()
        {
            std::cout << "# CONTEXT [" << GetParam().src_id << "] SEND TO [" << GetParam().dst_id << "]" << std::endl;
            ASSERT_NE(_actor, nullptr);
            _actor->send(GetParam().dst_id);
        }

        void TestWait()
        {
            std::cout << "# CONTEXT[" << GetParam().src_id << "] WAIT FOR [" << GetParam().dst_id << "]" << std::endl;
            ASSERT_NE(_actor, nullptr);
            ASSERT_TRUE(_actor->wait(GetParam().dst_id));
        }

        ed247_context_t _context;
        synchro::Entity * _actor;
};

#ifdef __cplusplus
extern "C" {
#endif
void memhooks_section_start();

bool memhooks_section_stop();
#ifdef __cplusplus
}
#endif

#endif
