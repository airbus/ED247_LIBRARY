#ifndef _ED247_TWO_ACTOR_TEST_H_
#define _ED247_TWO_ACTOR_TEST_H_
#include "synchronizer.h"
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
#define TEST_SIGNAL(title) synchro_signal(GetParam().other_actor_id(), title, LOG_SHORTFILE, __LINE__)
#define TEST_WAIT(title) synchro_wait(GetParam().other_actor_id(), title, LOG_SHORTFILE, __LINE__)
#define TEST_SYNC_TITLE(title)                  \
  do {                                          \
    switch (GetParam().actor_id) {              \
    case TEST_ACTOR1_ID:                        \
      TEST_SIGNAL(title);                       \
      break;                                    \
    case TEST_ACTOR2_ID:                        \
      TEST_WAIT(title);                         \
      break;                                    \
    }                                           \
  } while (0)

#define TEST_SYNC_NO_TITLE() TEST_SYNC_TITLE(nullptr)

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
  TestContext() : _context(nullptr) { }
  ~TestContext() { }

  void SetUp() override {
    synchro_init(GetParam().actor_id);

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

  ed247_context_t _context;
};

#endif
