#define TEST_ACTOR_ID TEST_ACTOR2_ID
#include "functional_test.h"

static std::string config_path;

class bidirectionalFixture : public TestContext {};

// Only in sender
TEST_P(bidirectionalFixture, ecicCheck)
{
}


//
// Check loopback on different channels
//
TEST_P(bidirectionalFixture, loopback_different_channels)
{
  ed247_context_t context(nullptr);

  // Check loopback on different channels
  ASSERT_EQ(ed247_load_file((config_path + "/ecic_func_bidirectional_loopback_channels_receiver.xml").c_str(), &context),
            ED247_STATUS_SUCCESS);

  ed247_stream_list_t stream_list;
  ed247_stream_t input_stream, input_stream_mc;
  ASSERT_EQ(ed247_find_streams(context, "StreamIn", &stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(stream_list, &input_stream), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_find_streams(context, "StreamMCIn", &stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(stream_list, &input_stream_mc), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);

  TEST_SYNC("loopback on different channels");

  const char* input_payload = nullptr;
  uint32_t payload_size;
  ASSERT_EQ(ed247_wait_during(context, nullptr, ED247_ONE_SECOND), ED247_STATUS_SUCCESS);

  ASSERT_EQ(ed247_stream_pop_sample(input_stream, (const void**)&input_payload, &payload_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(payload_size, 100);
  ASSERT_EQ(input_payload[10], 0x01);

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

  ASSERT_EQ(ed247_load_file((config_path + "/ecic_func_bidirectional_loopback_stream_inout_receiver.xml").c_str(), &context),
            ED247_STATUS_SUCCESS);

  ed247_stream_list_t stream_list;
  ed247_stream_t bidir_stream, bidir_stream_mc;
  ASSERT_EQ(ed247_find_streams(context, "StreamInOut", &stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(stream_list, &bidir_stream), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_find_streams(context, "StreamMCInOut", &stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(stream_list, &bidir_stream_mc), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);

  TEST_SYNC("loopback on different channels");

  const char* input_payload = nullptr;
  uint32_t payload_size;
  ASSERT_EQ(ed247_wait_during(context, nullptr, ED247_ONE_SECOND), ED247_STATUS_SUCCESS);

  ASSERT_EQ(ed247_stream_pop_sample(bidir_stream, (const void**)&input_payload, &payload_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(payload_size, 4);
  ASSERT_EQ(input_payload[2], 0x01);

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
