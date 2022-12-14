#define TEST_ACTOR_ID TEST_ACTOR1_ID
#include "functional_test.h"

class complexMultichannelFixture : public TestContext {};

TEST_P(complexMultichannelFixture, complexMultichannelMain)
{
  // Note: The receiver ECIC do not declare all sent streams

  ed247_stream_list_t streams;
  ed247_stream_t afdx_stream_not_received;
  ed247_stream_t afdx_stream;
  ed247_stream_t dis_stream;
  ASSERT_EQ(ed247_find_streams(_context, "AFDXNotReceived", &streams), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(streams, &afdx_stream_not_received), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);

  ASSERT_EQ(ed247_find_streams(_context, "AFDXStream", &streams), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(streams, &afdx_stream), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);

  ASSERT_EQ(ed247_find_streams(_context, "DisStream", &streams), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(streams, &dis_stream), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);

  ed247_signal_t dis01;
  ed247_signal_t dis02;
  ASSERT_EQ(ed247_get_signal(_context, "Signal00", &dis01), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_get_signal(_context, "Signal01", &dis02),ED247_STATUS_SUCCESS);

  ed247_stream_assistant_t dis_assistant;
  ASSERT_EQ(ed247_stream_get_assistant(dis_stream, &dis_assistant), ED247_STATUS_SUCCESS);

  char payload[500];

  memset(payload, 0x42, 100);
  ASSERT_EQ(ed247_stream_push_sample(afdx_stream_not_received, payload, 100, NULL, NULL), ED247_STATUS_SUCCESS);

  memset(payload, 0x43, 100);
  ASSERT_EQ(ed247_stream_push_sample(afdx_stream, payload, 100, NULL, NULL), ED247_STATUS_SUCCESS);

  uint8_t sample = 255;
  ASSERT_EQ(ed247_stream_assistant_write_signal(dis_assistant, dis01, (void*)&sample, 1), ED247_STATUS_SUCCESS);
  sample = 0;
  ASSERT_EQ(ed247_stream_assistant_write_signal(dis_assistant, dis02, (void*)&sample, 1), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_assistant_push_sample(dis_assistant, NULL, NULL), ED247_STATUS_SUCCESS);

  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

  // Payload sent
  TEST_SYNC("3 samples");
}


std::vector<TestParams> stream_files;
INSTANTIATE_TEST_CASE_P(complexMultichannel, complexMultichannelFixture,
                        ::testing::ValuesIn(stream_files));

int main(int argc, char **argv)
{
  std::string config_path = (argc >=1)? argv[1] : "../config";

  tests_tools::display_ed247_lib_infos();
  SAY("Configuration path: " << config_path);

  stream_files.push_back({TEST_ACTOR_ID, config_path + "/ecic_func_complex_multichannel_sender.xml"});

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
