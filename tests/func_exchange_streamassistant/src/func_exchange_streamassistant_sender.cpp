#define TEST_ACTOR_ID TEST_ACTOR1_ID
#include "functional_test.h"

static std::string config_path;

class StreamAssistantFixture : public TestContext {};

//
// Check NAD arrays
//
TEST_P(StreamAssistantFixture, nad_arrays)
{
  ed247_context_t context(nullptr);
  ASSERT_EQ(ed247_load_file((config_path + "/ecic_func_exchange_streamassistant_sender.xml").c_str(), &context), ED247_STATUS_SUCCESS);

  ed247_stream_t output_stream;
  ed247_stream_assistant_t assistant;
  ASSERT_EQ(ed247_get_stream(context, "NADStream", &output_stream), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_assistant(output_stream, &assistant), ED247_STATUS_SUCCESS);

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

  {
    char voided_data[2][2] = { 0x01, 0x02, 0x03, 0x04 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, NAD_INT8, &voided_data, 2*2*sizeof(char)), ED247_STATUS_SUCCESS);
    char data[2][2] = { -10, 20, -30, 40 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, NAD_INT8, &data, 2*2*sizeof(char)), ED247_STATUS_SUCCESS);
  }
  {
    int16_t voided_data[8] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, NAD_INT16, &voided_data, 8*sizeof(int16_t)), ED247_STATUS_SUCCESS);
    int16_t data[8] = { -10, 20, -30, 40, -50, 60, -70, 80 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, NAD_INT16, &data, 8*sizeof(int16_t)), ED247_STATUS_SUCCESS);
  }
  {
    int32_t voided_data[4] = { 0x01, 0x02, 0x03, 0x04 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, NAD_INT32, &voided_data, 4*sizeof(int32_t)), ED247_STATUS_SUCCESS);
    int32_t data[4] = { -1111, 2222, -3333, 4444 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, NAD_INT32, &data, 4*sizeof(int32_t)), ED247_STATUS_SUCCESS);
  }
  {
    int64_t voided_data[3] = { 0x01, 0x02, 0x03 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, NAD_INT64, &voided_data, 3*sizeof(int64_t)), ED247_STATUS_SUCCESS);
    int64_t data[3] = { 1010, -2020, 3030 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, NAD_INT64, &data, 3*sizeof(int64_t)), ED247_STATUS_SUCCESS);
  }
  {
    uint8_t voided_data = 5;
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, NAD_UINT8, &voided_data, 1*sizeof(uint8_t)), ED247_STATUS_SUCCESS);
    uint8_t data = 129;
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, NAD_UINT8, &data, 1*sizeof(uint8_t)), ED247_STATUS_SUCCESS);
  }
  {
    uint16_t voided_data[2][3] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, NAD_UINT16, &voided_data, 2*3*sizeof(uint16_t)), ED247_STATUS_SUCCESS);
    uint16_t data[2][3] = { 111, 222, 333, 444, 555, 666 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, NAD_UINT16, &data, 2*3*sizeof(uint16_t)), ED247_STATUS_SUCCESS);
  }
  {
    uint32_t voided_data[7] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, NAD_UINT32, &voided_data, 7*sizeof(uint32_t)), ED247_STATUS_SUCCESS);
    uint32_t data[7] = { 1111, 2222, 3333, 4444, 5555, 6666, 7777 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, NAD_UINT32, &data, 7*sizeof(uint32_t)), ED247_STATUS_SUCCESS);
  }
  {
    uint64_t voided_data[4] = { 0x01, 0x02, 0x03, 0x04 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, NAD_UINT64, &voided_data, 4*sizeof(uint64_t)), ED247_STATUS_SUCCESS);
    uint64_t data[4] = { 11111, 22222, 33333, 44444 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, NAD_UINT64, &data, 4*sizeof(uint64_t)), ED247_STATUS_SUCCESS);
  }
  {
    float voided_data[5] = { 1.0, 2.0, 3.0, 4.0, 5.0 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, NAD_FLOAT32, &voided_data, 5*sizeof(float)), ED247_STATUS_SUCCESS);
    float data[5] = { 11.0, 22.0, 33.0, 44.0, 55.0 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, NAD_FLOAT32, &data, 5*sizeof(float)), ED247_STATUS_SUCCESS);
  }
  {
    double voided_data[3] = { -1.0, -2.0, -3.0 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, NAD_FLOAT64, &voided_data, 3*sizeof(double)), ED247_STATUS_SUCCESS);
    double data[3] = { -111.0, 222.0, -333.0 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, NAD_FLOAT64, &data, 3*sizeof(double)), ED247_STATUS_SUCCESS);
  }

  TEST_SYNC("Receiver ready");
  ASSERT_EQ(ed247_stream_assistant_push_sample(assistant, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(context), ED247_STATUS_SUCCESS);
  TEST_SYNC("NAD Data sent");

  ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
  TEST_SYNC("Receiver done");
}

//
// Check VNAD arrays
//
TEST_P(StreamAssistantFixture, vnad_arrays)
{
  ed247_context_t context(nullptr);
  ASSERT_EQ(ed247_load_file((config_path + "/ecic_func_exchange_streamassistant_sender.xml").c_str(), &context), ED247_STATUS_SUCCESS);

  ed247_stream_t output_stream;
  ed247_stream_assistant_t assistant;
  ASSERT_EQ(ed247_get_stream(context, "VNADStream", &output_stream), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_assistant(output_stream, &assistant), ED247_STATUS_SUCCESS);

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

  {
    char voided_data[2][2] = { 0x01, 0x02, 0x03, 0x04 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, VNAD_INT8, &voided_data, 2*2*sizeof(char)), ED247_STATUS_SUCCESS);
    char data[2][2] = { -10, 20, -30, 40 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, VNAD_INT8, &data, 2*2*sizeof(char)), ED247_STATUS_SUCCESS);
  }
  {
    int16_t voided_data[8] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, VNAD_INT16, &voided_data, 8*sizeof(int16_t)), ED247_STATUS_SUCCESS);
    int16_t data[8] = { -10, 20, -30, 40, -50, 60, -70, 80 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, VNAD_INT16, &data, 8*sizeof(int16_t)), ED247_STATUS_SUCCESS);
  }
  {
    int32_t voided_data[4] = { 0x01, 0x02, 0x03, 0x04 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, VNAD_INT32, &voided_data, 4*sizeof(int32_t)), ED247_STATUS_SUCCESS);
    int32_t data[4] = { -1111, 2222, -3333, 4444 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, VNAD_INT32, &data, 4*sizeof(int32_t)), ED247_STATUS_SUCCESS);
  }
  {
    int64_t voided_data[3] = { 0x01, 0x02, 0x03 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, VNAD_INT64, &voided_data, 3*sizeof(int64_t)), ED247_STATUS_SUCCESS);
    int64_t data[3] = { 1010, -2020, 3030 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, VNAD_INT64, &data, 3*sizeof(int64_t)), ED247_STATUS_SUCCESS);
  }
  {
    uint8_t voided_data = 5;
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, VNAD_UINT8, &voided_data, 1*sizeof(uint8_t)), ED247_STATUS_SUCCESS);
    uint8_t data = 129;
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, VNAD_UINT8, &data, 1*sizeof(uint8_t)), ED247_STATUS_SUCCESS);
  }
  {
    uint16_t voided_data[2][3] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, VNAD_UINT16, &voided_data, 2*3*sizeof(uint16_t)), ED247_STATUS_SUCCESS);
    uint16_t data[2][3] = { 111, 222, 333, 444, 555, 666 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, VNAD_UINT16, &data, 2*3*sizeof(uint16_t)), ED247_STATUS_SUCCESS);
  }
  {
    uint32_t voided_data[7] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, VNAD_UINT32, &voided_data, 7*sizeof(uint32_t)), ED247_STATUS_SUCCESS);
    uint32_t data[7] = { 1111, 2222, 3333, 4444, 5555, 6666, 7777 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, VNAD_UINT32, &data, 7*sizeof(uint32_t)), ED247_STATUS_SUCCESS);
  }
  {
    uint64_t voided_data[4] = { 0x01, 0x02, 0x03, 0x04 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, VNAD_UINT64, &voided_data, 4*sizeof(uint64_t)), ED247_STATUS_SUCCESS);
    uint64_t data[4] = { 11111, 22222, 33333, 44444 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, VNAD_UINT64, &data, 4*sizeof(uint64_t)), ED247_STATUS_SUCCESS);
  }
  {
    float voided_data[5] = { 1.0, 2.0, 3.0, 4.0, 5.0 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, VNAD_FLOAT32, &voided_data, 5*sizeof(float)), ED247_STATUS_SUCCESS);
    float data[5] = { 11.0, 22.0, 33.0, 44.0, 55.0 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, VNAD_FLOAT32, &data, 5*sizeof(float)), ED247_STATUS_SUCCESS);
  }
  {
    double voided_data[3] = { -1.0, -2.0, -3.0 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, VNAD_FLOAT64, &voided_data, 3*sizeof(double)), ED247_STATUS_SUCCESS);
    double data[3] = { -111.0, 222.0, -333.0 };
    ASSERT_EQ(ed247_stream_assistant_write_signal(assistant, VNAD_FLOAT64, &data, 3*sizeof(double)), ED247_STATUS_SUCCESS);
  }

  TEST_SYNC("Receiver ready");
  ASSERT_EQ(ed247_stream_assistant_push_sample(assistant, NULL, NULL), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(context), ED247_STATUS_SUCCESS);
  TEST_SYNC("VNAD Data sent");

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
