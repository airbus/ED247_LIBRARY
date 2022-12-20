#define TEST_ACTOR_ID TEST_ACTOR1_ID
#include "functional_test.h"

class applicationFifoFixture : public TestContext {};

TEST_P(applicationFifoFixture, applicationFifoMain)
{
  ed247_stream_list_t afdx_streams;
  ed247_stream_t afdx_stream;
  ASSERT_EQ(ed247_find_streams(_context, "AFDXStream1", &afdx_streams), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(afdx_streams, &afdx_stream), ED247_STATUS_SUCCESS);
  ed247_stream_list_free(afdx_streams);

  char payload[500];

  memset(payload, 0x01, 100);
  ASSERT_EQ(ed247_stream_push_sample(afdx_stream, payload, 100, NULL, NULL), ED247_STATUS_SUCCESS);

  memset(payload, 0x02, 100);
  ASSERT_EQ(ed247_stream_push_sample(afdx_stream, payload, 100, NULL, NULL), ED247_STATUS_SUCCESS);

  memset(payload, 0x03, 100);
  ASSERT_EQ(ed247_stream_push_sample(afdx_stream, payload, 100, NULL, NULL), ED247_STATUS_SUCCESS);

  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

  // Payload sent
  TEST_SYNC("3 samples");
}


std::vector<TestParams> stream_files;
INSTANTIATE_TEST_CASE_P(applicationFifo, applicationFifoFixture,
                        ::testing::ValuesIn(stream_files));

int main(int argc, char **argv)
{
  std::string config_path = (argc >=1)? argv[1] : "../config";

  tests_tools::display_ed247_lib_infos();
  SAY("Configuration path: " << config_path);

  stream_files.push_back({TEST_ACTOR_ID, config_path + "/ecic_func_application_fifo_sender.xml"});

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
