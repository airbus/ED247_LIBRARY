#define TEST_ACTOR_ID TEST_ACTOR2_ID
#include "functional_test.h"

class exchangeTTSFixture : public TestContext {};

ed247_timestamp_t timestampEmpty = {0, 0};
ed247_timestamp_t timestamp1 = {123, 456};
ed247_timestamp_t timestamp2 = {456, 789};

bool timestamp_eq(const ed247_timestamp_t& a, const ed247_timestamp_t& b) {
  return (a.epoch_s == b.epoch_s && a.offset_ns == b.offset_ns);
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


  const char* payload = nullptr;
  size_t payload_size;
  const ed247_sample_details_t* sample_details;

  // Receive TTS 1
  TEST_SYNC("Receive TTS 1");
  ASSERT_EQ(ed247_wait_during(_context, &stream_list, ED247_100_MILI), ED247_STATUS_SUCCESS);

  ASSERT_EQ(ed247_stream_pop_sample(streamNoHeader, (const void**)&payload, &payload_size, NULL, NULL, &sample_details, NULL), ED247_STATUS_SUCCESS);
  ASSERT_TRUE(timestamp_eq(sample_details->transport_timestamp, timestampEmpty));

  ASSERT_EQ(ed247_stream_pop_sample(streamNoTTS, (const void**)&payload, &payload_size, NULL, NULL, &sample_details, NULL), ED247_STATUS_SUCCESS);
  ASSERT_TRUE(timestamp_eq(sample_details->transport_timestamp, timestampEmpty));

  ASSERT_EQ(ed247_stream_pop_sample(streamWithTTS, (const void**)&payload, &payload_size, NULL, NULL, &sample_details, NULL), ED247_STATUS_SUCCESS);
  ASSERT_TRUE(timestamp_eq(sample_details->transport_timestamp, timestamp1));

  TEST_SYNC("TTS 1 Received");

  // Receive TTS 2
  TEST_SYNC("Receive TTS 2");
  ASSERT_EQ(ed247_wait_during(_context, &stream_list, ED247_100_MILI), ED247_STATUS_SUCCESS);

  ASSERT_EQ(ed247_stream_pop_sample(streamNoHeader, (const void**)&payload, &payload_size, NULL, NULL, &sample_details, NULL), ED247_STATUS_SUCCESS);
  ASSERT_TRUE(timestamp_eq(sample_details->transport_timestamp, timestampEmpty));

  ASSERT_EQ(ed247_stream_pop_sample(streamNoTTS, (const void**)&payload, &payload_size, NULL, NULL, &sample_details, NULL), ED247_STATUS_SUCCESS);
  ASSERT_TRUE(timestamp_eq(sample_details->transport_timestamp, timestampEmpty));

  ASSERT_EQ(ed247_stream_pop_sample(streamWithTTS, (const void**)&payload, &payload_size, NULL, NULL, &sample_details, NULL), ED247_STATUS_SUCCESS);
  ASSERT_TRUE(timestamp_eq(sample_details->transport_timestamp, timestamp2));

  TEST_SYNC("TTS 2 Received");

}


std::vector<TestParams> stream_files;
INSTANTIATE_TEST_CASE_P(exchangeTTS, exchangeTTSFixture,
                        ::testing::ValuesIn(stream_files));

int main(int argc, char **argv)
{
  std::string config_path = (argc >=1)? argv[1] : "../config";

  tests_tools::display_ed247_lib_infos();
  SAY("Configuration path: " << config_path);

  stream_files.push_back({TEST_ACTOR_ID, config_path + "/ecic_func_exchange_tts_receiver.xml"});

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
