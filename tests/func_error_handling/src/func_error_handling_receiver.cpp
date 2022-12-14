#define TEST_ACTOR1_NAME "sender"
#define TEST_ACTOR2_NAME "receiver"
#define TEST_ACTOR_ID TEST_ACTOR2_ID
#include "functional_test.h"

//
// In this test, sender and receiver ECIC are not symmetrical:
// * The sender ECIC define 'raw' AFDX stream without any ED247 field activated.
//   So the sender is able to "forge" any ED247 stream it want
// * The receiver ECIC define the real expected ED247 streams
//
class errorHandlingFixture : public TestContext {};


// ============================================================================
// Invalid header test
// ============================================================================
TEST_P(errorHandlingFixture, invalidHeader)
{
  ed247_stream_list_t streams;
  ed247_stream_t afdx_stream;
  ASSERT_EQ(ed247_find_streams(_context, "HeaderTestStream", &streams), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(streams, &afdx_stream), ED247_STATUS_SUCCESS);
  ASSERT_NE(afdx_stream, nullptr) << "Stream has not been found in ECIC!";
  ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);

  // The sender shall have send 5 stream: 1 valid, 3 invalid and a last valid one.
  // We should receive the first and the last one: the 3 invalid ones shall have been dropped by wait_during(). See logs.
  TEST_WAIT("sent #1");
  ASSERT_EQ(ed247_wait_during(_context, &streams, ED247_100_MILI), ED247_STATUS_SUCCESS);
  ASSERT_POP_EQ(afdx_stream, 10, 0x42);
  TEST_SIGNAL("poped #1");

  TEST_WAIT("sent #2");
  ASSERT_EQ(ed247_wait_during(_context, &streams, ED247_100_MILI), ED247_STATUS_SUCCESS);
  ASSERT_POP_NODATA(afdx_stream);
  TEST_SIGNAL("poped #2");

  TEST_WAIT("sent #3");
  ASSERT_EQ(ed247_wait_during(_context, &streams, ED247_100_MILI), ED247_STATUS_SUCCESS);
  ASSERT_POP_EQ(afdx_stream, 20, 0x46);
  ASSERT_POP_NODATA(afdx_stream);
  TEST_SIGNAL("poped #3");
}

// ============================================================================
// Invalid multichannel test
// ============================================================================
TEST_P(errorHandlingFixture, invalidMultichannel)
{
  ed247_stream_list_t streams;
  ed247_stream_t afdx_stream;
  ASSERT_EQ(ed247_find_streams(_context, "MultichannelTestStream", &streams), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(streams, &afdx_stream), ED247_STATUS_SUCCESS);
  ASSERT_NE(afdx_stream, nullptr) << "Stream has not been found in ECIC!";
  ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);

  // The sender shall have send 5 stream: 1 valid, 3 invalid and a last valid one.
  // We should receive the first and the last one: the 3 invalid ones shall have been dropped by wait_during(). See logs.
  TEST_WAIT("sent #1");
  ASSERT_EQ(ed247_wait_during(_context, &streams, ED247_100_MILI), ED247_STATUS_SUCCESS);
  ASSERT_POP_EQ(afdx_stream, 10, 0x42);
  TEST_SIGNAL("poped #1");

  TEST_WAIT("sent #2");
  ASSERT_EQ(ed247_wait_during(_context, &streams, ED247_100_MILI), ED247_STATUS_SUCCESS);
  ASSERT_POP_NODATA(afdx_stream);
  TEST_SIGNAL("poped #2");

  TEST_WAIT("sent #3");
  ASSERT_EQ(ed247_wait_during(_context, &streams, ED247_100_MILI), ED247_STATUS_SUCCESS);
  ASSERT_POP_EQ(afdx_stream, 15, 0x49);
  ASSERT_POP_NODATA(afdx_stream);
  TEST_SIGNAL("poped #3");
}

// ============================================================================
// Invalid DataTimeStamp test
// ============================================================================
TEST_P(errorHandlingFixture, invalidDataTimeStamp)
{
  ed247_stream_list_t streams;
  ed247_stream_t afdx_stream;
  ASSERT_EQ(ed247_find_streams(_context, "DataTSTestStream", &streams), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(streams, &afdx_stream), ED247_STATUS_SUCCESS);
  ASSERT_NE(afdx_stream, nullptr) << "Stream has not been found in ECIC!";
  ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);

  // The sender shall have send 3 stream: 1 valid, 1 invalid and a last valid one.
  // We should receive the first and the last one: the invalid one shall have been dropped by wait_during(). See logs.
  TEST_WAIT("sent #1");
  ASSERT_EQ(ed247_wait_during(_context, &streams, ED247_100_MILI), ED247_STATUS_SUCCESS);
  ASSERT_POP_EQ(afdx_stream, 10, 0x42);
  TEST_SIGNAL("poped #1");

  TEST_WAIT("sent #2");
  ASSERT_EQ(ed247_wait_during(_context, &streams, ED247_100_MILI), ED247_STATUS_SUCCESS);
  ASSERT_POP_NODATA(afdx_stream);
  TEST_SIGNAL("poped #2");

  TEST_WAIT("sent #3");
  ASSERT_EQ(ed247_wait_during(_context, &streams, ED247_100_MILI), ED247_STATUS_SUCCESS);
  ASSERT_POP_EQ(afdx_stream, 15, 0x45);
  ASSERT_POP_NODATA(afdx_stream);
  TEST_SIGNAL("poped #3");
}

// ============================================================================
// Invalid Stream test
// ============================================================================
TEST_P(errorHandlingFixture, invalidStream)
{
  ed247_stream_list_t streams;
  ed247_stream_t a429_stream;
  ASSERT_EQ(ed247_find_streams(_context, "A429TestStream", &streams), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(streams, &a429_stream), ED247_STATUS_SUCCESS);
  ASSERT_NE(a429_stream, nullptr) << "Stream has not been found in ECIC!";
  ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);

  ed247_stream_t afdx_stream;
  ASSERT_EQ(ed247_find_streams(_context, "AFDXTestStream", &streams), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_list_next(streams, &afdx_stream), ED247_STATUS_SUCCESS);
  ASSERT_NE(afdx_stream, nullptr) << "Stream has not been found in ECIC!";
  ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);

  // The sender shall have send 3 stream: 1 valid, 1 invalid and a last valid one.
  // We should receive the first and the last one: the invalid one shall have been dropped by wait_during(). See logs.
  TEST_WAIT("sent #1");
  ASSERT_EQ(ed247_wait_during(_context, &streams, ED247_100_MILI), ED247_STATUS_SUCCESS);
  ASSERT_POP_EQ(a429_stream, 4, 0x42);
  TEST_SIGNAL("poped #1");

  TEST_WAIT("sent #2");
  ASSERT_EQ(ed247_wait_during(_context, &streams, ED247_100_MILI), ED247_STATUS_SUCCESS);
  ASSERT_POP_NODATA(a429_stream);
  TEST_SIGNAL("poped #2");

  TEST_WAIT("sent #3");
  ASSERT_EQ(ed247_wait_during(_context, &streams, ED247_100_MILI), ED247_STATUS_SUCCESS);
  ASSERT_POP_EQ(a429_stream, 4, 0x44);
  ASSERT_POP_NODATA(a429_stream);
  TEST_SIGNAL("poped #3");

  // The sender shall have send several streams: 1 valid, some invalids and a last valid one.
  // We should receive the first and the last one: the invalid ones shall have been dropped by wait_during(). See logs.
  TEST_WAIT("sent #4");
  ASSERT_EQ(ed247_wait_during(_context, &streams, ED247_100_MILI), ED247_STATUS_SUCCESS);
  ASSERT_POP_EQ(afdx_stream, 10, 0x42);
  TEST_SIGNAL("poped #4");

  TEST_WAIT("sent #5");
  ASSERT_EQ(ed247_wait_during(_context, &streams, ED247_100_MILI), ED247_STATUS_SUCCESS);
  ASSERT_POP_NODATA(afdx_stream);
  TEST_SIGNAL("poped #5");

  TEST_WAIT("sent #6");
  ASSERT_EQ(ed247_wait_during(_context, &streams, ED247_100_MILI), ED247_STATUS_SUCCESS);
  ASSERT_POP_EQ(afdx_stream, 15, 0x49);
  TEST_SIGNAL("poped #6");
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

  stream_files.push_back({TEST_ACTOR_ID, config_path + "/ecic_func_error_handling_receiver.xml"});

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
