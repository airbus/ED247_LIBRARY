#define TEST_ACTOR_ID TEST_ACTOR1_ID
#include "two_actors_test.h"

class exchangeTTSFixture : public TestContext {};


ed247_timestamp_t timestamp1 = {123, 456};
ed247_timestamp_t timestamp2 = {456, 789};
void get_time_test1(ed247_timestamp_t* timestamp) {
  *timestamp = timestamp1;
}
void get_time_test2(ed247_timestamp_t* timestamp) {
  *timestamp = timestamp2;
}


TEST_P(exchangeTTSFixture, exchangeTTSMain)
{
  ed247_stream_list_t stream_list;

  ed247_stream_t streamNoHeader;
  ASSERT_EQ(ed247_find_streams(_context, "StreamNoHeader", &stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(stream_list, &streamNoHeader), ED247_STATUS_SUCCESS);
  ed247_stream_list_free(stream_list);

  ed247_stream_t streamNoTTS;
  ASSERT_EQ(ed247_find_streams(_context, "StreamNoTTS", &stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(stream_list, &streamNoTTS), ED247_STATUS_SUCCESS);
  ed247_stream_list_free(stream_list);

  ed247_stream_t streamWithTTS;
  ASSERT_EQ(ed247_find_streams(_context, "StreamWithTTS", &stream_list), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(stream_list, &streamWithTTS), ED247_STATUS_SUCCESS);
  ed247_stream_list_free(stream_list);

  // Dummy payload
  char payload[500];
  memset(payload, 0x01, 100);


  // Set TTS callback to 1 then send data
  ed247_set_transport_timestamp_callback(&get_time_test1);

  ASSERT_EQ(ed247_stream_push_sample(streamNoHeader, payload, 100, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_push_sample(streamNoTTS, payload, 100, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_push_sample(streamWithTTS, payload, 100, NULL, NULL), ED247_STATUS_SUCCESS);

  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
  TEST_SYNC("TTS 1 sent");
  TEST_SYNC("TTS 1 received");

  // Set TTS callback to 2 then send data
  ed247_set_transport_timestamp_callback(&get_time_test2);

  ASSERT_EQ(ed247_stream_push_sample(streamNoHeader, payload, 100, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_push_sample(streamNoTTS, payload, 100, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_push_sample(streamWithTTS, payload, 100, NULL, NULL), ED247_STATUS_SUCCESS);

  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
  TEST_SYNC("TTS 2 sent");
  TEST_SYNC("TTS 2 received");

}


std::vector<TestParams> stream_files;
INSTANTIATE_TEST_CASE_P(exchangeTTS, exchangeTTSFixture,
                        ::testing::ValuesIn(stream_files));

int main(int argc, char **argv)
{
  std::string config_path = (argc >=1)? argv[1] : "../config";

  tests_tools::display_ed247_lib_infos();
  SAY("Configuration path: " << config_path);

  stream_files.push_back({TEST_ACTOR_ID, config_path + "/ecic_func_exchange_tts_sender.xml"});

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
