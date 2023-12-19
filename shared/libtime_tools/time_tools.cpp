#include "time_tools.h"
#include <time.h>
#ifdef _WIN32
# include <windows.h>
#endif

uint64_t time_tools::get_monotonic_time_us()
{
#ifdef __unix__
  struct timespec tp;
  clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
  return ((uint64_t)tp.tv_sec) * 1000000LL + ((uint64_t)tp.tv_nsec) / 1000LL;
#else
  LARGE_INTEGER s_frequency;
  BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
  if (s_use_qpc) {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (1000000LL * now.QuadPart) / s_frequency.QuadPart;
  } else {
    return GetTickCount() * 1000LL;
  }
#endif
}


void time_tools::sleep_us(uint32_t duration_us)
{
#ifdef _WIN32
    Sleep(duration_us / 1000);
#else
    struct timespec ts;
    ts.tv_sec = duration_us / (1000 * 1000);
    ts.tv_nsec = (duration_us % (1000 * 1000)) * 1000;
    nanosleep(&ts, NULL);
#endif
}
