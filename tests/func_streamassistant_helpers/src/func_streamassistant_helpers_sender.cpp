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
