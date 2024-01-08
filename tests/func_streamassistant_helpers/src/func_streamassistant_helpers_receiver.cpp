#define TEST_ACTOR_ID TEST_ACTOR2_ID
#include "two_actors_test.h"

class assistantHelpersFixture : public TestContext {};

TEST_P(assistantHelpersFixture, assistantHelpersWritten)
{
  // All tests done in sender
}

TEST_P(assistantHelpersFixture, assistantHelpersPushWritten)
{
  bool empty = true;
  uint32_t size = 0;

  //  ed247_set_log_level(ED247_LOG_LEVEL_CRAZY);

  // Read the configuration. But, unlike the sender, we use the stream to get their signals and its assistant.
  const uint8_t* dis_data = nullptr;
  ed247_stream_t DisStream1;
  ed247_stream_assistant_t DisStream1_assistant;
  ed247_signal_t DisSignal01;
  ed247_signal_t DisSignal03;
  ASSERT_EQ(ed247_get_stream(_context, "DisStream1", &DisStream1), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_assistant(DisStream1, &DisStream1_assistant), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_signal(DisStream1, "DisSignal01", &DisSignal01), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_signal(DisStream1, "DisSignal03", &DisSignal03), ED247_STATUS_SUCCESS);
  ed247_stream_t DisStream2;
  ed247_stream_assistant_t DisStream2_assistant;
  ed247_signal_t DisSignal04;
  ASSERT_EQ(ed247_get_stream(_context, "DisStream2", &DisStream2), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_assistant(DisStream2, &DisStream2_assistant), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_signal(DisStream2, "DisSignal04", &DisSignal04), ED247_STATUS_SUCCESS);
  ed247_stream_t DisStream3;
  ed247_stream_assistant_t DisStream3_assistant;
  ed247_signal_t DisSignal06;
  ASSERT_EQ(ed247_get_stream(_context, "DisStream3", &DisStream3), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_assistant(DisStream3, &DisStream3_assistant), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_signal(DisStream3, "DisSignal06", &DisSignal06), ED247_STATUS_SUCCESS);

  const float* ana_data = nullptr;
  ed247_stream_t AnaStream1;
  ed247_stream_assistant_t AnaStream1_assistant;
  ed247_signal_t AnaSignal01;
  ed247_signal_t AnaSignal03;
  ASSERT_EQ(ed247_get_stream(_context, "AnaStream1", &AnaStream1), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_assistant(AnaStream1, &AnaStream1_assistant), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_signal(AnaStream1, "AnaSignal01", &AnaSignal01), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_signal(AnaStream1, "AnaSignal03", &AnaSignal03), ED247_STATUS_SUCCESS);
  ed247_stream_t AnaStream2;
  ed247_stream_assistant_t AnaStream2_assistant;
  ed247_signal_t AnaSignal04;
  ASSERT_EQ(ed247_get_stream(_context, "AnaStream2", &AnaStream2), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_assistant(AnaStream2, &AnaStream2_assistant), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_signal(AnaStream2, "AnaSignal04", &AnaSignal04), ED247_STATUS_SUCCESS);
  ed247_stream_t AnaStream3;
  ed247_stream_assistant_t AnaStream3_assistant;
  ed247_signal_t AnaSignal06;
  ASSERT_EQ(ed247_get_stream(_context, "AnaStream3", &AnaStream3), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_assistant(AnaStream3, &AnaStream3_assistant), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_signal(AnaStream3, "AnaSignal06", &AnaSignal06), ED247_STATUS_SUCCESS);

  const uint8_t* nad_data = nullptr;
  ed247_stream_t NadStream1;
  ed247_stream_assistant_t NadStream1_assistant;
  ed247_signal_t NadSignal01;
  ed247_signal_t NadSignal03;
  ASSERT_EQ(ed247_get_stream(_context, "NadStream1", &NadStream1), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_assistant(NadStream1, &NadStream1_assistant), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_signal(NadStream1, "NadSignal01", &NadSignal01), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_signal(NadStream1, "NadSignal03", &NadSignal03), ED247_STATUS_SUCCESS);
  ed247_stream_t NadStream2;
  ed247_stream_assistant_t NadStream2_assistant;
  ed247_signal_t NadSignal04;
  ASSERT_EQ(ed247_get_stream(_context, "NadStream2", &NadStream2), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_assistant(NadStream2, &NadStream2_assistant), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_signal(NadStream2, "NadSignal04", &NadSignal04), ED247_STATUS_SUCCESS);
  ed247_stream_t NadStream3;
  ed247_stream_assistant_t NadStream3_assistant;
  ed247_signal_t NadSignal06;
  ASSERT_EQ(ed247_get_stream(_context, "NadStream3", &NadStream3), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_assistant(NadStream3, &NadStream3_assistant), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_signal(NadStream3, "NadSignal06", &NadSignal06), ED247_STATUS_SUCCESS);

  const uint8_t* vnad_data = nullptr;
  ed247_stream_t VnadStream1;
  ed247_stream_assistant_t VnadStream1_assistant;
  ed247_signal_t VnadSignal01;
  ed247_signal_t VnadSignal03;
  ASSERT_EQ(ed247_get_stream(_context, "VnadStream1", &VnadStream1), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_assistant(VnadStream1, &VnadStream1_assistant), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_signal(VnadStream1, "VnadSignal01", &VnadSignal01), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_signal(VnadStream1, "VnadSignal03", &VnadSignal03), ED247_STATUS_SUCCESS);
  ed247_stream_t VnadStream2;
  ed247_stream_assistant_t VnadStream2_assistant;
  ed247_signal_t VnadSignal04;
  ASSERT_EQ(ed247_get_stream(_context, "VnadStream2", &VnadStream2), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_assistant(VnadStream2, &VnadStream2_assistant), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_signal(VnadStream2, "VnadSignal04", &VnadSignal04), ED247_STATUS_SUCCESS);
  ed247_stream_t VnadStream3;
  ed247_stream_assistant_t VnadStream3_assistant;
  ed247_signal_t VnadSignal06;
  ASSERT_EQ(ed247_get_stream(_context, "VnadStream3", &VnadStream3), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_assistant(VnadStream3, &VnadStream3_assistant), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_stream_get_signal(VnadStream3, "VnadSignal06", &VnadSignal06), ED247_STATUS_SUCCESS);

  // ============================================================
  // Test1: write/push/send some signal but not all the streams
  // ============================================================

  // Wait for the sender to write/push/send
  TEST_SYNC("PushWritten send #1");
  ASSERT_EQ(ed247_wait_during(_context, nullptr, 0), ED247_STATUS_SUCCESS);

  // We shall have received only one time the DisStream1
  ASSERT_EQ(ed247_stream_assistant_pop_sample(DisStream1_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_SUCCESS);
  EXPECT_TRUE(empty);
  ASSERT_EQ(ed247_stream_assistant_pop_sample(DisStream1_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  ASSERT_EQ(ed247_stream_assistant_read_signal(DisStream1_assistant, DisSignal01, (const void**)&dis_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*dis_data, 101);
  ASSERT_EQ(ed247_stream_assistant_read_signal(DisStream1_assistant, DisSignal03, (const void**)&dis_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*dis_data, 103);
  // We shall not have received the DisStream2
  ASSERT_EQ(ed247_stream_assistant_pop_sample(DisStream2_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  EXPECT_TRUE(empty);
  // We shall have received only one time the DisStream3
  ASSERT_EQ(ed247_stream_assistant_pop_sample(DisStream3_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_SUCCESS);
  EXPECT_TRUE(empty);
  ASSERT_EQ(ed247_stream_assistant_pop_sample(DisStream3_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  EXPECT_EQ(ed247_stream_assistant_read_signal(DisStream3_assistant, DisSignal06, (const void**)&dis_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*dis_data, 106);

  // We shall have received only one time the AnaStream1
  ASSERT_EQ(ed247_stream_assistant_pop_sample(AnaStream1_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_SUCCESS);
  EXPECT_TRUE(empty);
  ASSERT_EQ(ed247_stream_assistant_pop_sample(AnaStream1_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  ASSERT_EQ(ed247_stream_assistant_read_signal(AnaStream1_assistant, AnaSignal01, (const void**)&ana_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 4); EXPECT_EQ(*ana_data, 101);
  ASSERT_EQ(ed247_stream_assistant_read_signal(AnaStream1_assistant, AnaSignal03, (const void**)&ana_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 4); EXPECT_EQ(*ana_data, 103);
  // We shall not have received the AnaStream2
  ASSERT_EQ(ed247_stream_assistant_pop_sample(AnaStream2_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  EXPECT_TRUE(empty);
  // We shall have received only one time the AnaStream3
  ASSERT_EQ(ed247_stream_assistant_pop_sample(AnaStream3_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_SUCCESS);
  EXPECT_TRUE(empty);
  ASSERT_EQ(ed247_stream_assistant_pop_sample(AnaStream3_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  EXPECT_EQ(ed247_stream_assistant_read_signal(AnaStream3_assistant, AnaSignal06, (const void**)&ana_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 4); EXPECT_EQ(*ana_data, 106);

  // We shall have received only one time the NadStream1
  ASSERT_EQ(ed247_stream_assistant_pop_sample(NadStream1_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_SUCCESS);
  EXPECT_TRUE(empty);
  ASSERT_EQ(ed247_stream_assistant_pop_sample(NadStream1_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  ASSERT_EQ(ed247_stream_assistant_read_signal(NadStream1_assistant, NadSignal01, (const void**)&nad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*nad_data, 101);
  ASSERT_EQ(ed247_stream_assistant_read_signal(NadStream1_assistant, NadSignal03, (const void**)&nad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*nad_data, 103);
  // We shall not have received the NadStream2
  ASSERT_EQ(ed247_stream_assistant_pop_sample(NadStream2_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  EXPECT_TRUE(empty);
  // We shall have received only one time the NadStream3
  ASSERT_EQ(ed247_stream_assistant_pop_sample(NadStream3_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_SUCCESS);
  EXPECT_TRUE(empty);
  ASSERT_EQ(ed247_stream_assistant_pop_sample(NadStream3_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  EXPECT_EQ(ed247_stream_assistant_read_signal(NadStream3_assistant, NadSignal06, (const void**)&nad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*nad_data, 106);

  // We shall have received only one time the VnadStream1
  ASSERT_EQ(ed247_stream_assistant_pop_sample(VnadStream1_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_SUCCESS);
  EXPECT_TRUE(empty);
  ASSERT_EQ(ed247_stream_assistant_pop_sample(VnadStream1_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  ASSERT_EQ(ed247_stream_assistant_read_signal(VnadStream1_assistant, VnadSignal01, (const void**)&vnad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*vnad_data, 101);
  ASSERT_EQ(ed247_stream_assistant_read_signal(VnadStream1_assistant, VnadSignal03, (const void**)&vnad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*vnad_data, 103);
  // We shall not have received the VnadStream2
  ASSERT_EQ(ed247_stream_assistant_pop_sample(VnadStream2_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  EXPECT_TRUE(empty);
  // We shall have received only one time the VnadStream3
  ASSERT_EQ(ed247_stream_assistant_pop_sample(VnadStream3_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_SUCCESS);
  EXPECT_TRUE(empty);
  ASSERT_EQ(ed247_stream_assistant_pop_sample(VnadStream3_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  EXPECT_EQ(ed247_stream_assistant_read_signal(VnadStream3_assistant, VnadSignal06, (const void**)&vnad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*vnad_data, 106);

  // Notify we have read all data
  TEST_SYNC("PushWritten read #1");


  // ============================================================
  // Test2: push/send nothing
  // ============================================================

  // Wait for the sender to push/send again
  TEST_SYNC("PushWritten send #2");
  ASSERT_EQ(ed247_wait_during(_context, nullptr, 0), ED247_STATUS_NODATA);

  // Nothing shall have been received
  ASSERT_EQ(ed247_stream_assistant_pop_sample(DisStream1_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  EXPECT_TRUE(empty);
  ASSERT_EQ(ed247_stream_assistant_pop_sample(DisStream2_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  EXPECT_TRUE(empty);
  ASSERT_EQ(ed247_stream_assistant_pop_sample(DisStream3_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  EXPECT_TRUE(empty);

  // Notify we have read all data
  TEST_SYNC("PushWritten read #2");


  // ============================================================
  // Test3: write/push/send other signals than in test1
  // ============================================================

  // Wait for the sender to write/push/send again
  TEST_SYNC("PushWritten send #3");
  ASSERT_EQ(ed247_wait_during(_context, nullptr, 0), ED247_STATUS_SUCCESS);

  // We shall not have received the DisStream1 and signal values sahll not have changed
  ASSERT_EQ(ed247_stream_assistant_pop_sample(DisStream1_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  EXPECT_TRUE(empty);
  ASSERT_EQ(ed247_stream_assistant_read_signal(DisStream1_assistant, DisSignal01, (const void**)&dis_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*dis_data, 101);
  ASSERT_EQ(ed247_stream_assistant_read_signal(DisStream1_assistant, DisSignal03, (const void**)&dis_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*dis_data, 103);
  // We shall have received only one time the DisStream2
  ASSERT_EQ(ed247_stream_assistant_pop_sample(DisStream2_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_SUCCESS);
  EXPECT_TRUE(empty);
  ASSERT_EQ(ed247_stream_assistant_pop_sample(DisStream2_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  ASSERT_EQ(ed247_stream_assistant_read_signal(DisStream2_assistant, DisSignal04, (const void**)&dis_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*dis_data, 104);
  // We shall not have received the DisStream3 and signal values sahll not have changed
  ASSERT_EQ(ed247_stream_assistant_pop_sample(DisStream3_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  EXPECT_TRUE(empty);
  EXPECT_EQ(ed247_stream_assistant_read_signal(DisStream3_assistant, DisSignal06, (const void**)&dis_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*dis_data, 106);

  // We shall not have received the AnaStream1 and signal values sahll not have changed
  ASSERT_EQ(ed247_stream_assistant_pop_sample(AnaStream1_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  EXPECT_TRUE(empty);
  ASSERT_EQ(ed247_stream_assistant_read_signal(AnaStream1_assistant, AnaSignal01, (const void**)&ana_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 4); EXPECT_EQ(*ana_data, 101);
  ASSERT_EQ(ed247_stream_assistant_read_signal(AnaStream1_assistant, AnaSignal03, (const void**)&ana_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 4); EXPECT_EQ(*ana_data, 103);
  // We shall have received only one time the AnaStream2
  ASSERT_EQ(ed247_stream_assistant_pop_sample(AnaStream2_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_SUCCESS);
  EXPECT_TRUE(empty);
  ASSERT_EQ(ed247_stream_assistant_pop_sample(AnaStream2_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  ASSERT_EQ(ed247_stream_assistant_read_signal(AnaStream2_assistant, AnaSignal04, (const void**)&ana_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 4); EXPECT_EQ(*ana_data, 104);
  // We shall not have received the AnaStream3 and signal values sahll not have changed
  ASSERT_EQ(ed247_stream_assistant_pop_sample(AnaStream3_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  EXPECT_TRUE(empty);
  EXPECT_EQ(ed247_stream_assistant_read_signal(AnaStream3_assistant, AnaSignal06, (const void**)&ana_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 4); EXPECT_EQ(*ana_data, 106);

  // We shall not have received the NadStream1 and signal values sahll not have changed
  ASSERT_EQ(ed247_stream_assistant_pop_sample(NadStream1_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  EXPECT_TRUE(empty);
  ASSERT_EQ(ed247_stream_assistant_read_signal(NadStream1_assistant, NadSignal01, (const void**)&nad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*nad_data, 101);
  ASSERT_EQ(ed247_stream_assistant_read_signal(NadStream1_assistant, NadSignal03, (const void**)&nad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*nad_data, 103);
  // We shall have received only one time the NadStream2
  ASSERT_EQ(ed247_stream_assistant_pop_sample(NadStream2_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_SUCCESS);
  EXPECT_TRUE(empty);
  ASSERT_EQ(ed247_stream_assistant_pop_sample(NadStream2_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  ASSERT_EQ(ed247_stream_assistant_read_signal(NadStream2_assistant, NadSignal04, (const void**)&nad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*nad_data, 104);
  // We shall not have received the NadStream3 and signal values sahll not have changed
  ASSERT_EQ(ed247_stream_assistant_pop_sample(NadStream3_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  EXPECT_TRUE(empty);
  EXPECT_EQ(ed247_stream_assistant_read_signal(NadStream3_assistant, NadSignal06, (const void**)&nad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*nad_data, 106);

  // We shall not have received the VnadStream1 and signal values sahll not have changed
  ASSERT_EQ(ed247_stream_assistant_pop_sample(VnadStream1_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  EXPECT_TRUE(empty);
  ASSERT_EQ(ed247_stream_assistant_read_signal(VnadStream1_assistant, VnadSignal01, (const void**)&vnad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*vnad_data, 101);
  ASSERT_EQ(ed247_stream_assistant_read_signal(VnadStream1_assistant, VnadSignal03, (const void**)&vnad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*vnad_data, 103);
  // We shall have received only one time the VnadStream2
  ASSERT_EQ(ed247_stream_assistant_pop_sample(VnadStream2_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_SUCCESS);
  EXPECT_TRUE(empty);
  ASSERT_EQ(ed247_stream_assistant_pop_sample(VnadStream2_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  ASSERT_EQ(ed247_stream_assistant_read_signal(VnadStream2_assistant, VnadSignal04, (const void**)&vnad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*vnad_data, 104);
  // We shall not have received the VnadStream3 and signal values sahll not have changed
  ASSERT_EQ(ed247_stream_assistant_pop_sample(VnadStream3_assistant, nullptr, nullptr, nullptr, &empty), ED247_STATUS_NODATA);
  EXPECT_TRUE(empty);
  EXPECT_EQ(ed247_stream_assistant_read_signal(VnadStream3_assistant, VnadSignal06, (const void**)&vnad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*vnad_data, 106);

  // Notify we have read all data
  TEST_SYNC("PushWritten read #3");

}

//
// stream_assistants_pop_samples for all kind of signals
//
TEST_P(assistantHelpersFixture, assistantHelpersPopAll)
{
  uint32_t size = 0;

  //  ed247_set_log_level(ED247_LOG_LEVEL_CRAZY);

  // Load signal and their assistants
  // Signals that are in the same stream will have the same assistant, but we needn't to know that.
  uint8_t* dis_data = nullptr;
  ed247_signal_t           DisSignal01;
  ed247_stream_assistant_t DisSignal01_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "DisSignal01", &DisSignal01), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(DisSignal01, &DisSignal01_assistant), ED247_STATUS_SUCCESS);
  ed247_signal_t           DisSignal02;
  ed247_stream_assistant_t DisSignal02_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "DisSignal02", &DisSignal02), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(DisSignal02, &DisSignal02_assistant), ED247_STATUS_SUCCESS);
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

  float* ana_data = nullptr;
  ed247_signal_t           AnaSignal01;
  ed247_stream_assistant_t AnaSignal01_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "AnaSignal01", &AnaSignal01), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(AnaSignal01, &AnaSignal01_assistant), ED247_STATUS_SUCCESS);
  ed247_signal_t           AnaSignal02;
  ed247_stream_assistant_t AnaSignal02_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "AnaSignal02", &AnaSignal02), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(AnaSignal02, &AnaSignal02_assistant), ED247_STATUS_SUCCESS);
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

  uint8_t* nad_data = nullptr;
  ed247_signal_t           NadSignal01;
  ed247_stream_assistant_t NadSignal01_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "NadSignal01", &NadSignal01), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(NadSignal01, &NadSignal01_assistant), ED247_STATUS_SUCCESS);
  ed247_signal_t           NadSignal02;
  ed247_stream_assistant_t NadSignal02_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "NadSignal02", &NadSignal02), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(NadSignal02, &NadSignal02_assistant), ED247_STATUS_SUCCESS);
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

  uint8_t* vnad_data = nullptr;
  ed247_signal_t           VnadSignal01;
  ed247_stream_assistant_t VnadSignal01_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "VnadSignal01", &VnadSignal01), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(VnadSignal01, &VnadSignal01_assistant), ED247_STATUS_SUCCESS);
  ed247_signal_t           VnadSignal02;
  ed247_stream_assistant_t VnadSignal02_assistant;
  ASSERT_EQ(ed247_get_signal(_context, "VnadSignal02", &VnadSignal02), ED247_STATUS_SUCCESS);
  ASSERT_EQ(ed247_signal_get_assistant(VnadSignal02, &VnadSignal02_assistant), ED247_STATUS_SUCCESS);
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

  // Wait for the sender to write/push/send
  TEST_SYNC("PopAll send #1");
  ASSERT_EQ(ed247_wait_during(_context, nullptr, 0), ED247_STATUS_SUCCESS);

  // Pop all signals
  ASSERT_EQ(stream_assistants_pop_samples(_context), ED247_STATUS_SUCCESS);

  // Check DIS received signal values
  ASSERT_EQ(ed247_stream_assistant_read_signal(DisSignal01_assistant, DisSignal01, (const void**)&dis_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*dis_data, 101);
  ASSERT_EQ(ed247_stream_assistant_read_signal(DisSignal03_assistant, DisSignal03, (const void**)&dis_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*dis_data, 103);
  ASSERT_EQ(ed247_stream_assistant_read_signal(DisSignal06_assistant, DisSignal06, (const void**)&dis_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*dis_data, 106);
  // DisSignal04 has never been received, its value shall be 0
  ASSERT_EQ(ed247_stream_assistant_read_signal(DisSignal04_assistant, DisSignal04, (const void**)&dis_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*dis_data, 0);

  // Check ANA received signal values
  ASSERT_EQ(ed247_stream_assistant_read_signal(AnaSignal01_assistant, AnaSignal01, (const void**)&ana_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 4); EXPECT_EQ(*ana_data, 101);
  ASSERT_EQ(ed247_stream_assistant_read_signal(AnaSignal03_assistant, AnaSignal03, (const void**)&ana_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 4); EXPECT_EQ(*ana_data, 103);
  ASSERT_EQ(ed247_stream_assistant_read_signal(AnaSignal06_assistant, AnaSignal06, (const void**)&ana_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 4); EXPECT_EQ(*ana_data, 106);
  // AnaSignal04 has never been received, its value shall be 0
  ASSERT_EQ(ed247_stream_assistant_read_signal(AnaSignal04_assistant, AnaSignal04, (const void**)&ana_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 4); EXPECT_EQ(*ana_data, 0);

  // Check NAD received signal values
  ASSERT_EQ(ed247_stream_assistant_read_signal(NadSignal01_assistant, NadSignal01, (const void**)&nad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*nad_data, 101);
  ASSERT_EQ(ed247_stream_assistant_read_signal(NadSignal03_assistant, NadSignal03, (const void**)&nad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*nad_data, 103);
  ASSERT_EQ(ed247_stream_assistant_read_signal(NadSignal06_assistant, NadSignal06, (const void**)&nad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*nad_data, 106);
  // NadSignal04 has never been received, its value shall be 0
  ASSERT_EQ(ed247_stream_assistant_read_signal(NadSignal04_assistant, NadSignal04, (const void**)&nad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*nad_data, 0);

  // Check VNAD received signal values
  ASSERT_EQ(ed247_stream_assistant_read_signal(VnadSignal01_assistant, VnadSignal01, (const void**)&vnad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*vnad_data, 101);
  ASSERT_EQ(ed247_stream_assistant_read_signal(VnadSignal03_assistant, VnadSignal03, (const void**)&vnad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*vnad_data, 103);
  ASSERT_EQ(ed247_stream_assistant_read_signal(VnadSignal06_assistant, VnadSignal06, (const void**)&vnad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*vnad_data, 106);
  // VnadSignal04 has never been received
  ASSERT_EQ(ed247_stream_assistant_read_signal(VnadSignal04_assistant, VnadSignal04, (const void**)&vnad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 0);

  // Notify we have read all data
  TEST_SYNC("PopAll read #1");


  // ============================================================
  // Test2: push/send nothing
  // ============================================================

  // Wait for the sender to send
  TEST_SYNC("PopAll send #2");
  ASSERT_EQ(ed247_wait_during(_context, nullptr, 0), ED247_STATUS_NODATA);

  // Pop all signals
  ASSERT_EQ(stream_assistants_pop_samples(_context), ED247_STATUS_SUCCESS);

  // Check DIS signals kept their old values
  ASSERT_EQ(ed247_stream_assistant_read_signal(DisSignal01_assistant, DisSignal01, (const void**)&dis_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*dis_data, 101);
  ASSERT_EQ(ed247_stream_assistant_read_signal(DisSignal03_assistant, DisSignal03, (const void**)&dis_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*dis_data, 103);
  ASSERT_EQ(ed247_stream_assistant_read_signal(DisSignal06_assistant, DisSignal06, (const void**)&dis_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*dis_data, 106);
  // DisSignal04 has never been received, its value shall be 0
  ASSERT_EQ(ed247_stream_assistant_read_signal(DisSignal04_assistant, DisSignal04, (const void**)&dis_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*dis_data, 0);

  // Check ANA signals kept their old values
  ASSERT_EQ(ed247_stream_assistant_read_signal(AnaSignal01_assistant, AnaSignal01, (const void**)&ana_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 4); EXPECT_EQ(*ana_data, 101);
  ASSERT_EQ(ed247_stream_assistant_read_signal(AnaSignal03_assistant, AnaSignal03, (const void**)&ana_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 4); EXPECT_EQ(*ana_data, 103);
  ASSERT_EQ(ed247_stream_assistant_read_signal(AnaSignal06_assistant, AnaSignal06, (const void**)&ana_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 4); EXPECT_EQ(*ana_data, 106);
  // AnaSignal04 has never been received, its value shall be 0
  ASSERT_EQ(ed247_stream_assistant_read_signal(AnaSignal04_assistant, AnaSignal04, (const void**)&ana_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 4); EXPECT_EQ(*ana_data, 0);

  // Check NAD signals kept their old values
  ASSERT_EQ(ed247_stream_assistant_read_signal(NadSignal01_assistant, NadSignal01, (const void**)&nad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*nad_data, 101);
  ASSERT_EQ(ed247_stream_assistant_read_signal(NadSignal03_assistant, NadSignal03, (const void**)&nad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*nad_data, 103);
  ASSERT_EQ(ed247_stream_assistant_read_signal(NadSignal06_assistant, NadSignal06, (const void**)&nad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*nad_data, 106);
  // NadSignal04 has never been received, its value shall be 0
  ASSERT_EQ(ed247_stream_assistant_read_signal(NadSignal04_assistant, NadSignal04, (const void**)&nad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*nad_data, 0);

  // Check VNAD signals kept their old values
  ASSERT_EQ(ed247_stream_assistant_read_signal(VnadSignal01_assistant, VnadSignal01, (const void**)&vnad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*vnad_data, 101);
  ASSERT_EQ(ed247_stream_assistant_read_signal(VnadSignal03_assistant, VnadSignal03, (const void**)&vnad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*vnad_data, 103);
  ASSERT_EQ(ed247_stream_assistant_read_signal(VnadSignal06_assistant, VnadSignal06, (const void**)&vnad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*vnad_data, 106);
  // VnadSignal04 has never been received
  ASSERT_EQ(ed247_stream_assistant_read_signal(VnadSignal04_assistant, VnadSignal04, (const void**)&vnad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 0);

  // Notify we have read all data
  TEST_SYNC("PopAll read #2");


  // ============================================================
  // Test3: write/push/send other signals than in test1 and
  // several times.
  // ============================================================

  // Wait for the sender to write/push/send
  TEST_SYNC("PopAll send #3");
  ASSERT_EQ(ed247_wait_during(_context, nullptr, 0), ED247_STATUS_SUCCESS);

  // Pop all signals
  ASSERT_EQ(stream_assistants_pop_samples(_context), ED247_STATUS_SUCCESS);


  // Check DIS signals
  ASSERT_EQ(ed247_stream_assistant_read_signal(DisSignal01_assistant, DisSignal01, (const void**)&dis_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*dis_data, 101);
  ASSERT_EQ(ed247_stream_assistant_read_signal(DisSignal03_assistant, DisSignal03, (const void**)&dis_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*dis_data, 103);
  ASSERT_EQ(ed247_stream_assistant_read_signal(DisSignal06_assistant, DisSignal06, (const void**)&dis_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*dis_data, 106);
  ASSERT_EQ(ed247_stream_assistant_read_signal(DisSignal04_assistant, DisSignal04, (const void**)&dis_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*dis_data, 134);
  // DisSignal02 has never been received, its value shall be 0
  ASSERT_EQ(ed247_stream_assistant_read_signal(DisSignal02_assistant, DisSignal02, (const void**)&dis_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*dis_data, 0);

  // Check ANA signals
  ASSERT_EQ(ed247_stream_assistant_read_signal(AnaSignal01_assistant, AnaSignal01, (const void**)&ana_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 4); EXPECT_EQ(*ana_data, 101);
  ASSERT_EQ(ed247_stream_assistant_read_signal(AnaSignal03_assistant, AnaSignal03, (const void**)&ana_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 4); EXPECT_EQ(*ana_data, 103);
  ASSERT_EQ(ed247_stream_assistant_read_signal(AnaSignal06_assistant, AnaSignal06, (const void**)&ana_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 4); EXPECT_EQ(*ana_data, 106);
  ASSERT_EQ(ed247_stream_assistant_read_signal(AnaSignal04_assistant, AnaSignal04, (const void**)&ana_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 4); EXPECT_EQ(*ana_data, 134);
  // AnaSignal02 has never been received, its value shall be 0
  ASSERT_EQ(ed247_stream_assistant_read_signal(AnaSignal02_assistant, AnaSignal02, (const void**)&ana_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 4); EXPECT_EQ(*ana_data, 0);

  // Check NAD signals
  ASSERT_EQ(ed247_stream_assistant_read_signal(NadSignal01_assistant, NadSignal01, (const void**)&nad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*nad_data, 101);
  ASSERT_EQ(ed247_stream_assistant_read_signal(NadSignal03_assistant, NadSignal03, (const void**)&nad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*nad_data, 103);
  ASSERT_EQ(ed247_stream_assistant_read_signal(NadSignal06_assistant, NadSignal06, (const void**)&nad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*nad_data, 106);
  ASSERT_EQ(ed247_stream_assistant_read_signal(NadSignal04_assistant, NadSignal04, (const void**)&nad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*nad_data, 134);
  // NadSignal02 has never been received, its value shall be 0
  ASSERT_EQ(ed247_stream_assistant_read_signal(NadSignal02_assistant, NadSignal02, (const void**)&nad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*nad_data, 0);

  // Check VNAD signals
  ASSERT_EQ(ed247_stream_assistant_read_signal(VnadSignal01_assistant, VnadSignal01, (const void**)&vnad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*vnad_data, 101);
  ASSERT_EQ(ed247_stream_assistant_read_signal(VnadSignal03_assistant, VnadSignal03, (const void**)&vnad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*vnad_data, 103);
  ASSERT_EQ(ed247_stream_assistant_read_signal(VnadSignal06_assistant, VnadSignal06, (const void**)&vnad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*vnad_data, 106);
  ASSERT_EQ(ed247_stream_assistant_read_signal(VnadSignal04_assistant, VnadSignal04, (const void**)&vnad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 1); EXPECT_EQ(*vnad_data, 134);
  // VnadSignal02 has never been received
  ASSERT_EQ(ed247_stream_assistant_read_signal(VnadSignal02_assistant, VnadSignal02, (const void**)&vnad_data, &size), ED247_STATUS_SUCCESS);
  EXPECT_EQ(size, 0);


  // Notify we have read all data
  TEST_SYNC("PopAll read #3");
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
