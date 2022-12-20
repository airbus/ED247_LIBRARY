#define TEST_ACTOR_ID TEST_ACTOR2_ID
#include "functional_test.h"

class applicationFifoFixture : public TestContext {};

TEST_P(applicationFifoFixture, applicationFifoMain)
{
  ed247_stream_list_t afdx_streams;
  ed247_stream_t afdx_stream;
  ASSERT_EQ(ed247_find_streams(_context, "AFDXStream1", &afdx_streams), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(afdx_streams, &afdx_stream), ED247_STATUS_SUCCESS);
  ed247_stream_list_free(afdx_streams);

  TEST_SYNC("3 samples");
  ASSERT_EQ(ed247_wait_during(_context, &afdx_streams, ED247_ONE_SECOND), ED247_STATUS_SUCCESS);

  const char* payload = nullptr;
  uint32_t payload_size;

  ASSERT_EQ(ed247_stream_pop_sample(afdx_stream, (const void**)&payload, &payload_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(payload_size, 100);
  ASSERT_EQ(payload[10], 0x01);

  ASSERT_EQ(ed247_stream_pop_sample(afdx_stream, (const void**)&payload, &payload_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(payload_size, 100);
  ASSERT_EQ(payload[10], 0x02);

  ASSERT_EQ(ed247_stream_pop_sample(afdx_stream, (const void**)&payload, &payload_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(payload_size, 100);
  ASSERT_EQ(payload[10], 0x03);
}


std::vector<TestParams> stream_files;
INSTANTIATE_TEST_CASE_P(applicationFifo, applicationFifoFixture,
                        ::testing::ValuesIn(stream_files));

int main(int argc, char **argv)
{
  std::string config_path = (argc >=1)? argv[1] : "../config";

  tests_tools::display_ed247_lib_infos();
  SAY("Configuration path: " << config_path);

  stream_files.push_back({TEST_ACTOR_ID, config_path + "/ecic_func_application_fifo_receiver.xml"});

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
