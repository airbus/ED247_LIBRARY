#include <unistd.h>
#include "ed247.h"
#include "a429_tools.h"
#include "traces.h"
#include "sync.h"

#define ECIC EC_ROOT "ECIC.xml"

ed247_context_t ed247_context;

int main() {
  const char* afdx_data = nullptr;
  a429_word_const_ptr_t a429_data = nullptr;
  uint32_t sample_size = 0;
  bool queue_empty = true;

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


  // =========================================================================
  // Demo 1 - The behavior described here is generic to all streams (not specific to AFDX ones)
  // The emitter has send an AFDX stream with 3 messages
  // But the receiver has a SampleMaxNumber set to 2.
  // So the first ADFX message (the older one) is lost.
  LOG("------ [ DEMO 1 ] -------");
  ASSERT(ed247_stream_get_sample_max_number(afdx_message1_stream) == 2);

  // In order to synchronize with the sender, we wait for the first incoming frame.
  LOG("Waiting for incoming data ...");
  ASSERT(ed247_wait_frame(ed247_context, nullptr, 10 * 1000 * 1000) == ED247_STATUS_SUCCESS);
  LOG("A frame has been received.");

  // Receive the second AFDX message
  ASSERT(ed247_stream_pop_sample(afdx_message1_stream, (const void**)&afdx_data, &sample_size,
                                 nullptr, nullptr, nullptr,
                                 &queue_empty)
         == ED247_STATUS_SUCCESS);
  ASSERT(sample_size == 200);
  ASSERT(afdx_data[0] == 42);
  LOG("AFDX 2 received.");

  // It still a message in the queue
  ASSERT(queue_empty == false);

  // Receive the third AFDX message
  ASSERT(ed247_stream_pop_sample(afdx_message1_stream, (const void**)&afdx_data, &sample_size,
                                 nullptr, nullptr, nullptr,
                                 &queue_empty)
         == ED247_STATUS_SUCCESS);
  ASSERT(sample_size == 300);
  ASSERT(afdx_data[0] == 43);
  LOG("AFDX 3 received.");

  // The queue is now empty
  ASSERT(queue_empty == true);
  ASSERT(ed247_stream_pop_sample(afdx_message1_stream, (const void**)&afdx_data, &sample_size,
                                 nullptr, nullptr, nullptr,
                                 &queue_empty)
         == ED247_STATUS_NODATA);
  LOG("AFDX queue is empty.");

  SYNC_SEND(ed247_context, 1);

  // =========================================================================
  // Demo 2 - The behavior described here is generic to all streams (not specific to AFDX ones)
  // The emitter push 3 messages in a stream with a SampleMaxNumber of 2.
  // Only the 2 last messages has been sent.
  // The receiver has a SampleMaxNumber set to 10, it can receive up to 10 messages, but
  // one has been lost on the send side.
  LOG("------ [ DEMO 2 ] -------");
  ASSERT(ed247_stream_get_sample_max_number(afdx_message2_stream) == 10);

  ASSERT(ed247_wait_frame(ed247_context, nullptr, 10 * 1000 * 1000) == ED247_STATUS_SUCCESS);
  LOG("A frame has been received.");

  // Receive the second AFDX message
  ASSERT(ed247_stream_pop_sample(afdx_message2_stream, (const void**)&afdx_data, &sample_size,
                                 nullptr, nullptr, nullptr,
                                 &queue_empty)
         == ED247_STATUS_SUCCESS);
  ASSERT(sample_size == 200);
  ASSERT(afdx_data[0] == 42);
  LOG("AFDX 2 received.");

  // It still a message in the queue
  ASSERT(queue_empty == false);

  // Receive the third AFDX message
  ASSERT(ed247_stream_pop_sample(afdx_message2_stream, (const void**)&afdx_data, &sample_size,
                                 nullptr, nullptr, nullptr,
                                 &queue_empty)
         == ED247_STATUS_SUCCESS);
  ASSERT(sample_size == 300);
  ASSERT(afdx_data[0] == 43);
  LOG("AFDX 3 received.");

  // The queue is now empty
  ASSERT(queue_empty == true);
  ASSERT(ed247_stream_pop_sample(afdx_message2_stream, (const void**)&afdx_data, &sample_size,
                                 nullptr, nullptr, nullptr,
                                 &queue_empty)
         == ED247_STATUS_NODATA);
  ASSERT(afdx_data == nullptr);
  ASSERT(sample_size == 0);
  LOG("AFDX queue is empty.");

  SYNC_SEND(ed247_context, 2);

  // =========================================================================
  // Demo 3 - The behavior described here is generic to all streams (not specific to AFDX ones)
  // The emitter sent 3 time the same stream, each one with one message.
  // The receiver SampleMaxNumber is set to 10, this is enough to store these 3 messages.
  LOG("------ [ DEMO 3 ] -------");

  // We wait 1 second to let the time to the emitter send its 3 message (sleep-synchronization...)
  ASSERT(ed247_wait_during(ed247_context, nullptr, 1000 * 1000) == ED247_STATUS_SUCCESS);

  // Receive the first AFDX message
  ASSERT(ed247_stream_pop_sample(afdx_message2_stream, (const void**)&afdx_data, &sample_size,
                                 nullptr, nullptr, nullptr,
                                 &queue_empty)
         == ED247_STATUS_SUCCESS);
  ASSERT(sample_size == 100);
  ASSERT(afdx_data[0] == 41);
  LOG("AFDX 1 received.");

  // It still message(s) in the queue
  ASSERT(queue_empty == false);

  // Receive the second AFDX message
  ASSERT(ed247_stream_pop_sample(afdx_message2_stream, (const void**)&afdx_data, &sample_size,
                                 nullptr, nullptr, nullptr,
                                 &queue_empty)
         == ED247_STATUS_SUCCESS);
  ASSERT(sample_size == 200);
  ASSERT(afdx_data[0] == 42);
  LOG("AFDX 2 received.");

  // It still message(s) in the queue
  ASSERT(queue_empty == false);

  // Receive the third AFDX message
  ASSERT(ed247_stream_pop_sample(afdx_message2_stream, (const void**)&afdx_data, &sample_size,
                                 nullptr, nullptr, nullptr,
                                 &queue_empty)
         == ED247_STATUS_SUCCESS);
  ASSERT(sample_size == 300);
  ASSERT(afdx_data[0] == 43);
  LOG("AFDX 3 received.");

  // The queue is now empty
  ASSERT(queue_empty == true);
  ASSERT(ed247_stream_pop_sample(afdx_message2_stream, (const void**)&afdx_data, &sample_size,
                                 nullptr, nullptr, nullptr,
                                 &queue_empty)
         == ED247_STATUS_NODATA);
  ASSERT(afdx_data == nullptr);
  ASSERT(sample_size == 0);
  LOG("AFDX queue is empty.");

  SYNC_SEND(ed247_context, 3);

  // =========================================================================
  // Demo 4
  // The emitter send one stream with 9 A429 words
  LOG("------ [ DEMO 4 ] -------");
  ASSERT(ed247_wait_frame(ed247_context, nullptr, 10 * 1000 * 1000) == ED247_STATUS_SUCCESS);
  LOG("A frame has been received.");

  const a429_label_number_t LABEL_140 = a429_label_encode("140");
  const a429_label_number_t LABEL_025 = a429_label_encode("025");
  const a429_label_number_t LABEL_124 = a429_label_encode("124");
  const a429_label_number_t LABEL_350 = a429_label_encode("350");

  uint32_t label_received_count[256];
  memset(label_received_count, 0, sizeof(label_received_count));

  a429_word_t queuing_350_data[50];
  uint32_t queuing_350_size = 0;

  // The receiver receive the whole bus, and it don't know the labels order.
  // It has to sort label itself.
  ed247_status_t status = ED247_STATUS_FAILURE;
  do {
    status = ed247_stream_pop_sample(a429_bus_stream, (const void**)&a429_data, &sample_size,
                                     nullptr, nullptr, nullptr, nullptr);

    if (status == ED247_STATUS_SUCCESS) {
      a429_label_number_t received_number = a429_label_get(a429_data);

      label_received_count[received_number]++;

      if (received_number == LABEL_140) {
        LOG("Label 140 received");
        ASSERT(a429_sdi_get(a429_data) == A429_SDI_00);
      }

      else if (received_number == LABEL_025) {
        LOG("Label 025 received");
        ASSERT(a429_sdi_get(a429_data) == A429_SDI_01);
        ASSERT(a429_bnr_get(a429_data) == 42);
      }

      else if (received_number == LABEL_124) {
        LOG("Label 124 received");
        ASSERT(a429_sdi_get(a429_data) == A429_SDI_10);
        ASSERT(a429_bnr_get(a429_data) == 43);
      }

      else if (received_number == LABEL_350) {
        // for queuing, we rebuild the initial queuing buffer.
        LOG("Label 350 received");
        memcpy(queuing_350_data[queuing_350_size], a429_data, sizeof(a429_word_t));
        queuing_350_size++;
      }

      else {
        // Since we receive the whole bus, we may received labels that we don't know.
        // This is not an error: this label maybe consumed by another system.
        LOG("Unknown label received: " << a429_label_decode(received_number) << ".");
      }

    }
  } while (status == ED247_STATUS_SUCCESS);

  // We have received 2 times the 140, one time 025 and 124, and 5 times 350.
  ASSERT(label_received_count[LABEL_140] == 2);
  ASSERT(label_received_count[LABEL_025] == 1);
  ASSERT(label_received_count[LABEL_124] == 1);
  ASSERT(label_received_count[LABEL_350] == 5);
  LOG("We have received the expected count of A429 words.");

  // The content of the queuing is what has been sent
  for (uint32_t id = 0; id < queuing_350_size; id++) {
    ASSERT(queuing_350_data[id][2] == 50 + id);
  }
  LOG("We have received the expected queuing content.");

  SYNC_SEND(ed247_context, 4);

  // =========================================================================
  // Demo 5
  // labels 140 and 025 has been dropped by the emitter.
  // We receive the labels in pushed order, starting by the label 124.
  LOG("------ [ DEMO 5 ] -------");
  ASSERT(ed247_wait_frame(ed247_context, nullptr, 10 * 1000 * 1000) == ED247_STATUS_SUCCESS);
  LOG("A frame has been received.");


  ASSERT(ed247_stream_pop_sample(a429_bus_stream, (const void**)&a429_data, &sample_size,
                                 nullptr, nullptr, nullptr, &queue_empty) == ED247_STATUS_SUCCESS);
  ASSERT(a429_label_get(a429_data) == LABEL_124);
  LOG("Label 124 received.");
  ASSERT(queue_empty == false);

  for (uint32_t id = 0; id < 8; id++) {
    ASSERT(ed247_stream_pop_sample(a429_bus_stream, (const void**)&a429_data, &sample_size,
                                   nullptr, nullptr, nullptr, &queue_empty) == ED247_STATUS_SUCCESS);
    ASSERT(a429_label_get(a429_data) == LABEL_350);
    ASSERT(queue_empty == false);
  }
  LOG("Queuing 350 received.");

  // The label 124 is present two times
  ASSERT(ed247_stream_pop_sample(a429_bus_stream, (const void**)&a429_data, &sample_size,
                                 nullptr, nullptr, nullptr, &queue_empty) == ED247_STATUS_SUCCESS);
  ASSERT(a429_label_get(a429_data) == LABEL_124);
  LOG("Label 124 received.");

  // now, the queue is empty (pop return ED247_STATUS_NODATA)
  ASSERT(queue_empty == true);
  ASSERT(ed247_stream_pop_sample(a429_bus_stream, (const void**)&a429_data, &sample_size,
                                 nullptr, nullptr, nullptr, nullptr) == ED247_STATUS_NODATA);
  LOG("Incoming queue is empty");

  ed247_unload(ed247_context);
  return 0;
}
