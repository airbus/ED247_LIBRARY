/******************************************************************************
 * The MIT Licence
 *
 * Copyright (c) 2020 Airbus Operations S.A.S
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

#ifndef _PROBE_HOOKS_PRIVATE_H_
#define _PROBE_HOOKS_PRIVATE_H_

#include "memhooks.h"

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#ifndef _MSC_VER
    #include <unistd.h>
#endif
#ifdef __linux__
    #include <unistd.h>
    #include <execinfo.h>
    #include <sys/types.h>
    #include <sys/syscall.h>
#endif

extern "C" {

/**
 * @brief The size of the buffer to print backtrace
 */
#define BACKTRACE_BUFFER_SIZE 4096

#ifdef __linux__

/************************************
 * GLIBC internal builtin functions *
 ************************************/
extern void* __libc_malloc(size_t size);
extern void __libc_free(void* ptr);

#endif

}

#endif