#include <unistd.h>
#include "ed247.h"
#include "traces.h"

#define ECIC EC_ROOT "ECIC.xml"

int main() {
  SAY(VERSION);

  SAY("Load ECIC " ECIC);

  // The ed247_context is an abstract object that contain all information related to one ECIC.
  // It is created from an ECIC file.
  ed247_context_t ed247_context;
  ASSERT(ed247_load_file(ECIC, &ed247_context) == ED247_STATUS_SUCCESS);

  // Get references to our streams from the context
  ed247_stream_t afdx1_stream;
  ASSERT(ed247_get_stream(ed247_context, "AFDX_MESSAGE1", &afdx1_stream) == ED247_STATUS_SUCCESS);

  // You needn't to allocate memory for receiving samples: the library will provide you
  // a pointer to its internal memory (that prevent a copy). DO NOT free this pointer.
  // If you need to modify the data, you have to make a copy.
  const char* afdx1_sample_data = nullptr;
  uint32_t afdx1_sample_size;

  do {
    // The wait_during will read all data from network.
    // It will read all data already received and all those that will be received during duration_us.
    ed247_wait_during(ed247_context,
                      nullptr,        // This list of received stream is not easy to use
                      1000 * 1000);   // The call will always block during one second

    // Pop a sample from the afdx1_stream input queue.
    // This sample is remove from the queue.
    ed247_status_t status =
      ed247_stream_pop_sample(afdx1_stream,
                              (const void**)&afdx1_sample_data, &afdx1_sample_size,
                              nullptr, nullptr, nullptr,  // We do not need timestamps
                              nullptr);                   // We needn't to know the input queue status

    if (status == ED247_STATUS_SUCCESS) {
      // We have received new a sample
      ASSERT(afdx1_sample_size == 400);
      SAY("New sample received. Value: " << (uint32_t) (afdx1_sample_data[0]));
    }
    else if (status == ED247_STATUS_NODATA) {
      // We do not have receive a new sample
      // sample content is invalid
      ASSERT(afdx1_sample_data == nullptr);
      ASSERT(afdx1_sample_size == 0);
      SAY("No data has been received.");
    }
    else {
      FAIL("Unexpected ed247_stream_pop_sample() status: " << ed247_status_string(status));
    }

  } while (true);

  // Free all resources linked to this context.
  // all object and pointers provided by the library related to this context become invalid.
  ed247_unload(ed247_context);

  return 0;
}
