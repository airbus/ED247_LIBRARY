#include "sync.h"

#define SYNC_FAIL(m) do                           \
  {                                               \
    LOG("SYNC[" << step_id << "] " <<  m);        \
    return false;                                 \
  } while (0)


static ed247_stream_t sync_stream = nullptr;
static bool load_stream(ed247_context_t context, int step_id) {
  if (sync_stream == nullptr) {
    if (ed247_get_stream(context, "SYNC", &sync_stream) != ED247_STATUS_SUCCESS) {
      SYNC_FAIL("Cannot load 'SYNC' stream !");
    }
  }
  return true;
}

bool sync_wait(ed247_context_t context, int step_id) {
  char* data = nullptr;
  uint32_t size = 0;

  if (load_stream(context, step_id) == false) return false;

  if (ed247_wait_frame(context, nullptr, 10 * 1000 * 1000) != ED247_STATUS_SUCCESS) {
    SYNC_FAIL("Timeout while waiting SYNC message !");
  }

  if (ed247_stream_pop_sample(sync_stream, (const void**)&data, &size, nullptr, nullptr, nullptr, nullptr) != ED247_STATUS_SUCCESS) {
    SYNC_FAIL("SYNC message not received !");
  }

  if (size != 4 || *data != step_id) {
    SYNC_FAIL("Invalid SYNC message received ! (size: " << size << " id: " << step_id << ")");
  }

  return true;
}

bool sync_send(ed247_context_t context, int step_id) {
  if (load_stream(context, step_id) == false) return false;

  if (ed247_stream_push_sample(sync_stream, &step_id, 4, nullptr, nullptr) != ED247_STATUS_SUCCESS) {
    SYNC_FAIL("Cannot push SYNC message !");
  }

  if(ed247_send_pushed_samples(context) != ED247_STATUS_SUCCESS) {
    SYNC_FAIL("Cannot send SYNC message !");
  }

  return true;
}
