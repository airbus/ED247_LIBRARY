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
#include "memhooks.h"
#include <iostream>

// Enable this for debugging
//#define MEMHOOKS_BACKTRACE_ENDABLED

#define BACKTRACE_BUFFER_SIZE 4096

typedef enum
{
  MEMHOOKS_TYPE_MALLOC,
  MEMHOOKS_TYPE_FREE
} memhooks_type_t;


memhooks_count_t _counts = { 0, 0 };
static uint8_t memhook_enabled = 0;

void memhooks_enable(uint8_t enable)
{
  memhook_enabled = enable;
}

uint8_t memhooks_is_enabled()
{
  return memhook_enabled;
}

void memhooks_get_count(memhooks_count_t *count)
{
  if(count){
    count->malloc_count = _counts.malloc_count;
    count->free_count = _counts.free_count;
  }
}

void memhooks_reset_count()
{
  _counts.malloc_count = _counts.free_count = 0;
}

//
// Backtrace tools (linux specific)
//
#ifdef __linux__
#include <execinfo.h>
__attribute__((unused)) static void memhooks_backtrace(memhooks_type_t type)
{
#ifdef MEMHOOKS_BACKTRACE_ENDABLED
  static char *_backtrace_buffer[BACKTRACE_BUFFER_SIZE];
  int backtrace_size;
  char **backtrace_strings;

  backtrace_size = backtrace((void **)_backtrace_buffer,BACKTRACE_BUFFER_SIZE);
  backtrace_strings = backtrace_symbols((void * const *)_backtrace_buffer,backtrace_size);

  std::cerr << "[BT] " << ((type == MEMHOOKS_TYPE_MALLOC)? "MALLOC" : "FREE") << " backtrace:" << std::endl;
  for(int i = 0 ; i < backtrace_size ; i++){
    std::cerr << "[BT] " << backtrace_strings[i] << std::endl;
  }
  std::cerr << "[BT] END" << std::endl;

  free(backtrace_strings);
#endif
}
#endif


//
// malloc/free catch (linux specific)
//
#ifdef __linux__
extern "C" {
  static uint8_t memhook_recusive_lock = 0;

  extern void* __libc_malloc(size_t size);
  extern void __libc_free(void* ptr);

  void *malloc(size_t size)
  {
    if(memhook_enabled && !memhook_recusive_lock){
      memhook_recusive_lock = 1;
      _counts.malloc_count++;
      memhooks_backtrace(MEMHOOKS_TYPE_MALLOC);
      memhook_recusive_lock = 0;
    }
    return __libc_malloc(size);
  }

  void free(void *ptr)
  {
    if(memhook_enabled && !memhook_recusive_lock && ptr != NULL){
      memhook_recusive_lock = 1;
      _counts.free_count++;
      memhooks_backtrace(MEMHOOKS_TYPE_FREE);
      memhook_recusive_lock = 0;
    }
    return __libc_free(ptr);
  }
}
#endif

