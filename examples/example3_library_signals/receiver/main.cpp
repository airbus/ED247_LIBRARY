#include <unistd.h>
#include "ed247.h"
#include "traces.h"
#include "sync.h"
#include <unordered_map>

#define ECIC EC_ROOT "ECIC.xml"

int main() {
  LOG_BOOLALPHA();
  SAY(VERSION);

  SAY("Load ECIC " ECIC);
  ed247_context_t ed247_context;
  ASSERT(ed247_load_file(ECIC, &ed247_context) == ED247_STATUS_SUCCESS);

  const uint8_t* data = nullptr;
  uint32_t size = 0;

  // The ED247 protocol define "true" as a byte with all bits at one.
  // The ED247 library doesn't check the value: it only read/write the data provided by the user.
  uint8_t dis_true = 255;
  uint8_t dis_false = 0;


  // Get references to our signals and their assistants.
  // Lease red comments on sender/main.cpp
  ed247_signal_t dis_signal1;
  ed247_signal_t dis_signal2;
  ed247_signal_t dis_signal3;
  ed247_stream_assistant_t dis_signal1_assistant;
  ed247_stream_assistant_t dis_signal2_assistant;
  ed247_stream_assistant_t dis_signal3_assistant;
  ASSERT(ed247_get_signal(ed247_context, "DIS_SIGNAL1", &dis_signal1) == ED247_STATUS_SUCCESS);
  ASSERT(ed247_get_signal(ed247_context, "DIS_SIGNAL2", &dis_signal2) == ED247_STATUS_SUCCESS);
  ASSERT(ed247_get_signal(ed247_context, "DIS_SIGNAL3", &dis_signal3) == ED247_STATUS_SUCCESS);
  ASSERT(ed247_signal_get_assistant(dis_signal1, &dis_signal1_assistant) == ED247_STATUS_SUCCESS);
  ASSERT(ed247_signal_get_assistant(dis_signal2, &dis_signal2_assistant) == ED247_STATUS_SUCCESS);
  ASSERT(ed247_signal_get_assistant(dis_signal3, &dis_signal3_assistant) == ED247_STATUS_SUCCESS);


  // =========================================================================
  // Demo 0 - Signal are always available and their default value is 0.
  LOG("------ [ DEMO 0 ] -------");

  // We always can read signal values, regardless if signal has been received or not.
  // The library guaranty that a never received signal will have a value of 0.
  ASSERT(ed247_stream_assistant_read_signal(dis_signal1_assistant, dis_signal1, (const void**) &data, &size) == ED247_STATUS_SUCCESS);
  ASSERT(size == 1 && *data == 0);
  SAY("Non received signal can be read and has a value of 0.");


  // =========================================================================
  // Demo 1 - Simple API: stream assistant and automatic push/pop written signals
  LOG("------ [ DEMO 1 ] -------");
  SAY("Waiting for incoming data...");
  ASSERT(ed247_wait_frame(ed247_context, nullptr, 10 * 1000 * 1000) == ED247_STATUS_SUCCESS);

  // Ask the library to fill all stream assistant with the received data.
  // Without this, all reads will return old signal values.
  ASSERT(stream_assistants_pop_samples(ed247_context) == ED247_STATUS_SUCCESS);

  // If you need more details like which streams have been received or their associated timestamps,
  // you have to use ed247_stream_assistant_pop_sample() instead (without 's').
  // **BUT WARNING**, as explained in sender/main.cpp, dis_signal1_assistant and dis_signal2_assistant
  // are the same object. So ed247_stream_assistant_pop_sample() will return SUCCESS for the first one
  // but NO_DATA for the second one! An example of this more complex algo is provided below.


  // Check received data
  ASSERT(ed247_stream_assistant_read_signal(dis_signal1_assistant, dis_signal1, (const void**) &data, &size) == ED247_STATUS_SUCCESS);
  ASSERT(size == 1 && *data == dis_true);
  ASSERT(ed247_stream_assistant_read_signal(dis_signal3_assistant, dis_signal3, (const void**) &data, &size) == ED247_STATUS_SUCCESS);
  ASSERT(size == 1 && *data == dis_false);

  // dis_signal2 has not be written by sender. But it has been received since its part of stream1 (see
  // comments in sender/main.cpp). The library guaranty its value to be 0.
  ASSERT(ed247_stream_assistant_read_signal(dis_signal2_assistant, dis_signal2, (const void**) &data, &size) == ED247_STATUS_SUCCESS);
  ASSERT(size == 1 && *data == 0);
  SAY("Expected data has been received.");

  SYNC_SEND(ed247_context, 1);


  // =========================================================================
  // Demo 2 - Signal values are preserved
  LOG("------ [ DEMO 2 ] -------");
  SAY("Waiting for incoming data...");
  ASSERT(ed247_wait_frame(ed247_context, nullptr, 10 * 1000 * 1000) == ED247_STATUS_SUCCESS);

  // Read all data
  ASSERT(stream_assistants_pop_samples(ed247_context) == ED247_STATUS_SUCCESS);

  // Stream 1 has been sent and dis_signal2 has be written
  ASSERT(ed247_stream_assistant_read_signal(dis_signal2_assistant, dis_signal2, (const void**) &data, &size) == ED247_STATUS_SUCCESS);
  ASSERT(size == 1 && *data == dis_true);

  // Stream 1 has been sent but dis_signal1 has not be written. The last written value at be received again.
  ASSERT(ed247_stream_assistant_read_signal(dis_signal1_assistant, dis_signal1, (const void**) &data, &size) == ED247_STATUS_SUCCESS);
  ASSERT(size == 1 && *data == dis_true);

  // Stream 2 has not be sent. dis_signal3 kept it last received value.
  ASSERT(ed247_stream_assistant_read_signal(dis_signal3_assistant, dis_signal3, (const void**) &data, &size) == ED247_STATUS_SUCCESS);
  ASSERT(size == 1 && *data == dis_false);

  SAY("Expected data has been received.");

  SYNC_SEND(ed247_context, 2);


  // =========================================================================
  // Demo 3 - manual stream assistant push/pop, setting different data timestamp
  LOG("------ [ DEMO 3 ] -------");
  SAY("Waiting for incoming data...");
  ASSERT(ed247_wait_frame(ed247_context, nullptr, 10 * 1000 * 1000) == ED247_STATUS_SUCCESS);

  // The data timestamp and 'is received' status is associated to a sample of
  // a stream. We store this information to have it later when we will use signals.
  struct sample_info_t {
    const char*                   stream_name = nullptr;
    const ed247_timestamp_t*      dts         = nullptr;  // Data timestamp (aka acquisition timestamp)
    const ed247_timestamp_t*      rts         = nullptr;  // Receive timestamp
    const ed247_sample_details_t* details     = nullptr;  // contain Transport timestamp (aka send timestamp)
    bool                          received    = false;

    sample_info_t(ed247_stream_assistant_t assistant) {
      ed247_stream_t stream;
      ed247_stream_assistant_get_stream(assistant, &stream);
      stream_name = ed247_stream_get_name(stream);
    }

  };

  // Create a map of assistants.
  // Since dis_signal1_assistant and dis_signal2_assistant are the same object (they
  // belong to the same stream), the second 'emplace' will override the previous one.
  // In the end, it will be two objects in the map.
  std::unordered_map<ed247_stream_assistant_t, sample_info_t> assistant_infos;
  assistant_infos.emplace(dis_signal1_assistant, sample_info_t(dis_signal1_assistant));
  assistant_infos.emplace(dis_signal2_assistant, sample_info_t(dis_signal2_assistant));
  assistant_infos.emplace(dis_signal3_assistant, sample_info_t(dis_signal3_assistant));

  // Pop all assistants only once
  for (auto& assistant_info: assistant_infos) {
    ed247_stream_assistant_t assistant = assistant_info.first;
    sample_info_t& sample_info = assistant_info.second;
    ed247_status_t status = ed247_stream_assistant_pop_sample(assistant, &sample_info.dts, &sample_info.rts, &sample_info.details, nullptr);
    ASSERT(status != ED247_STATUS_FAILURE);
    sample_info.received = status == ED247_STATUS_SUCCESS;
    SAY("Popped stream " << sample_info.stream_name << ".");
  }

  // Display the timestamps
  SAY("");
  SAY("disc1 received: " << assistant_infos.at(dis_signal1_assistant).received);
  SAY("disc1 sent at : " << &(assistant_infos.at(dis_signal1_assistant).details->transport_timestamp));
  SAY("disc1 recv at : " << assistant_infos.at(dis_signal1_assistant).rts);
  SAY("disc1 DTS     : " << assistant_infos.at(dis_signal1_assistant).dts);

  SAY("");
  SAY("disc2 received: " << assistant_infos.at(dis_signal2_assistant).received);
  SAY("disc2 sent at : " << &(assistant_infos.at(dis_signal2_assistant).details->transport_timestamp));
  SAY("disc2 recv at : " << assistant_infos.at(dis_signal2_assistant).rts);
  SAY("disc2 DTS     : " << assistant_infos.at(dis_signal2_assistant).dts);

  SAY("");
  SAY("disc3 received: " << assistant_infos.at(dis_signal3_assistant).received);
  SAY("disc3 sent at : " << &(assistant_infos.at(dis_signal3_assistant).details->transport_timestamp));
  SAY("disc3 recv at : " << assistant_infos.at(dis_signal3_assistant).rts);
  SAY("disc3 DTS     : " << assistant_infos.at(dis_signal3_assistant).dts);

  ed247_unload(ed247_context);
  return 0;
}
