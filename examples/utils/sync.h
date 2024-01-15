//
// Really basic API to synchronize examples
// A 'SYNC' message has to be defined in ECIC (see example2)
//
#ifndef __SYNC_H__
#define __SYNC_H__
#include "traces.h"
#include "ed247.h"

#define SYNC_WAIT(context, step_id) do                  \
  {                                                     \
    if (sync_wait(context, step_id) == true) {          \
      LOG("SYNC[" << step_id << "] Sync received.");    \
    } else {                                            \
      LOG("SYNC[" << step_id << "] Sync failed !");     \
      abort();                                          \
    }                                                   \
  } while (0)

#define SYNC_SEND(context, step_id) do                  \
  {                                                     \
    if (sync_send(context, step_id) == true) {          \
      LOG("SYNC[" << step_id << "] Sync send.");        \
    } else {                                            \
      LOG("SYNC[" << step_id << "] Sync failed !");     \
      abort();                                          \
    }                                                   \
  } while (0)


bool sync_wait(ed247_context_t context, int step_id);
bool sync_send(ed247_context_t context, int step_id);


#endif
