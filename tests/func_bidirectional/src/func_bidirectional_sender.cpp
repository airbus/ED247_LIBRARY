#define TEST_ACTOR_ID TEST_ACTOR1_ID
#include "two_actors_test.h"

static std::string config_path;

class bidirectionalFixture : public TestContext {};

//
// ECIC check validation
//
TEST_P(bidirectionalFixture, ecicCheck)
{
  ed247_context_t context(nullptr);

  // UdpSocket: Bidirectional not supported
  ASSERT_EQ(ed247_load_file((config_path + "/ecic_func_bidirectional_updsocket_bidir.xml").c_str(), &context),
            ED247_STATUS_FAILURE);

  // UdpSocket: Undefined direction is valid if it can be deduce from stream direction
  ASSERT_EQ(ed247_load_file((config_path + "/ecic_func_bidirectional_updsocket_unset_valid.xml").c_str(), &context),
            ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);

  // UdpSocket: Undefined direction is invalid if it cannot be deduce from stream direction
  ASSERT_EQ(ed247_load_file((config_path + "/ecic_func_bidirectional_updsocket_unset_invalid.xml").c_str(), &context),
            ED247_STATUS_FAILURE);
}


//
// Check loopback on different channels
//
TEST_P(bidirectionalFixture, loopback_different_channels)
{
  ed247_context_t context(nullptr);
  char output_payload[500];
  const char* input_payload = nullptr;
  uint32_t payload_size;

  ASSERT_EQ(ed247_load_file((config_path + "/ecic_func_bidirectional_loopback_channels_sender.xml").c_str(), &context),
            ED247_STATUS_SUCCESS);

  ed247_stream_list_t stream_list;
  ed247_stream_t output_stream, output_stream_mc, input_stream, input_stream_mc;
  ASSERT_EQ(ed247_find_streams(context, "StreamOut", &stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(stream_list, &output_stream), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_find_streams(context, "StreamMCOut", &stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(stream_list, &output_stream_mc), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_find_streams(context, "StreamIn", &stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(stream_list, &input_stream), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_find_streams(context, "StreamMCIn", &stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(stream_list, &input_stream_mc), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);
  TEST_SYNC("Receiver ready"); // Wait receiver bind.

  // Unicast
  memset(output_payload, 0x01, 100);
  ASSERT_EQ(ed247_stream_push_sample(output_stream, output_payload, 100, NULL, NULL), ED247_STATUS_SUCCESS);

  ASSERT_EQ(ed247_send_pushed_samples(context), ED247_STATUS_SUCCESS);
  TEST_SYNC("unicast loopback on different channels");

  // Receive data we just have sent
  ASSERT_EQ(ed247_wait_frame(context, nullptr, ED247_ONE_SECOND), ED247_STATUS_SUCCESS);

  ASSERT_EQ(ed247_stream_pop_sample(input_stream, (const void**)&input_payload, &payload_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(payload_size, 100);
  ASSERT_EQ(input_payload[10], 0x01);

  TEST_SYNC("Receiver done");

  // Multicast
  memset(output_payload, 0x42, 100);
  ASSERT_EQ(ed247_stream_push_sample(output_stream_mc, output_payload, 100, NULL, NULL), ED247_STATUS_SUCCESS);

  ASSERT_EQ(ed247_send_pushed_samples(context), ED247_STATUS_SUCCESS);
  TEST_SYNC("multicast loopback on different channels");

  // Receive data we just have sent
  ASSERT_EQ(ed247_wait_frame(context, nullptr, ED247_ONE_SECOND), ED247_STATUS_SUCCESS);

  ASSERT_EQ(ed247_stream_pop_sample(input_stream_mc, (const void**)&input_payload, &payload_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(payload_size, 100);
  ASSERT_EQ(input_payload[10], 0x42);

  ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
}

//
// Check loopback on bidirectional streams
//
TEST_P(bidirectionalFixture, loopback_same_channels)
{
  ed247_context_t context(nullptr);
  char output_payload[500];
  const char* input_payload = nullptr;
  uint32_t payload_size;

  ASSERT_EQ(ed247_load_file((config_path + "/ecic_func_bidirectional_loopback_stream_inout_sender.xml").c_str(), &context),
            ED247_STATUS_SUCCESS);

  ed247_stream_list_t stream_list;
  ed247_stream_t bidir_stream, bidir_stream_mc;
  ASSERT_EQ(ed247_find_streams(context, "StreamInOut", &stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(stream_list, &bidir_stream), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_find_streams(context, "StreamMCInOut", &stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(stream_list, &bidir_stream_mc), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);
  TEST_SYNC("Receiver ready"); // Wait receiver bind.

  // Unicast
  memset(output_payload, 0x01, 4);
  ASSERT_EQ(ed247_stream_push_sample(bidir_stream, output_payload, 4, NULL, NULL), ED247_STATUS_SUCCESS);

  ASSERT_EQ(ed247_send_pushed_samples(context), ED247_STATUS_SUCCESS);
  TEST_SYNC("Unicast loopback on same channel");

  // Receive data we just have sent
  ASSERT_EQ(ed247_wait_frame(context, nullptr, ED247_ONE_SECOND), ED247_STATUS_SUCCESS);

  ASSERT_EQ(ed247_stream_pop_sample(bidir_stream, (const void**)&input_payload, &payload_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(payload_size, 4);
  ASSERT_EQ(input_payload[2], 0x01);

  TEST_SYNC("Receiver done");

  // Multicast
  memset(output_payload, 0x42, 4);
  ASSERT_EQ(ed247_stream_push_sample(bidir_stream_mc, output_payload, 4, NULL, NULL), ED247_STATUS_SUCCESS);

  ASSERT_EQ(ed247_send_pushed_samples(context), ED247_STATUS_SUCCESS);
  TEST_SYNC("Multicast loopback on same channel");

  // Receive data we just have sent
  ASSERT_EQ(ed247_wait_frame(context, nullptr, ED247_ONE_SECOND), ED247_STATUS_SUCCESS);

  ASSERT_EQ(ed247_stream_pop_sample(bidir_stream_mc, (const void**)&input_payload, &payload_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(payload_size, 4);
  ASSERT_EQ(input_payload[2], 0x42);

  ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
}


std::vector<TestParams> stream_files;
INSTANTIATE_TEST_CASE_P(bidirectional, bidirectionalFixture,
                        ::testing::ValuesIn(stream_files));

int main(int argc, char **argv)
{
  config_path = (argc >=1)? argv[1] : "../config";

  tests_tools::display_ed247_lib_infos();
  SAY("Configuration path: " << config_path);

  stream_files.push_back({TEST_ACTOR_ID, std::string()});

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
