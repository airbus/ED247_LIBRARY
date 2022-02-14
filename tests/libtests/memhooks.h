/******************************************************************************
 * The MIT Licence
 *
 * Copyright (c) 2021 Airbus Operations S.A.S
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *****************************************************************************/
#ifndef _MEMHOOKS_H_
#define _MEMHOOKS_H_
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


  struct memhooks_count_t
  {
    unsigned long malloc_count;
    unsigned long free_count;
  };

  // Count malloc. Only on Linux (else return 0).
  extern void malloc_count_start();
  extern int malloc_count_stop();

  extern void memhooks_enable(uint8_t enable);
  extern uint8_t memhooks_is_enabled();
  extern void memhooks_get_count(memhooks_count_t *count);
  extern void memhooks_reset_count();

#ifdef __cplusplus
}
#endif

#endif
