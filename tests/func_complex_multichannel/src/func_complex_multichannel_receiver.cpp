#define TEST_ACTOR_ID TEST_ACTOR2_ID
#include "functional_test.h"

class complexMultichannelFixture : public TestContext {};

TEST_P(complexMultichannelFixture, complexMultichannelMain)
{
  // Note: The receiver ECIC do not declare all sent streams
  ed247_stream_list_t streams;
  ed247_stream_t afdx_stream;
  ed247_stream_t dis_stream;
  ASSERT_EQ(ed247_find_streams(_context, "AFDXStream", &streams), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(streams, &afdx_stream), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_find_streams(_context, "DisStream", &streams), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(streams, &dis_stream), ED247_STATUS_SUCCESS);

  ed247_signal_t dis01;
  ed247_signal_t dis02;
  ASSERT_EQ(ed247_get_signal(_context, "Signal00", &dis01), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_get_signal(_context, "Signal01", &dis02),ED247_STATUS_SUCCESS);

  ed247_stream_assistant_t dis_assistant;
  ASSERT_EQ(ed247_stream_get_assistant(dis_stream, &dis_assistant), ED247_STATUS_SUCCESS);

  TEST_SYNC();
  ASSERT_EQ(ed247_wait_during(_context, &streams, ED247_ONE_SECOND), ED247_STATUS_SUCCESS);

  const char* payload = nullptr;
  uint8_t* sample_data;
  size_t payload_size;

  ASSERT_EQ(ed247_stream_pop_sample(afdx_stream, (const void**)&payload, &payload_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(payload_size, 100);
  ASSERT_EQ(payload[10], 0x43);

  ASSERT_EQ(ed247_stream_assistant_pop_sample(dis_assistant, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);

  ASSERT_EQ(ed247_stream_assistant_read_signal(dis_assistant, dis01, (const void**) &sample_data, &payload_size), ED247_STATUS_SUCCESS);
  ASSERT_EQ(payload_size, 1);
  ASSERT_EQ(*sample_data, 255);

  ASSERT_EQ(ed247_stream_assistant_read_signal(dis_assistant, dis02, (const void**) &sample_data, &payload_size), ED247_STATUS_SUCCESS);
  ASSERT_EQ(payload_size, 1);
  ASSERT_EQ(*sample_data, 0);
}


std::vector<TestParams> stream_files;
INSTANTIATE_TEST_CASE_P(complexMultichannel, complexMultichannelFixture,
                        ::testing::ValuesIn(stream_files));

int main(int argc, char **argv)
{
  std::string config_path = (argc >=1)? argv[1] : "../config";

  tests_tools::display_ed247_lib_infos();
  SAY("Configuration path: " << config_path);

  stream_files.push_back({TEST_ACTOR_ID, config_path + "/ecic_func_complex_multichannel_receiver.xml"});

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
