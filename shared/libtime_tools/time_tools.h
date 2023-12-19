#ifndef _TIME_TOOLS_H_
#define _TIME_TOOLS_H_
#include <cstdint>

namespace time_tools {
  // Return a monotonic time
  uint64_t get_monotonic_time_us();

  // sleep some us
  void sleep_us(uint32_t duration_us);
}


#endif
