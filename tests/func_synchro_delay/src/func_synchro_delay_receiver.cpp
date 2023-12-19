#include "tests_tools.h"
#include "synchronizer.h"
#include <unistd.h>

#define TEST_ACTOR_ID 2
#define TEST_OTHER_ACTOR_ID 1

static std::string config_path;

class syncroDelayFixture: public ::testing::Test {};

TEST(syncroDelayFixture, syncroDelay)
{
  // Let first actor perform test without a second actor
  SAY("Sleep a little before start...");
  usleep(1 * 1000 * 1000);
  SAY("Sleep done.");

  // Sync with actor1
  synchro_init(TEST_ACTOR_ID);
  synchro_wait(TEST_OTHER_ACTOR_ID);

  // Load context and streams
  ed247_context_t context(nullptr);
  ASSERT_EQ(ed247_load_file((config_path + "/ecic_func_synchro_delay_receiver.xml").c_str(), &context), ED247_STATUS_SUCCESS);

  ed247_stream_t stream_out;
  ASSERT_EQ(ed247_get_stream(context, "StreamOut", &stream_out), ED247_STATUS_SUCCESS);

  ed247_stream_t stream_in;
  ASSERT_EQ(ed247_get_stream(context, "StreamIn", &stream_in), ED247_STATUS_SUCCESS);

  // Check nothing can be received since the context was not loaded when first actor perform it send
  SAY("Check no data is available");
  ASSERT_EQ(ed247_wait_frame(context, nullptr, 1), ED247_STATUS_TIMEOUT);

  // Signal we have performed our tests
  synchro_wait(TEST_OTHER_ACTOR_ID);

  // Check we can receive data from StreamIn
  const char* data;
  uint32_t size;
  synchro_wait(TEST_OTHER_ACTOR_ID);
  ASSERT_EQ(ed247_wait_frame(context, nullptr, 1), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_pop_sample(stream_in, (const void**) &data, &size, nullptr, nullptr, nullptr, nullptr), ED247_STATUS_SUCCESS);
  ASSERT_EQ(size, 10);
}


int main(int argc, char **argv)
{
  config_path = (argc >=1)? argv[1] : "../config";

  tests_tools::display_ed247_lib_infos();
  SAY("Configuration path: " << config_path);

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
