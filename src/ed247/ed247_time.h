/* -*- mode: c++; c-basic-offset: 2 -*-  */
#ifndef ED247_TIME_H
#define ED247_TIME_H
#include <stdint.h>

// Note: most time function are defined in ed247.h

namespace ed247 {
  uint64_t get_monotonic_time_us();
  void sleep_us(uint32_t duration_us);
}


#endif
