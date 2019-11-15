/******************************************************************************
 * The MIT Licence
 *
 * Copyright (c) 2019 Airbus Operations S.A.S
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

#ifdef __linux__
    #define MEMHOOKS_EXPORT __attribute__ ((visibility ("default")))
#elif _WIN32
    #ifdef MEMHOOKS_EXPORTS
        #define  MEMHOOKS_EXPORT __declspec(dllexport)
    #else
        #define  MEMHOOKS_EXPORT __declspec(dllimport)
    #endif
#endif

#include <stdint.h>
#include <stdlib.h>

#define ENV_MEMHOOKS_LEVEL "MEMHOOKS_LEVEL"

#ifdef __cplusplus
extern "C" {
#endif

#define _CrtDefaultAllocHook crt_alloc_hook;

/*********
 * Types *
 *********/

typedef enum
{
    MEMHOOKS_LEVEL_NONE,
    MEMHOOKS_LEVEL_COUNT,
    MEMHOOKS_LEVEL_BACKTRACE
} memhooks_level_t;

typedef enum
{
    MEMHOOKS_TYPE_MALLOC,
    MEMHOOKS_TYPE_FREE
} memhooks_type_t;

typedef struct memhooks_count_s
{
    unsigned long malloc_count;
    unsigned long free_count;
} memhooks_count_t;
#define _MEMHOOKS_COUNT_DEFAULT { 0, 0 }

/**
 * @brief Type of the backtrace callback
 * @param backtraces [in] the backtrace stack line array
 * @param size [in] the number of lines
 */
typedef void (*memhooks_backtrace_callback_t)(
    memhooks_type_t type,
    const memhooks_count_t *count,
    const char ** backtraces,
    int size);

/*************
 * Functions *
 *************/

/**
 * @brief Initialize the hooks
 */
extern MEMHOOKS_EXPORT void memhooks_initialize(memhooks_backtrace_callback_t callback);

/**
 * @brief Enable hooks
 */
extern MEMHOOKS_EXPORT void memhooks_enable(uint8_t enable);

/**
 * @brief Get hooks' enable state
 */
extern MEMHOOKS_EXPORT uint8_t memhooks_is_enabled();

/**
 * @brief Check if the hooks are enabled
 * @return Hook enable state
 */
extern MEMHOOKS_EXPORT memhooks_level_t memhooks_level();

/**
 * @brief Get the hooks count
 * @param count [out]
 */
extern MEMHOOKS_EXPORT void memhooks_get_count(memhooks_count_t *count);

/**
 * @brief Reset internal hook counter
 */
extern MEMHOOKS_EXPORT void memhooks_reset_count();

#ifdef __linux__

/*************
 * Overrides *
 *************/
extern MEMHOOKS_EXPORT void *malloc(size_t size);
extern MEMHOOKS_EXPORT void free(void *ptr);

#endif

#ifdef __cplusplus
}
#endif

#endif