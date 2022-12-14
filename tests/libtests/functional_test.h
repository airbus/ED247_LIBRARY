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

#ifndef _ED247_TEST_H_
#define _ED247_TEST_H_
#include "sync_entity.h"
#include "tests_tools.h"

// This framework is desing for two actors: one sending data an the other to receive them
enum test_actor_id_t {
  TEST_ACTOR1_ID = 1,
  TEST_ACTOR2_ID = 2
};

// Actor names
// Overwrite actor name before including this header
#ifndef TEST_ACTOR1_NAME
# define TEST_ACTOR1_NAME "sender"
#endif
#ifndef TEST_ACTOR2_NAME
# define TEST_ACTOR2_NAME "receiver"
#endif

#define TEST_ACTOR_NAME() ((GetParam().actor_id == TEST_ACTOR1_ID)? TEST_ACTOR1_NAME : TEST_ACTOR2_NAME)
#define TEST_OTHER_NAME() ((GetParam().actor_id == TEST_ACTOR2_ID)? TEST_ACTOR1_NAME : TEST_ACTOR2_NAME)

// Display a message prefixed by actor name
#define SAY_SELF(m) do { SAY(TEST_ACTOR_NAME() << " " << m); } while (0)

// Use TEST_SYNC(<title?>) to synchronize both actors
#define TEST_SYNC_MSG(title, m) do { SAY("[SYNC:" << title << " " << TEST_ACTOR_NAME() << "=>" << TEST_OTHER_NAME() << "] " << m);} while (0)
#define TEST_SIGNAL(title) do { TEST_SYNC_MSG(title, "Send signal."); test_signal(); } while(0)
#define TEST_WAIT(title) do { TEST_SYNC_MSG(title, "Wait for signal."); test_wait(); TEST_SYNC_MSG(title, "Signal received."); } while(0)


#define TEST_SYNC_TITLE(title)                  \
  do {                                          \
    switch (GetParam().actor_id) {              \
    case TEST_ACTOR1_ID:                        \
      TEST_SIGNAL(title);                       \
      TEST_WAIT(title);                         \
      break;                                    \
    case TEST_ACTOR2_ID:                        \
      TEST_WAIT(title);                         \
      TEST_SIGNAL(title);                       \
      break;                                    \
    }                                           \
  } while (0)

#define TEST_SYNC_NO_TITLE() TEST_SYNC_TITLE(std::string())

// Note 1: Ugly macros so TEST_SYNC(<title?>) works with or without <title> argument
// Note 2: All these are macros only to make SAY() correctly display __FILE__, __LINE__
#define TEST_SYNC_WRAP(unused, title, test_sync_macro, ...) test_sync_macro
#define TEST_SYNC(...) TEST_SYNC_WRAP(, ##__VA_ARGS__, TEST_SYNC_TITLE(__VA_ARGS__), TEST_SYNC_NO_TITLE())



struct TestParams {
  TestParams(test_actor_id_t id, std::string path) : actor_id(id), gtest_padding_bug_hack(0), filepath(path) {}
  test_actor_id_t actor_id;
  uint32_t gtest_padding_bug_hack;   // Workaround GTEST #3805. TODO: update GTEST version.
  std::string filepath;
  uint32_t other_actor_id() const { return (actor_id == TEST_ACTOR1_ID)? TEST_ACTOR2_ID : TEST_ACTOR1_ID; }
};

class TestContext : public ::testing::TestWithParam<TestParams>
{
protected:
  TestContext() : _context(nullptr), _actor(nullptr) {
    synchro::Entity::init();
  }

  ~TestContext() {
    delete _actor;
  }

  void SetUp() override {
    if(!_actor) {
      _actor = new synchro::Entity(GetParam().actor_id);
    }
    ASSERT_NE(_actor, nullptr);

    if (GetParam().filepath.empty() == false) {
      SAY_SELF("Load ECIC: " << GetParam().filepath);
      ASSERT_EQ(ed247_load_file(GetParam().filepath.c_str(), &_context), ED247_STATUS_SUCCESS);
    }
    TEST_SYNC("Setup");
  }

  void TearDown() override {
    if (_context) {
      ASSERT_EQ(ed247_unload(_context), ED247_STATUS_SUCCESS);
    }
  }

  void test_signal() {
    ASSERT_NE(_actor, nullptr);
    _actor->send(GetParam().other_actor_id());
  }

  void test_wait() {
    ASSERT_NE(_actor, nullptr);
    ASSERT_TRUE(_actor->wait(GetParam().other_actor_id()));
  }

  ed247_context_t   _context;
  synchro::Entity * _actor;
};

#endif
