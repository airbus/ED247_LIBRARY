#define TEST_ACTOR_ID TEST_ACTOR1_ID
#include "two_actors_test.h"

class signalsAutoPushFixture : public TestContext {};

TEST_P(signalsAutoPushFixture, signalsAutoPushMain)
{
  ed247_signal_t           DisSignal01;
  ed247_stream_assistant_t DisStream1_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "DisSignal01", &DisSignal01), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(DisSignal01, &DisStream1_assistant), ED247_STATUS_SUCCESS);

  ed247_signal_t           DisSignal04;
  ed247_stream_assistant_t DisStream2_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "DisSignal04", &DisSignal04), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(DisSignal04, &DisStream2_assistant), ED247_STATUS_SUCCESS);
}


std::vector<TestParams> ecic_files;
INSTANTIATE_TEST_CASE_P(signalsAutoPush, signalsAutoPushFixture,
                        ::testing::ValuesIn(ecic_files));

int main(int argc, char **argv)
{
  std::string config_path = (argc >=1)? argv[1] : "../config";

  tests_tools::display_ed247_lib_infos();
  SAY("Configuration path: " << config_path);

  ecic_files.push_back({TEST_ACTOR_ID, config_path + "/ecic_func_signals_autopush_sender.xml"});

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
