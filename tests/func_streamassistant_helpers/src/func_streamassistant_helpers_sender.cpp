#define TEST_ACTOR_ID TEST_ACTOR1_ID
#include "two_actors_test.h"

class assistantHelpersFixture : public TestContext {};

//
// Validate ed247_signal_get_assistant & ed247_stream_assistant_was_written for all kind of signals
//
TEST_P(assistantHelpersFixture, assistantHelpersWritten)
{
  //
  // DIS test
  //
  char dis_data = 1;

  ed247_signal_t           DisSignal01;
  ed247_stream_assistant_t DisStream1_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "DisSignal01", &DisSignal01), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(DisSignal01, &DisStream1_assistant), ED247_STATUS_SUCCESS);

  ed247_signal_t           DisSignal04;
  ed247_stream_assistant_t DisStream2_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "DisSignal04", &DisSignal04), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(DisSignal04, &DisStream2_assistant), ED247_STATUS_SUCCESS);

  SAY("Check DIS stream assistant 'was_written' ...");

  EXPECT_FALSE(ed247_stream_assistant_was_written(DisStream1_assistant));
  EXPECT_FALSE(ed247_stream_assistant_was_written(DisStream2_assistant));

  ASSERT_EQ(ed247_stream_assistant_write_signal(DisStream1_assistant, DisSignal01, &dis_data, 1), ED247_STATUS_SUCCESS);

  EXPECT_TRUE(ed247_stream_assistant_was_written(DisStream1_assistant));
  EXPECT_FALSE(ed247_stream_assistant_was_written(DisStream2_assistant));

  ASSERT_EQ(ed247_stream_assistant_push_sample(DisStream1_assistant, nullptr, nullptr), ED247_STATUS_SUCCESS);

  EXPECT_FALSE(ed247_stream_assistant_was_written(DisStream1_assistant));
  EXPECT_FALSE(ed247_stream_assistant_was_written(DisStream2_assistant));

  //
  // ANA test
  //
  float ana_data = 1.0;

  ed247_signal_t           AnaSignal01;
  ed247_stream_assistant_t AnaStream1_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "AnaSignal01", &AnaSignal01), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(AnaSignal01, &AnaStream1_assistant), ED247_STATUS_SUCCESS);

  ed247_signal_t           AnaSignal04;
  ed247_stream_assistant_t AnaStream2_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "AnaSignal04", &AnaSignal04), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(AnaSignal04, &AnaStream2_assistant), ED247_STATUS_SUCCESS);

  SAY("Check ANA stream assistant 'was_written' ...");

  EXPECT_FALSE(ed247_stream_assistant_was_written(AnaStream1_assistant));
  EXPECT_FALSE(ed247_stream_assistant_was_written(AnaStream2_assistant));

  ASSERT_EQ(ed247_stream_assistant_write_signal(AnaStream1_assistant, AnaSignal01, &ana_data, 4), ED247_STATUS_SUCCESS);

  EXPECT_TRUE(ed247_stream_assistant_was_written(AnaStream1_assistant));
  EXPECT_FALSE(ed247_stream_assistant_was_written(AnaStream2_assistant));

  ASSERT_EQ(ed247_stream_assistant_push_sample(AnaStream1_assistant, nullptr, nullptr), ED247_STATUS_SUCCESS);

  EXPECT_FALSE(ed247_stream_assistant_was_written(AnaStream1_assistant));
  EXPECT_FALSE(ed247_stream_assistant_was_written(AnaStream2_assistant));

  //
  // NAD test
  //
  uint8_t nad_data = 1;

  ed247_signal_t           NadSignal01;
  ed247_stream_assistant_t NadStream1_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "NadSignal01", &NadSignal01), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(NadSignal01, &NadStream1_assistant), ED247_STATUS_SUCCESS);

  ed247_signal_t           NadSignal04;
  ed247_stream_assistant_t NadStream2_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "NadSignal04", &NadSignal04), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(NadSignal04, &NadStream2_assistant), ED247_STATUS_SUCCESS);

  SAY("Check NAD stream assistant 'was_written' ...");

  EXPECT_FALSE(ed247_stream_assistant_was_written(NadStream1_assistant));
  EXPECT_FALSE(ed247_stream_assistant_was_written(NadStream2_assistant));

  ASSERT_EQ(ed247_stream_assistant_write_signal(NadStream1_assistant, NadSignal01, &nad_data, 1), ED247_STATUS_SUCCESS);

  EXPECT_TRUE(ed247_stream_assistant_was_written(NadStream1_assistant));
  EXPECT_FALSE(ed247_stream_assistant_was_written(NadStream2_assistant));

  ASSERT_EQ(ed247_stream_assistant_push_sample(NadStream1_assistant, nullptr, nullptr), ED247_STATUS_SUCCESS);

  EXPECT_FALSE(ed247_stream_assistant_was_written(NadStream1_assistant));
  EXPECT_FALSE(ed247_stream_assistant_was_written(NadStream2_assistant));

  //
  // VNAD test
  //
  uint8_t vnad_data = 1;

  ed247_signal_t           VnadSignal01;
  ed247_stream_assistant_t VnadStream1_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "VnadSignal01", &VnadSignal01), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(VnadSignal01, &VnadStream1_assistant), ED247_STATUS_SUCCESS);

  ed247_signal_t           VnadSignal04;
  ed247_stream_assistant_t VnadStream2_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "VnadSignal04", &VnadSignal04), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(VnadSignal04, &VnadStream2_assistant), ED247_STATUS_SUCCESS);

  SAY("Check VNAD stream assistant 'was_written' ...");

  EXPECT_FALSE(ed247_stream_assistant_was_written(VnadStream1_assistant));
  EXPECT_FALSE(ed247_stream_assistant_was_written(VnadStream2_assistant));

  ASSERT_EQ(ed247_stream_assistant_write_signal(VnadStream1_assistant, VnadSignal01, &vnad_data, 1), ED247_STATUS_SUCCESS);

  EXPECT_TRUE(ed247_stream_assistant_was_written(VnadStream1_assistant));
  EXPECT_FALSE(ed247_stream_assistant_was_written(VnadStream2_assistant));

  ASSERT_EQ(ed247_stream_assistant_push_sample(VnadStream1_assistant, nullptr, nullptr), ED247_STATUS_SUCCESS);

  EXPECT_FALSE(ed247_stream_assistant_was_written(VnadStream1_assistant));
  EXPECT_FALSE(ed247_stream_assistant_was_written(VnadStream2_assistant));

}

//
// Validate ed247_stream_assistants_written_push_samples for all kind of signals
//
TEST_P(assistantHelpersFixture, assistantHelpersPushWritten)
{
  //  ed247_set_log_level(ED247_LOG_LEVEL_CRAZY);

  // Load signal and their assistants
  // Signals that are in the same stream will have the same assistant, but we needn't to know that.
  uint8_t dis_data = 0;
  ed247_signal_t           DisSignal01;
  ed247_stream_assistant_t DisSignal01_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "DisSignal01", &DisSignal01), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(DisSignal01, &DisSignal01_assistant), ED247_STATUS_SUCCESS);
  ed247_signal_t           DisSignal03;
  ed247_stream_assistant_t DisSignal03_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "DisSignal03", &DisSignal03), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(DisSignal03, &DisSignal03_assistant), ED247_STATUS_SUCCESS);
  ed247_signal_t           DisSignal04;
  ed247_stream_assistant_t DisSignal04_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "DisSignal04", &DisSignal04), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(DisSignal04, &DisSignal04_assistant), ED247_STATUS_SUCCESS);
  ed247_signal_t           DisSignal06;
  ed247_stream_assistant_t DisSignal06_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "DisSignal06", &DisSignal06), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(DisSignal06, &DisSignal06_assistant), ED247_STATUS_SUCCESS);

  float ana_data = 0.0;
  ed247_signal_t           AnaSignal01;
  ed247_stream_assistant_t AnaSignal01_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "AnaSignal01", &AnaSignal01), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(AnaSignal01, &AnaSignal01_assistant), ED247_STATUS_SUCCESS);
  ed247_signal_t           AnaSignal03;
  ed247_stream_assistant_t AnaSignal03_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "AnaSignal03", &AnaSignal03), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(AnaSignal03, &AnaSignal03_assistant), ED247_STATUS_SUCCESS);
  ed247_signal_t           AnaSignal04;
  ed247_stream_assistant_t AnaSignal04_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "AnaSignal04", &AnaSignal04), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(AnaSignal04, &AnaSignal04_assistant), ED247_STATUS_SUCCESS);
  ed247_signal_t           AnaSignal06;
  ed247_stream_assistant_t AnaSignal06_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "AnaSignal06", &AnaSignal06), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(AnaSignal06, &AnaSignal06_assistant), ED247_STATUS_SUCCESS);

  uint8_t nad_data = 0;
  ed247_signal_t           NadSignal01;
  ed247_stream_assistant_t NadSignal01_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "NadSignal01", &NadSignal01), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(NadSignal01, &NadSignal01_assistant), ED247_STATUS_SUCCESS);
  ed247_signal_t           NadSignal03;
  ed247_stream_assistant_t NadSignal03_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "NadSignal03", &NadSignal03), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(NadSignal03, &NadSignal03_assistant), ED247_STATUS_SUCCESS);
  ed247_signal_t           NadSignal04;
  ed247_stream_assistant_t NadSignal04_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "NadSignal04", &NadSignal04), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(NadSignal04, &NadSignal04_assistant), ED247_STATUS_SUCCESS);
  ed247_signal_t           NadSignal06;
  ed247_stream_assistant_t NadSignal06_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "NadSignal06", &NadSignal06), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(NadSignal06, &NadSignal06_assistant), ED247_STATUS_SUCCESS);

  uint8_t vnad_data = 0;
  ed247_signal_t           VnadSignal01;
  ed247_stream_assistant_t VnadSignal01_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "VnadSignal01", &VnadSignal01), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(VnadSignal01, &VnadSignal01_assistant), ED247_STATUS_SUCCESS);
  ed247_signal_t           VnadSignal03;
  ed247_stream_assistant_t VnadSignal03_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "VnadSignal03", &VnadSignal03), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(VnadSignal03, &VnadSignal03_assistant), ED247_STATUS_SUCCESS);
  ed247_signal_t           VnadSignal04;
  ed247_stream_assistant_t VnadSignal04_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "VnadSignal04", &VnadSignal04), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(VnadSignal04, &VnadSignal04_assistant), ED247_STATUS_SUCCESS);
  ed247_signal_t           VnadSignal06;
  ed247_stream_assistant_t VnadSignal06_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "VnadSignal06", &VnadSignal06), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(VnadSignal06, &VnadSignal06_assistant), ED247_STATUS_SUCCESS);

  // ============================================================
  // Test1: write/push/send some signal but not all the streams
  // ============================================================

  // Write some signals (but not alls)
  dis_data = 101;
  ASSERT_EQ(ed247_stream_assistant_write_signal(DisSignal01_assistant, DisSignal01, &dis_data, 1), ED247_STATUS_SUCCESS);
  dis_data = 103;
  ASSERT_EQ(ed247_stream_assistant_write_signal(DisSignal03_assistant, DisSignal03, &dis_data, 1), ED247_STATUS_SUCCESS);
  dis_data = 106;
  ASSERT_EQ(ed247_stream_assistant_write_signal(DisSignal06_assistant, DisSignal06, &dis_data, 1), ED247_STATUS_SUCCESS);

  ana_data = 101;
  ASSERT_EQ(ed247_stream_assistant_write_signal(AnaSignal01_assistant, AnaSignal01, &ana_data, 4), ED247_STATUS_SUCCESS);
  ana_data = 103;
  ASSERT_EQ(ed247_stream_assistant_write_signal(AnaSignal03_assistant, AnaSignal03, &ana_data, 4), ED247_STATUS_SUCCESS);
  ana_data = 106;
  ASSERT_EQ(ed247_stream_assistant_write_signal(AnaSignal06_assistant, AnaSignal06, &ana_data, 4), ED247_STATUS_SUCCESS);

  nad_data = 101;
  ASSERT_EQ(ed247_stream_assistant_write_signal(NadSignal01_assistant, NadSignal01, &nad_data, 1), ED247_STATUS_SUCCESS);
  nad_data = 103;
  ASSERT_EQ(ed247_stream_assistant_write_signal(NadSignal03_assistant, NadSignal03, &nad_data, 1), ED247_STATUS_SUCCESS);
  nad_data = 106;
  ASSERT_EQ(ed247_stream_assistant_write_signal(NadSignal06_assistant, NadSignal06, &nad_data, 1), ED247_STATUS_SUCCESS);

  vnad_data = 101;
  ASSERT_EQ(ed247_stream_assistant_write_signal(VnadSignal01_assistant, VnadSignal01, &vnad_data, 1), ED247_STATUS_SUCCESS);
  vnad_data = 103;
  ASSERT_EQ(ed247_stream_assistant_write_signal(VnadSignal03_assistant, VnadSignal03, &vnad_data, 1), ED247_STATUS_SUCCESS);
  vnad_data = 106;
  ASSERT_EQ(ed247_stream_assistant_write_signal(VnadSignal06_assistant, VnadSignal06, &vnad_data, 1), ED247_STATUS_SUCCESS);

  // Push all stream assistants that has been written then send all data
  ASSERT_EQ(ed247_stream_assistants_written_push_samples(_context, nullptr), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
  TEST_SYNC("PushWritten send #1");
  TEST_SYNC("PushWritten read #1");


  // ============================================================
  // Test2: push/send nothing
  // ============================================================

  // Push and send without any new writes
  ASSERT_EQ(ed247_stream_assistants_written_push_samples(_context, nullptr), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
  TEST_SYNC("PushWritten send #2");
  TEST_SYNC("PushWritten read #2");

  // ============================================================
  // Test3: write/push/send other signals than in test1
  // ============================================================

  // Write some signals (but not alls)
  dis_data = 104;
  ASSERT_EQ(ed247_stream_assistant_write_signal(DisSignal04_assistant, DisSignal04, &dis_data, 1), ED247_STATUS_SUCCESS);

  ana_data = 104;
  ASSERT_EQ(ed247_stream_assistant_write_signal(AnaSignal04_assistant, AnaSignal04, &ana_data, 4), ED247_STATUS_SUCCESS);

  nad_data = 104;
  ASSERT_EQ(ed247_stream_assistant_write_signal(NadSignal04_assistant, NadSignal04, &nad_data, 1), ED247_STATUS_SUCCESS);

  vnad_data = 104;
  ASSERT_EQ(ed247_stream_assistant_write_signal(VnadSignal04_assistant, VnadSignal04, &vnad_data, 1), ED247_STATUS_SUCCESS);

  // Push all stream assistants that has been written then send all data
  ASSERT_EQ(ed247_stream_assistants_written_push_samples(_context, nullptr), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
  TEST_SYNC("PushWritten send #3");
  TEST_SYNC("PushWritten read #3");
}


std::vector<TestParams> ecic_files;
INSTANTIATE_TEST_CASE_P(assistantHelpers, assistantHelpersFixture,
                        ::testing::ValuesIn(ecic_files));

int main(int argc, char **argv)
{
  std::string config_path = (argc >=1)? argv[1] : "../config";

  tests_tools::display_ed247_lib_infos();
  SAY("Configuration path: " << config_path);

  ecic_files.push_back({TEST_ACTOR_ID, config_path + "/ecic_func_streamassistant_helpers_sender.xml"});

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
