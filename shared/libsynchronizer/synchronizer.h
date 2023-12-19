//
// Synchronize actors with UDP messages
//
#ifndef _SYNCHRONIZER_H_
#define _SYNCHRONIZER_H_
#include <cstdint>

// This API need an UDP port per actor
// An actor receive from (BASE_PORT + actor_id)
#define SYNCHRO_DEFAULT_BASE_PORT (uint16_t) 13000

// Currently, this API does not allows actors to be on separate hosts
#define SYNCHRO_DEFAULT_HOST_IP  (const char*) "127.0.0.1"

// Wait/Ack timeout
#define SYNCHRO_TIMEOUT_MS (uint32_t) 5000

// Initialize the API
extern void synchro_init(uint32_t actor_id, const char* host_ip = SYNCHRO_DEFAULT_HOST_IP, uint16_t base_port = SYNCHRO_DEFAULT_BASE_PORT);

// Send a signal to target_actor_id and wait for its acknowledgment
// The sync use an internal counter: all signals had to be waited
// user_message is only used for traces. It needn't to match wait user_message.
// caller_file/caller_line can be used to display caller file/line instead of sync api file/line
extern void synchro_signal(uint32_t target_actor_id, const char* user_message = nullptr, const char* caller_file = nullptr, uint32_t caller_line = 0);

// Wait from_actor_id send a signal and acknowledge it.
// The sync use an internal counter: all signals had to be waited
// user_message is only used for traces. It needn't to match send user_message.
// caller_file/caller_line can be used to display caller file/line instead of sync api file/line
extern void synchro_wait(uint32_t from_actor_id, const char* user_message = nullptr, const char* caller_file = nullptr, uint32_t caller_line = 0);

#endif
