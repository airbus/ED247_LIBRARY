#define TEST_ACTOR_ID TEST_ACTOR2_ID
#include "functional_test.h"

static std::string config_path;

class StreamAssistantFixture : public TestContext {};

#define EXPECT_PAYLOAD(payload, paySize, expected, expectSize)                 \
  EXPECT_EQ(paySize, expectSize); \
  EXPECT_TRUE(memcmp(payload, &expected, std::min((uint32_t)(paySize), (uint32_t)(expectSize))) == 0) << \
           "  Actual: " << hex_stream(payload, paySize) << std::endl << "Expected: " << hex_stream(&expected, expectSize)


//
// Check NAD arrays
//
TEST_P(StreamAssistantFixture, nad_arrays)
{
  ed247_context_t context(nullptr);
  const void* payload = NULL;
  uint32_t size;

  ASSERT_EQ(ed247_load_file((config_path + "/ecic_func_exchange_streamassistant_receiver.xml").c_str(), &context), ED247_STATUS_SUCCESS);

  ed247_stream_t input_stream;
  ed247_stream_assistant_t assistant;
  ASSERT_EQ(ed247_get_stream(context, "NADStream", &input_stream), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_assistant(input_stream, &assistant), ED247_STATUS_SUCCESS);

  ed247_signal_t NAD_INT8; ASSERT_EQ(ed247_get_signal(context, "NAD_INT8", &NAD_INT8), ED247_STATUS_SUCCESS);
  ed247_signal_t NAD_INT16; ASSERT_EQ(ed247_get_signal(context, "NAD_INT16", &NAD_INT16), ED247_STATUS_SUCCESS);
  ed247_signal_t NAD_INT32; ASSERT_EQ(ed247_get_signal(context, "NAD_INT32", &NAD_INT32), ED247_STATUS_SUCCESS);
  ed247_signal_t NAD_INT64; ASSERT_EQ(ed247_get_signal(context, "NAD_INT64", &NAD_INT64), ED247_STATUS_SUCCESS);
  ed247_signal_t NAD_UINT8; ASSERT_EQ(ed247_get_signal(context, "NAD_UINT8", &NAD_UINT8), ED247_STATUS_SUCCESS);
  ed247_signal_t NAD_UINT16; ASSERT_EQ(ed247_get_signal(context, "NAD_UINT16", &NAD_UINT16), ED247_STATUS_SUCCESS);
  ed247_signal_t NAD_UINT32; ASSERT_EQ(ed247_get_signal(context, "NAD_UINT32", &NAD_UINT32), ED247_STATUS_SUCCESS);
  ed247_signal_t NAD_UINT64; ASSERT_EQ(ed247_get_signal(context, "NAD_UINT64", &NAD_UINT64), ED247_STATUS_SUCCESS);
  ed247_signal_t NAD_FLOAT32; ASSERT_EQ(ed247_get_signal(context, "NAD_FLOAT32", &NAD_FLOAT32), ED247_STATUS_SUCCESS);
  ed247_signal_t NAD_FLOAT64; ASSERT_EQ(ed247_get_signal(context, "NAD_FLOAT64", &NAD_FLOAT64), ED247_STATUS_SUCCESS);

  TEST_SYNC("Receiver ready");
  TEST_SYNC("NAD Data sent");
  ASSERT_EQ(ed247_wait_frame(context, nullptr, ED247_ONE_SECOND), ED247_STATUS_SUCCESS);

  ASSERT_EQ(ed247_stream_assistant_pop_sample(assistant, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);

  {
    char expected[2][2] = { -10, 20, -30, 40 };
    ed247_stream_assistant_read_signal(assistant, NAD_INT8, &payload, &size);
    EXPECT_PAYLOAD(payload, size, expected, 4*sizeof(char));
  }
  {
    int16_t expected[8] = { -10, 20, -30, 40, -50, 60, -70, 80 };
    ed247_stream_assistant_read_signal(assistant, NAD_INT16, &payload, &size);
    EXPECT_PAYLOAD(payload, size, expected, 8*sizeof(int16_t));
  }
  {
    int32_t expected[4] = { -1111, 2222, -3333, 4444 };
    ed247_stream_assistant_read_signal(assistant, NAD_INT32, &payload, &size);
    EXPECT_PAYLOAD(payload, size, expected, 4*sizeof(int32_t));
  }
  {
    int64_t expected[3] = { 1010, -2020, 3030 };
    ed247_stream_assistant_read_signal(assistant, NAD_INT64, &payload, &size);
    EXPECT_PAYLOAD(payload, size, expected, 3*sizeof(int64_t));
  }
  {
    uint8_t expected = 129;
    ed247_stream_assistant_read_signal(assistant, NAD_UINT8, &payload, &size);
    EXPECT_PAYLOAD(payload, size, expected, 1*sizeof(uint8_t));
  }
  {
    uint16_t expected[2][3] = { 111, 222, 333, 444, 555, 666 };
    ed247_stream_assistant_read_signal(assistant, NAD_UINT16, &payload, &size);
    EXPECT_PAYLOAD(payload, size, expected, 6*sizeof(uint16_t));
  }
  {
    uint32_t expected[7] = { 1111, 2222, 3333, 4444, 5555, 6666, 7777 };
    ed247_stream_assistant_read_signal(assistant, NAD_UINT32, &payload, &size);
    EXPECT_PAYLOAD(payload, size, expected, 7*sizeof(uint32_t));
  }
  {
    uint64_t expected[4] = { 11111, 22222, 33333, 44444 };
    ed247_stream_assistant_read_signal(assistant, NAD_UINT64, &payload, &size);
    EXPECT_PAYLOAD(payload, size, expected, 4*sizeof(uint64_t));
  }
  {
    float expected[5] = { 11.0, 22.0, 33.0, 44.0, 55.0 };
    ed247_stream_assistant_read_signal(assistant, NAD_FLOAT32, &payload, &size);
    EXPECT_PAYLOAD(payload, size, expected, 5*sizeof(float));
  }
  {
    double expected[3] = { -111.0, 222.0, -333.0 };
    ed247_stream_assistant_read_signal(assistant, NAD_FLOAT64, &payload, &size);
    EXPECT_PAYLOAD(payload, size, expected, 3*sizeof(double));
  }

  ASSERT_EQ(ed247_stream_assistant_pop_sample(assistant, NULL, NULL, NULL, NULL), ED247_STATUS_NODATA);

  ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
  TEST_SYNC("Receiver done");
}


//
// Check VNAD arrays
//
TEST_P(StreamAssistantFixture, vnad_arrays)
{
  ed247_context_t context(nullptr);
  const void* payload = NULL;
  uint32_t size;

  ASSERT_EQ(ed247_load_file((config_path + "/ecic_func_exchange_streamassistant_receiver.xml").c_str(), &context), ED247_STATUS_SUCCESS);

  ed247_stream_t input_stream;
  ed247_stream_assistant_t assistant;
  ASSERT_EQ(ed247_get_stream(context, "VNADStream", &input_stream), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_assistant(input_stream, &assistant), ED247_STATUS_SUCCESS);

  ed247_signal_t VNAD_INT8; ASSERT_EQ(ed247_get_signal(context, "VNAD_INT8", &VNAD_INT8), ED247_STATUS_SUCCESS);
  ed247_signal_t VNAD_INT16; ASSERT_EQ(ed247_get_signal(context, "VNAD_INT16", &VNAD_INT16), ED247_STATUS_SUCCESS);
  ed247_signal_t VNAD_INT32; ASSERT_EQ(ed247_get_signal(context, "VNAD_INT32", &VNAD_INT32), ED247_STATUS_SUCCESS);
  ed247_signal_t VNAD_INT64; ASSERT_EQ(ed247_get_signal(context, "VNAD_INT64", &VNAD_INT64), ED247_STATUS_SUCCESS);
  ed247_signal_t VNAD_UINT8; ASSERT_EQ(ed247_get_signal(context, "VNAD_UINT8", &VNAD_UINT8), ED247_STATUS_SUCCESS);
  ed247_signal_t VNAD_UINT16; ASSERT_EQ(ed247_get_signal(context, "VNAD_UINT16", &VNAD_UINT16), ED247_STATUS_SUCCESS);
  ed247_signal_t VNAD_UINT32; ASSERT_EQ(ed247_get_signal(context, "VNAD_UINT32", &VNAD_UINT32), ED247_STATUS_SUCCESS);
  ed247_signal_t VNAD_UINT64; ASSERT_EQ(ed247_get_signal(context, "VNAD_UINT64", &VNAD_UINT64), ED247_STATUS_SUCCESS);
  ed247_signal_t VNAD_FLOAT32; ASSERT_EQ(ed247_get_signal(context, "VNAD_FLOAT32", &VNAD_FLOAT32), ED247_STATUS_SUCCESS);
  ed247_signal_t VNAD_FLOAT64; ASSERT_EQ(ed247_get_signal(context, "VNAD_FLOAT64", &VNAD_FLOAT64), ED247_STATUS_SUCCESS);

  TEST_SYNC("Receiver ready");
  TEST_SYNC("VNAD Data sent");
  ASSERT_EQ(ed247_wait_frame(context, nullptr, ED247_ONE_SECOND), ED247_STATUS_SUCCESS);

  ASSERT_EQ(ed247_stream_assistant_pop_sample(assistant, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS);

  {
    char expected[2][2] = { -10, 20, -30, 40 };
    ed247_stream_assistant_read_signal(assistant, VNAD_INT8, &payload, &size);
    EXPECT_PAYLOAD(payload, size, expected, 4*sizeof(char));
  }
  {
    int16_t expected[8] = { -10, 20, -30, 40, -50, 60, -70, 80 };
    ed247_stream_assistant_read_signal(assistant, VNAD_INT16, &payload, &size);
    EXPECT_PAYLOAD(payload, size, expected, 8*sizeof(int16_t));
  }
  {
    int32_t expected[4] = { -1111, 2222, -3333, 4444 };
    ed247_stream_assistant_read_signal(assistant, VNAD_INT32, &payload, &size);
    EXPECT_PAYLOAD(payload, size, expected, 4*sizeof(int32_t));
  }
  {
    int64_t expected[3] = { 1010, -2020, 3030 };
    ed247_stream_assistant_read_signal(assistant, VNAD_INT64, &payload, &size);
    EXPECT_PAYLOAD(payload, size, expected, 3*sizeof(int64_t));
  }
  {
    uint8_t expected = 129;
    ed247_stream_assistant_read_signal(assistant, VNAD_UINT8, &payload, &size);
    EXPECT_PAYLOAD(payload, size, expected, 1*sizeof(uint8_t));
  }
  {
    uint16_t expected[2][3] = { 111, 222, 333, 444, 555, 666 };
    ed247_stream_assistant_read_signal(assistant, VNAD_UINT16, &payload, &size);
    EXPECT_PAYLOAD(payload, size, expected, 6*sizeof(uint16_t));
  }
  {
    uint32_t expected[7] = { 1111, 2222, 3333, 4444, 5555, 6666, 7777 };
    ed247_stream_assistant_read_signal(assistant, VNAD_UINT32, &payload, &size);
    EXPECT_PAYLOAD(payload, size, expected, 7*sizeof(uint32_t));
  }
  {
    uint64_t expected[4] = { 11111, 22222, 33333, 44444 };
    ed247_stream_assistant_read_signal(assistant, VNAD_UINT64, &payload, &size);
    EXPECT_PAYLOAD(payload, size, expected, 4*sizeof(uint64_t));
  }
  {
    float expected[5] = { 11.0, 22.0, 33.0, 44.0, 55.0 };
    ed247_stream_assistant_read_signal(assistant, VNAD_FLOAT32, &payload, &size);
    EXPECT_PAYLOAD(payload, size, expected, 5*sizeof(float));
  }
  {
    double expected[3] = { -111.0, 222.0, -333.0 };
    ed247_stream_assistant_read_signal(assistant, VNAD_FLOAT64, &payload, &size);
    EXPECT_PAYLOAD(payload, size, expected, 3*sizeof(double));
  }

  ASSERT_EQ(ed247_stream_assistant_pop_sample(assistant, NULL, NULL, NULL, NULL), ED247_STATUS_NODATA);

  ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
  TEST_SYNC("Receiver done");
}


std::vector<TestParams> stream_files;
INSTANTIATE_TEST_CASE_P(bidirectional, StreamAssistantFixture, ::testing::ValuesIn(stream_files));

int main(int argc, char **argv)
{
  config_path = (argc >=1)? argv[1] : "../config";

  tests_tools::display_ed247_lib_infos();
  SAY("Configuration path: " << config_path);

  stream_files.push_back({TEST_ACTOR_ID, std::string()});

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
