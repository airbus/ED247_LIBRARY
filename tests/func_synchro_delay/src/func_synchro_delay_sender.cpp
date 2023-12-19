#include "tests_tools.h"
#include "synchronizer.h"

#define TEST_ACTOR_ID 1
#define TEST_OTHER_ACTOR_ID 2 // TODO

static std::string config_path;

class syncroDelayFixture: public ::testing::Test {};

TEST(syncroDelayFixture, syncroDelay)
{
  // Load context and streams
  ed247_context_t context(nullptr);
  ASSERT_EQ(ed247_load_file((config_path + "/ecic_func_synchro_delay_sender.xml").c_str(), &context), ED247_STATUS_SUCCESS);

  ed247_stream_t stream_out;
  ASSERT_EQ(ed247_get_stream(context, "StreamOut", &stream_out), ED247_STATUS_SUCCESS);

  ed247_stream_t stream_in;
  ASSERT_EQ(ed247_get_stream(context, "StreamIn", &stream_in), ED247_STATUS_SUCCESS);

  // Send random data on stream_out before the receiver load the context (it shall be in sleep state)
  SAY("Send random data.");
  char data[10];
  memset(data, 42, 10);
  ASSERT_EQ(ed247_stream_push_sample(stream_out, &data, 10, nullptr, nullptr), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(context), ED247_STATUS_SUCCESS);

  // stream_in read data from the socket used as source to send stream_out
  // No data shall be received since no one send data to this socket
  // On windows, we may received an ICMP message 10054 - "Connection reset by peer"
  // https://copyprogramming.com/howto/windows-udp-sockets-recvfrom-fails-with-error-10054
  // This test validate that this behavior has been fixed
  SAY("Check there is no input data and recfrom success.");
  ASSERT_EQ(ed247_wait_frame(context, nullptr, 1), ED247_STATUS_TIMEOUT);

  // Send sync signal while other actor has not inialized the synchro API (it shall be in sleep state)
  // the synchro_signal() call shall wait for other actor to be ready
  synchro_init(TEST_ACTOR_ID);
  synchro_signal(TEST_OTHER_ACTOR_ID);

  // Wait ohter actor perform its tests
  synchro_signal(TEST_OTHER_ACTOR_ID);

  // Send random data and signal it
  ASSERT_EQ(ed247_stream_push_sample(stream_out, &data, 10, nullptr, nullptr), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(context), ED247_STATUS_SUCCESS);
  synchro_signal(TEST_OTHER_ACTOR_ID);
}


int main(int argc, char **argv)
{
  config_path = (argc >=1)? argv[1] : "../config";

  tests_tools::display_ed247_lib_infos();
  SAY("Configuration path: " << config_path);

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
