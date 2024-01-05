#define TEST_ACTOR_ID TEST_ACTOR2_ID
#include "two_actors_test.h"

class assistantHelpersFixture : public TestContext {};

TEST_P(assistantHelpersFixture, assistantHelpersWritten)
{
  // All tests done in sender
}


std::vector<TestParams> ecic_files;
INSTANTIATE_TEST_CASE_P(assistantHelpers, assistantHelpersFixture,
                        ::testing::ValuesIn(ecic_files));

int main(int argc, char **argv)
{
  std::string config_path = (argc >=1)? argv[1] : "../config";

  tests_tools::display_ed247_lib_infos();
  SAY("Configuration path: " << config_path);

  ecic_files.push_back({TEST_ACTOR_ID, config_path + "/ecic_func_streamassistant_helpers_receiver.xml"});

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
