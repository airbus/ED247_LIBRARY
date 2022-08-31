#define TEST_ACTOR_ID TEST_ACTOR2_ID
#include "functional_test.h"

static std::string config_path;

class multiAddressFixture : public TestContext {};

//
// Check loopback on multiAddress streams
//
TEST_P(multiAddressFixture, main)
{
  ed247_stream_list_t stream_list;
  const char* input_payload = nullptr;
  uint32_t payload_size;

  ed247_stream_t stream1, stream2, stream3;
  ASSERT_EQ(ed247_find_streams(_context, "Stream1In", &stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(stream_list, &stream1), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_find_streams(_context, "Stream2In", &stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(stream_list, &stream2), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_find_streams(_context, "Stream3In", &stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(stream_list, &stream3), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);

  TEST_SYNC("Unicast");
  ASSERT_EQ(ed247_wait_during(_context, nullptr, ED247_100_MILI), ED247_STATUS_SUCCESS);

  // Check we have received each stream and only once
  ASSERT_EQ(ed247_stream_pop_sample(stream1, (const void**)&input_payload, &payload_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(payload_size, 10);
  ASSERT_EQ(input_payload[8], (char)0x91);
  ASSERT_EQ(ed247_stream_pop_sample(stream1, (const void**)&input_payload, &payload_size, NULL, NULL, NULL, NULL), ED247_STATUS_NODATA);

  ASSERT_EQ(ed247_stream_pop_sample(stream2, (const void**)&input_payload, &payload_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(payload_size, 10);
  ASSERT_EQ(input_payload[8], (char)0x91);
  ASSERT_EQ(ed247_stream_pop_sample(stream2, (const void**)&input_payload, &payload_size, NULL, NULL, NULL, NULL), ED247_STATUS_NODATA);

  ASSERT_EQ(ed247_stream_pop_sample(stream3, (const void**)&input_payload, &payload_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(payload_size, 10);
  ASSERT_EQ(input_payload[8], (char)0x91);
  ASSERT_EQ(ed247_stream_pop_sample(stream3, (const void**)&input_payload, &payload_size, NULL, NULL, NULL, NULL), ED247_STATUS_NODATA);

  TEST_SYNC("Unicast validated");


  ed247_stream_t stream1_mc, stream2_mc, stream3_mc;
  ASSERT_EQ(ed247_find_streams(_context, "StreamMC1In", &stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(stream_list, &stream1_mc), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_find_streams(_context, "StreamMC2In", &stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(stream_list, &stream2_mc), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_find_streams(_context, "StreamMC3In", &stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(stream_list, &stream3_mc), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);

  TEST_SYNC("Multicast");
  ASSERT_EQ(ed247_wait_during(_context, nullptr, ED247_100_MILI), ED247_STATUS_SUCCESS);

  // Check we have received each stream and only once
  ASSERT_EQ(ed247_stream_pop_sample(stream1_mc, (const void**)&input_payload, &payload_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(payload_size, 10);
  ASSERT_EQ(input_payload[8], (char)0x84);
  ASSERT_EQ(ed247_stream_pop_sample(stream1_mc, (const void**)&input_payload, &payload_size, NULL, NULL, NULL, NULL), ED247_STATUS_NODATA);

  ASSERT_EQ(ed247_stream_pop_sample(stream2_mc, (const void**)&input_payload, &payload_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(payload_size, 10);
  ASSERT_EQ(input_payload[8], (char)0x84);
  ASSERT_EQ(ed247_stream_pop_sample(stream2_mc, (const void**)&input_payload, &payload_size, NULL, NULL, NULL, NULL), ED247_STATUS_NODATA);

  ASSERT_EQ(ed247_stream_pop_sample(stream3_mc, (const void**)&input_payload, &payload_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(payload_size, 10);
  ASSERT_EQ(input_payload[8], (char)0x84);
  ASSERT_EQ(ed247_stream_pop_sample(stream3_mc, (const void**)&input_payload, &payload_size, NULL, NULL, NULL, NULL), ED247_STATUS_NODATA);
}



std::vector<TestParams> stream_files;
INSTANTIATE_TEST_CASE_P(multiAddress, multiAddressFixture,
                        ::testing::ValuesIn(stream_files));

int main(int argc, char **argv)
{
  config_path = (argc >=1)? argv[1] : "../config";

  tests_tools::display_ed247_lib_infos();
  SAY("Configuration path: " << config_path);

  stream_files.push_back({TEST_ACTOR_ID, config_path + "/ecic_func_multi_address_receiver.xml"});

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
