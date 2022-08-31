#define TEST_ACTOR_ID TEST_ACTOR1_ID
#include "functional_test.h"

static std::string config_path;

class multiAddressFixture : public TestContext {};

//
// Check loopback on multiAddress streams
//
TEST_P(multiAddressFixture, main)
{
  ed247_stream_list_t stream_list;
  char output_payload[500];

  ed247_stream_t stream_out;
  ASSERT_EQ(ed247_find_streams(_context, "StreamOut", &stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(stream_list, &stream_out), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);

  memset(output_payload, 0x91, 10);
  ASSERT_EQ(ed247_stream_push_sample(stream_out, output_payload, 10, NULL, NULL), ED247_STATUS_SUCCESS);

  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
  TEST_SYNC("unicast stream sent");
  TEST_SYNC("unicast validated");

  ed247_stream_t stream_out_mc;
  ASSERT_EQ(ed247_find_streams(_context, "StreamMCOut", &stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(stream_list, &stream_out_mc), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_free(stream_list), ED247_STATUS_SUCCESS);

  memset(output_payload, 0x84, 10);
  ASSERT_EQ(ed247_stream_push_sample(stream_out_mc, output_payload, 10, NULL, NULL), ED247_STATUS_SUCCESS);

  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
  TEST_SYNC("multicast stream sent");

}


std::vector<TestParams> stream_files;
INSTANTIATE_TEST_CASE_P(multiAddress, multiAddressFixture,
                        ::testing::ValuesIn(stream_files));

int main(int argc, char **argv)
{
  config_path = (argc >=1)? argv[1] : "../config";

  tests_tools::display_ed247_lib_infos();
  SAY("Configuration path: " << config_path);

  stream_files.push_back({TEST_ACTOR_ID, config_path + "/ecic_func_multi_address_sender.xml"});

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
