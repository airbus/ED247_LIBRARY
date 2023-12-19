#define TEST_ACTOR1_NAME "sender"
#define TEST_ACTOR2_NAME "receiver"
#define TEST_ACTOR_ID TEST_ACTOR1_ID
#include "two_actors_test.h"

//
// In this test, sender and receiver ECIC are not symmetrical:
// * The sender ECIC define 'raw' AFDX stream without any ED247 field activated.
//   So the sender is able to "forge" any ED247 stream it want
// * The receiver ECIC define the real expected ED247 streams
//
class errorHandlingFixture : public TestContext {};

#define DISABLED 242

// Write a dummy ED247 header and return its size
uint32_t write_ED247_header(char* payload, bool set_component_id, bool set_sequence_number, bool set_TTS)
{
  uint32_t payload_size = 0;
  if (set_component_id) {
    uint16_t net_component_id = htons(1);
    memcpy(payload + payload_size, &net_component_id, sizeof(uint16_t));
    payload_size += sizeof(uint16_t);
  }
  if (set_sequence_number) {
    uint16_t net_sequence_number = htons(0);
    memcpy(payload + payload_size, &net_sequence_number, sizeof(uint16_t));
    payload_size += sizeof(uint16_t);
  }
  if (set_TTS) {
    uint64_t net_TTS = 0;
    memcpy(payload + payload_size, &net_TTS, sizeof(uint64_t));
    payload_size += sizeof(uint64_t);
  }
  return payload_size;
}

// Write a multichannel stream header and return its size
uint32_t write_multichannel_stream_header(char* payload, uint16_t stream_uid, uint16_t data_size)
{
  uint32_t payload_size = 0;
  if (stream_uid != DISABLED) {
    uint16_t net_stream_uid = htons(stream_uid);
    memcpy(payload + payload_size, &net_stream_uid, sizeof(uint16_t));
    payload_size += sizeof(uint16_t);
  }

  if (data_size != DISABLED) {
    uint16_t net_data_size = htons(data_size);
    memcpy(payload + payload_size, &net_data_size, sizeof(uint16_t));
    payload_size += sizeof(uint16_t);
  }
  return payload_size;
}

// Write a stream DTS header and return its size
uint32_t write_DTS_stream_header(char* payload, bool set_DTS)
{
  uint32_t payload_size = 0;
  if (set_DTS) {
    uint64_t net_DTS = htons(0);
    memcpy(payload + payload_size, &net_DTS, sizeof(uint64_t));
    payload_size += sizeof(uint64_t);
  }
  return payload_size;
}

// Write a dummy AFDX stream and return its size
uint32_t write_AFDX_stream(char* payload, uint16_t message_size, uint8_t message_data, uint16_t header_message_size)
{
  uint32_t payload_size = 0;
  if (header_message_size != DISABLED) {
    uint16_t net_message_size = htons(header_message_size);
    memcpy(payload + payload_size, &net_message_size, sizeof(uint16_t));
    payload_size += sizeof(uint16_t);
  }
  memset(payload + payload_size, message_data, message_size);
  return payload_size + message_size;
}

// Write a dummy A429 stream and return its size
uint32_t write_A429_stream(char* payload, uint16_t message_size, uint8_t message_data)
{
  memset(payload, message_data, message_size);
  return message_size;
}


// ============================================================================
// Invalid header test
// ============================================================================
TEST_P(errorHandlingFixture, invalidHeader)
{
  ed247_stream_list_t streams;
  ed247_stream_t forge_stream;
  ASSERT_EQ(ed247_find_streams(_context, "HeaderTestStream", &streams), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(streams, &forge_stream), ED247_STATUS_SUCCESS);
  ASSERT_NE(forge_stream, nullptr) << "Stream has not been found in ECIC!";
  ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);

  char payload[500];
  uint32_t payload_size = 0;

  // Forge a valid stream
  payload_size = 0;
  payload_size += write_ED247_header(payload + payload_size, true, true, true);
  payload_size += write_AFDX_stream (payload + payload_size, 10, 0x42, DISABLED);
  ASSERT_EQ(ed247_stream_push_sample(forge_stream, payload, payload_size, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

  TEST_SIGNAL("sent #1");
  TEST_WAIT("sent #1");

  // Forge a stream without component Id
  payload_size = 0;
  payload_size += write_ED247_header(payload + payload_size, false, false, false);
  payload_size += write_AFDX_stream (payload + payload_size, 1, 0x43, DISABLED);
  ASSERT_EQ(ed247_stream_push_sample(forge_stream, payload, payload_size, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

  // Forge a stream without SN
  payload_size = 0;
  payload_size += write_ED247_header(payload + payload_size, true, false, false);
  payload_size += write_AFDX_stream (payload + payload_size, 1, 0x44, DISABLED);
  ASSERT_EQ(ed247_stream_push_sample(forge_stream, payload, payload_size, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

  // Forge a stream without timestamp
  payload_size = 0;
  payload_size += write_ED247_header(payload + payload_size, true, true, false);
  payload_size += write_AFDX_stream (payload + payload_size, 1, 0x45, DISABLED);
  ASSERT_EQ(ed247_stream_push_sample(forge_stream, payload, payload_size, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

  TEST_SIGNAL("sent #2");
  TEST_WAIT("sent #2");

  // Forge another valid stream
  payload_size = 0;
  payload_size += write_ED247_header(payload + payload_size, true, true, true);
  payload_size += write_AFDX_stream (payload + payload_size, 20, 0x46, DISABLED);
  ASSERT_EQ(ed247_stream_push_sample(forge_stream, payload, payload_size, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

  TEST_SIGNAL("sent #3");
  TEST_WAIT("sent #3");
}

// ============================================================================
// Invalid multichannel test
// ============================================================================
TEST_P(errorHandlingFixture, invalidMultichannel)
{
  ed247_stream_list_t streams;
  ed247_stream_t forge_stream;
  ASSERT_EQ(ed247_find_streams(_context, "MultichannelTestStream", &streams), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(streams, &forge_stream), ED247_STATUS_SUCCESS);
  ASSERT_NE(forge_stream, nullptr) << "Stream has not been found in ECIC!";
  ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);

  char payload[500];
  uint32_t payload_size = 0;
  uint16_t data_size = 0;

  // Forge a valid stream
  payload_size = 0;
  data_size = 10;
  payload_size += write_multichannel_stream_header(payload + payload_size, 5, data_size);
  payload_size += write_AFDX_stream               (payload + payload_size, data_size, 0x42, DISABLED);
  ASSERT_EQ(ed247_stream_push_sample(forge_stream, payload, payload_size, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

  TEST_SIGNAL("sent #1");
  TEST_WAIT("sent #1");

  // Forge a stream without UID
  payload_size = 0;
  data_size = 1;
  payload_size += write_multichannel_stream_header(payload + payload_size, DISABLED, DISABLED);
  payload_size += write_AFDX_stream               (payload + payload_size, data_size, 0x43, DISABLED);
  ASSERT_EQ(ed247_stream_push_sample(forge_stream, payload, payload_size, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

  // Forge a stream without data size
  payload_size = 0;
  data_size = 1;
  payload_size += write_multichannel_stream_header(payload + payload_size, 5, DISABLED);
  payload_size += write_AFDX_stream               (payload + payload_size, data_size, 0x44, DISABLED);
  ASSERT_EQ(ed247_stream_push_sample(forge_stream, payload, payload_size, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

  // Forge a stream with an invalid uid
  payload_size = 0;
  data_size = 15;
  payload_size += write_multichannel_stream_header(payload + payload_size, 15, DISABLED);
  payload_size += write_AFDX_stream               (payload + payload_size, data_size, 0x45, DISABLED);
  ASSERT_EQ(ed247_stream_push_sample(forge_stream, payload, payload_size, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

  TEST_SIGNAL("sent #2");
  TEST_WAIT("sent #2");

  // Forge another valid stream
  payload_size = 0;
  data_size = 15;
  payload_size += write_multichannel_stream_header(payload + payload_size, 5, data_size);
  payload_size += write_AFDX_stream               (payload + payload_size, data_size, 0x49, DISABLED);
  ASSERT_EQ(ed247_stream_push_sample(forge_stream, payload, payload_size, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

  TEST_SIGNAL("sent #3");
  TEST_WAIT("sent #3");
}

// ============================================================================
// Invalid DataTimeStamp test
// ============================================================================
TEST_P(errorHandlingFixture, invalidDataTimeStamp)
{
  ed247_stream_list_t streams;
  ed247_stream_t forge_stream;
  ASSERT_EQ(ed247_find_streams(_context, "DataTSTestStream", &streams), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(streams, &forge_stream), ED247_STATUS_SUCCESS);
  ASSERT_NE(forge_stream, nullptr) << "Stream has not been found in ECIC!";
  ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);

  char payload[500];
  uint32_t payload_size = 0;
  uint16_t data_size = 0;

  // Forge a valid stream with DTS
  payload_size = 0;
  data_size = 10;
  payload_size += write_DTS_stream_header(payload, true);
  payload_size += write_AFDX_stream (payload + payload_size, data_size, 0x42, DISABLED);
  ASSERT_EQ(ed247_stream_push_sample(forge_stream, payload, payload_size, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

  TEST_SIGNAL("sent #1");
  TEST_WAIT("sent #1");

  // Forge a stream without DTS
  payload_size = 0;
  data_size = 1;
  payload_size += write_DTS_stream_header(payload, false);
  payload_size += write_AFDX_stream (payload + payload_size, data_size, 0x43, DISABLED);
  ASSERT_EQ(ed247_stream_push_sample(forge_stream, payload, payload_size, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

  TEST_SIGNAL("sent #2");
  TEST_WAIT("sent #2");

  // Forge another valid stream with DTS
  payload_size = 0;
  data_size = 15;
  payload_size += write_DTS_stream_header(payload, true);
  payload_size += write_AFDX_stream (payload + payload_size, data_size, 0x45, DISABLED);
  ASSERT_EQ(ed247_stream_push_sample(forge_stream, payload, payload_size, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

  TEST_SIGNAL("sent #3");
  TEST_WAIT("sent #3");
}

// ============================================================================
// Invalid Stream test
// ============================================================================
TEST_P(errorHandlingFixture, invalidStream)
{
  ed247_stream_list_t streams;
  ed247_stream_t forge_stream_a429;
  ASSERT_EQ(ed247_find_streams(_context, "A429TestStream", &streams), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(streams, &forge_stream_a429), ED247_STATUS_SUCCESS);
  ASSERT_NE(forge_stream_a429, nullptr) << "Stream has not been found in ECIC!";
  ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);

  ed247_stream_t forge_stream_afdx;
  ASSERT_EQ(ed247_find_streams(_context, "AFDXTestStream", &streams), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(streams, &forge_stream_afdx), ED247_STATUS_SUCCESS);
  ASSERT_NE(forge_stream_afdx, nullptr) << "Stream has not been found in ECIC!";
  ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);

  char payload[500];
  uint32_t payload_size = 0;
  uint16_t data_size = 0;

  // Forge a valid A429 stream
  payload_size = 0;
  data_size = 4;
  payload_size += write_A429_stream(payload + payload_size, data_size, 0x42);
  ASSERT_EQ(ed247_stream_push_sample(forge_stream_a429, payload, payload_size, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

  TEST_SIGNAL("sent #1");
  TEST_WAIT("sent #1");

  // Forge a too small A429 stream
  payload_size = 0;
  data_size = 1;
  payload_size += write_A429_stream(payload + payload_size, data_size, 0x43);
  ASSERT_EQ(ed247_stream_push_sample(forge_stream_a429, payload, payload_size, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

  TEST_SIGNAL("sent #2");
  TEST_WAIT("sent #2");

  // Forge another valid A429 stream
  payload_size = 0;
  data_size = 4;
  payload_size += write_A429_stream(payload + payload_size, data_size, 0x44);
  ASSERT_EQ(ed247_stream_push_sample(forge_stream_a429, payload, payload_size, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

  TEST_SIGNAL("sent #3");
  TEST_WAIT("sent #3");

  // Forge a valid AFDX stream
  payload_size = 0;
  data_size = 10;
  payload_size += write_AFDX_stream(payload + payload_size, data_size, 0x42, data_size);
  ASSERT_EQ(ed247_stream_push_sample(forge_stream_afdx, payload, payload_size, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

  TEST_SIGNAL("sent #4");
  TEST_WAIT("sent #4");

  // Forge an AFDX without MessageSize
  payload_size = 0;
  data_size = 1;
  payload_size += write_AFDX_stream(payload + payload_size, data_size, 0x43, DISABLED);
  ASSERT_EQ(ed247_stream_push_sample(forge_stream_afdx, payload, payload_size, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

  // Forge an AFDX with an invalid MessageSize
  payload_size = 0;
  data_size = 10;
  payload_size += write_AFDX_stream(payload + payload_size, data_size, 0x44, data_size + 3);
  ASSERT_EQ(ed247_stream_push_sample(forge_stream_afdx, payload, payload_size, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

  // Forge a too big AFDX
  payload_size = 0;
  data_size = 100;
  payload_size += write_AFDX_stream(payload + payload_size, data_size, 0x44, data_size);
  ASSERT_EQ(ed247_stream_push_sample(forge_stream_afdx, payload, payload_size, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
  TEST_SIGNAL("sent #5");
  TEST_WAIT("sent #5");

  // Forge another valid AFDX stream
  payload_size = 0;
  data_size = 15;
  payload_size += write_AFDX_stream(payload + payload_size, data_size, 0x49, data_size);
  ASSERT_EQ(ed247_stream_push_sample(forge_stream_afdx, payload, payload_size, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);

  TEST_SIGNAL("sent #6");
  TEST_WAIT("sent #6");
}


// ============================================================================
// Main
// ============================================================================
std::vector<TestParams> stream_files;
INSTANTIATE_TEST_CASE_P(errorHandlingInstance, errorHandlingFixture,
                        ::testing::ValuesIn(stream_files));

int main(int argc, char **argv)
{
  std::string config_path = (argc >=1)? argv[1] : "../config";
  tests_tools::display_ed247_lib_infos();
  SAY("Configuration path: " << config_path);

  stream_files.push_back({TEST_ACTOR_ID, config_path + "/ecic_func_error_handling_sender.xml"});

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
