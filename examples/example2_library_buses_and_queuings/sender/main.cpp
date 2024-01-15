#include <unistd.h>
#include "ed247.h"
#include "a429_tools.h"
#include "traces.h"
#include "sync.h"

#define ECIC EC_ROOT "ECIC.xml"

ed247_context_t ed247_context;

int main() {
  char afdx_data[400];
  a429_word_t a429_data;
  bool queue_full = false;

  SAY(VERSION);

  // Load context
  SAY("Load ECIC " ECIC);
  ASSERT(ed247_load_file(ECIC, &ed247_context) == ED247_STATUS_SUCCESS);

  // Get references to our streams from the context
  ed247_stream_t afdx_message1_stream;
  ASSERT(ed247_get_stream(ed247_context, "AFDX_MESSAGE1", &afdx_message1_stream) == ED247_STATUS_SUCCESS);
  ed247_stream_t afdx_message2_stream;
  ASSERT(ed247_get_stream(ed247_context, "AFDX_MESSAGE2", &afdx_message2_stream) == ED247_STATUS_SUCCESS);
  ed247_stream_t a429_bus_stream;
  ASSERT(ed247_get_stream(ed247_context, "A429_BUS1", &a429_bus_stream) == ED247_STATUS_SUCCESS);


  // The library provides information on the stream
  ASSERT(ed247_stream_get_type(afdx_message1_stream) == ED247_STREAM_TYPE_A664);
  ASSERT(ed247_stream_get_sample_max_size_bytes(afdx_message1_stream) == 500);
  ASSERT(ed247_stream_get_sample_max_number(afdx_message1_stream) == 10);

  // =========================================================================
  // Demo 1 - The behavior described here is generic to all streams (not specific to AFDX ones)
  // The AFDX_MESSAGE1 SampleMaxNumber is set to 10. We can push up to 10 messages before calling push.
  LOG("------ [ DEMO 1 ] -------");
  memset(afdx_data, 41, 100);
  ASSERT(ed247_stream_push_sample(afdx_message1_stream, afdx_data, 100, nullptr, &queue_full) == ED247_STATUS_SUCCESS);
  ASSERT(queue_full == false);

  memset(afdx_data, 42, 200);
  ASSERT(ed247_stream_push_sample(afdx_message1_stream, afdx_data, 200, nullptr, &queue_full) == ED247_STATUS_SUCCESS);
  ASSERT(queue_full == false);

  memset(afdx_data, 43, 300);
  ASSERT(ed247_stream_push_sample(afdx_message1_stream, afdx_data, 300, nullptr, &queue_full) == ED247_STATUS_SUCCESS);
  ASSERT(queue_full == false);

  // ed247_send_pushed_samples will create one stream with the 3 AFDX messages and sent it on the network.
  ASSERT(ed247_send_pushed_samples(ed247_context) == ED247_STATUS_SUCCESS);
  LOG("One stream sent with 3 AFDX messages.");

  SYNC_WAIT(ed247_context, 1);


  // =========================================================================
  // Demo 2 - The behavior described here is generic to all streams (not specific to AFDX ones)
  // The AFDX_MESSAGE2 SampleMaxNumber is set to 2. If we push more than 2 samples, older ones will be lost.
  LOG("------ [ DEMO 2 ] -------");

  // This first message will be discarded on the third push below
  memset(afdx_data, 41, 100);
  ASSERT(ed247_stream_push_sample(afdx_message2_stream, afdx_data, 100, nullptr, &queue_full) == ED247_STATUS_SUCCESS);
  ASSERT(queue_full == false);

  memset(afdx_data, 42, 200);
  ASSERT(ed247_stream_push_sample(afdx_message2_stream, afdx_data, 200, nullptr, &queue_full) == ED247_STATUS_SUCCESS);
  ASSERT(queue_full == true);
  LOG("After two push(), the AFDX2 queue is full.");

  memset(afdx_data, 43, 300);
  ASSERT(ed247_stream_push_sample(afdx_message2_stream, afdx_data, 300, nullptr, &queue_full) == ED247_STATUS_SUCCESS);
  ASSERT(queue_full == true);
  LOG("The third push() make the first AFDX to be discarded");

  // ed247_send_pushed_samples will create one stream with only the two last messages.
  ASSERT(ed247_send_pushed_samples(ed247_context) == ED247_STATUS_SUCCESS);
  LOG("One stream sent with 2 AFDX messages.");

  SYNC_WAIT(ed247_context, 2);


  // =========================================================================
  // Demo 3 - The behavior described here is generic to all streams (not specific to AFDX ones)
  // The AFDX_MESSAGE2 SampleMaxNumber is set to 2.
  // That limit the number of samples to 2 between two send(). But we can make several send().
  // This demo will send 3 streams, each one with 1 message.
  LOG("------ [ DEMO 3 ] -------");
  memset(afdx_data, 41, 100);
  ASSERT(ed247_stream_push_sample(afdx_message2_stream, afdx_data, 100, nullptr, &queue_full) == ED247_STATUS_SUCCESS);
  ASSERT(queue_full == false);
  ASSERT(ed247_send_pushed_samples(ed247_context) == ED247_STATUS_SUCCESS);
  LOG("stream 1 sent.");

  memset(afdx_data, 42, 200);
  ASSERT(ed247_stream_push_sample(afdx_message2_stream, afdx_data, 200, nullptr, &queue_full) == ED247_STATUS_SUCCESS);
  // The queue is not full because it has been emptied by the send.
  ASSERT(queue_full == false);
  ASSERT(ed247_send_pushed_samples(ed247_context) == ED247_STATUS_SUCCESS);
  LOG("stream 2 sent.");

  memset(afdx_data, 43, 300);
  ASSERT(ed247_stream_push_sample(afdx_message2_stream, afdx_data, 300, nullptr, &queue_full) == ED247_STATUS_SUCCESS);
  // The queue is not full because it has been emptied by the send.
  ASSERT(queue_full == false);
  ASSERT(ed247_send_pushed_samples(ed247_context) == ED247_STATUS_SUCCESS);
  LOG("stream 3 sent.");

  SYNC_WAIT(ed247_context, 3);


  // =========================================================================
  // Demo 4
  // An A429 stream vitalize an A429 bus. All samplings and queuings of the same bus have
  // to be pushed on the same stream and will be received on the same stream.
  // The SampleMaxSizeBytes is fixed to 4 - an A429 word - So all push shall have a
  // size of 4, including the queuings ones.
  // The behavior of SampleMaxNumber illustrated in the AFDX demos upside is exactly
  // the same for A429 streams. It will not be illustrated again for A429.
  //
  // ** WARNING **
  // The ED247 protocol and the library expect the A429 word payloads are in natural order.
  // If you store them as 'int' on a little-endian computer (x86, x64), your payload may
  // be inverted. Be sure to swap your words before calling push. example: htonl(my_int_word);
  // You may use a wrapper function around ed247_stream_push_sample to perform this task.
  //
  // The example below store A429 words as char[4] to avoid such issues.
  //
  LOG("------ [ DEMO 4 ] -------");

  // Push some sampling labels
  a429_set(a429_data, "140", A429_SDI_00, 41);
  ASSERT(ed247_stream_push_sample(a429_bus_stream, a429_data, 4, nullptr, nullptr) == ED247_STATUS_SUCCESS);

  a429_set(a429_data, "025", A429_SDI_01, 42);
  ASSERT(ed247_stream_push_sample(a429_bus_stream, a429_data, 4, nullptr, nullptr) == ED247_STATUS_SUCCESS);

  a429_set(a429_data, "124", A429_SDI_10, 43);
  ASSERT(ed247_stream_push_sample(a429_bus_stream, a429_data, 4, nullptr, nullptr) == ED247_STATUS_SUCCESS);

  // The library do not perform any check or treatment on pushed data.
  // If we push again the same label number/sdi, it will just be present two times in the stream.
  a429_set(a429_data, "140", A429_SDI_00, 44);
  ASSERT(ed247_stream_push_sample(a429_bus_stream, a429_data, 4, nullptr, nullptr) == ED247_STATUS_SUCCESS);

  LOG("4 A429 sampling pushed.");

  // Create a pseudo A429 queuing payload
  a429_label_number_t queuing_number = a429_label_encode("350");
  const uint32_t queuing_count = 5;
  a429_word_t queuing_data[queuing_count];
  uint32_t sample_sizes[queuing_count];
  for (uint32_t id = 0; id < queuing_count; id++) {
    sample_sizes[id] = sizeof(a429_word_t);
    memset(queuing_data[id], 50 + id, sample_sizes[id]);
    a429_label_set(queuing_data[id], queuing_number);
  }

  // The SampleMaxSizeBytes is fixed to 4. To push a queuing, we can:
  // - Loop on ed247_stream_push_sample() and push each word one by one.
  // - Use ed247_stream_push_sample*s*() to send an array of samples.
  //   Since this function allows to send samples of different sizes (for other protocols),
  //   we have to provide it the sizes of each samples.
  //   This function only perform the loop on ed247_stream_push_sample() for you.
  ASSERT(ed247_stream_push_samples(a429_bus_stream,
                                   queuing_data,         // Array of samples
                                   sample_sizes,         // Array of sample sizes
                                   queuing_count,        // Number of element in the two previous arrays
                                   nullptr, nullptr) == ED247_STATUS_SUCCESS);

  LOG("5 A429 queuing pushed.");

  ASSERT(ed247_send_pushed_samples(ed247_context) == ED247_STATUS_SUCCESS);
  LOG("A429 stream sent.");

  SYNC_WAIT(ed247_context, 4);

  // =========================================================================
  // Demo 5
  // The SampleMaxNumber of the A429 stream is set to 10.
  // Like for AFDX, if we push more that 10 words, the older ones will be lost.
  // The library do not try to perform think like 'dropping the older words with
  // the same label'. So pushing to much words will drop the first pushed ones,
  // regardless their label number.
  // Be sure to size your SampleMaxNumber correctly.
  LOG("------ [ DEMO 4 ] -------");

  // Push some sampling labels
  a429_set(a429_data, "140", A429_SDI_00, 41);
  ASSERT(ed247_stream_push_sample(a429_bus_stream, a429_data, 4, nullptr, nullptr) == ED247_STATUS_SUCCESS);

  a429_set(a429_data, "025", A429_SDI_01, 42);
  ASSERT(ed247_stream_push_sample(a429_bus_stream, a429_data, 4, nullptr, nullptr) == ED247_STATUS_SUCCESS);

  a429_set(a429_data, "124", A429_SDI_10, 43);
  ASSERT(ed247_stream_push_sample(a429_bus_stream, a429_data, 4, nullptr, nullptr) == ED247_STATUS_SUCCESS);

  // Push some queuing labels
  a429_label_set(a429_data, queuing_number);
  for (uint32_t id = 0; id < 6; id++) {
    ASSERT(ed247_stream_push_sample(a429_bus_stream, a429_data, 4, nullptr, nullptr) == ED247_STATUS_SUCCESS);
  }

  // We have pushed 9 label. The next push will completely fill the stream. queue_full will become true.
  ASSERT(ed247_stream_push_sample(a429_bus_stream, a429_data, 4, nullptr, &queue_full) == ED247_STATUS_SUCCESS);
  ASSERT(queue_full == true);

  // The next push will drop the first word pushed, the label 140. And this regardless we are pushing a queuing word.
  // The queue still full.
  ASSERT(ed247_stream_push_sample(a429_bus_stream, a429_data, 4, nullptr, &queue_full) == ED247_STATUS_SUCCESS);
  ASSERT(queue_full == true);

  // Pushing again the label 124, will drop the older pushed one, the label 025. And we will have two times
  // the label 124 in the stream queue.
  a429_set(a429_data, "124", A429_SDI_10, 43);
  ASSERT(ed247_stream_push_sample(a429_bus_stream, a429_data, 4, nullptr, nullptr) == ED247_STATUS_SUCCESS);


  ASSERT(ed247_send_pushed_samples(ed247_context) == ED247_STATUS_SUCCESS);
  LOG("A429 stream sent.");


  ed247_unload(ed247_context);
  return 0;
}
