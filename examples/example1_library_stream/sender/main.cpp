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

  // A sample is a bit of data that will be send to or received from an ED247 stream.
  // For AFDX, it correspond to one AFDX message.
  // The size of an AFDX sample shall be lower or equal to the SampleMaxSizeBytes defined in the ECIC
  const uint32_t afdx1_sample_size = 400;
  char afdx1_sample_data[afdx1_sample_size];

  for (uint32_t counter = 0; true; counter++) {
    // ED247 do not read the content of the streams
    // It will not check if you fill a valid AFDX message
    memset(afdx1_sample_data, counter, afdx1_sample_size);

    // Add a sample to afdx1_stream.
    // push_sample() will not send the sample on the network.
    // The sample will only be added to the afdx1_stream output queue.
    ASSERT(ed247_stream_push_sample(afdx1_stream,
                                    afdx1_sample_data, afdx1_sample_size,
                                    nullptr,                  // We do not date the sample
                                    nullptr                   // We needn't to know the output queue status
                                    ) == ED247_STATUS_SUCCESS);

    // Send all the pushed samples on the network
    // In our case, only one AFDX sample has been pushed and will be sent
    ASSERT(ed247_send_pushed_samples(ed247_context) == ED247_STATUS_SUCCESS);
    SAY("Sample " << counter << " pushed.");

    // Wait next cycle
    usleep(1000 * 1000);
  }

  // Free all resources linked to this context.
  // all object and pointers provided by the library related to this context become invalid.
  ed247_unload(ed247_context);

  return 0;
}
