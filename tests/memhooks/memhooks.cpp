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

#include "memhooks_private.h"

extern "C" {

/**********
 * Locals *
 **********/

memhooks_backtrace_callback_t _backtrace_callback = NULL;

/**
 * @brief Hooks level
 */
memhooks_level_t _level = MEMHOOKS_LEVEL_NONE;

/**
 * @brief Enable hooks
 */
uint8_t _enable = 0;

/**
 * @brief Local enable state
 */
uint8_t _on = 0;

/**
 * @brief Auto print of backtrace state
 */
uint8_t _backtrace_enable = 0;

/**
 * @brief Backtrace print buffer
 */
char *_backtrace_buffer[BACKTRACE_BUFFER_SIZE];

/**
 * @brief Internal hook counter
 */
memhooks_count_t _counts = _MEMHOOKS_COUNT_DEFAULT;

/*************
 * Functions *
 *************/

void memhooks_initialize(memhooks_backtrace_callback_t callback)
{
#ifdef _MSC_VER
	char * strlevel;
	size_t len;
	_dupenv_s(&strlevel, &len, ENV_MEMHOOKS_LEVEL);
#else
    char * strlevel = getenv(ENV_MEMHOOKS_LEVEL);
#endif
    if(strlevel){
        _level = (memhooks_level_t)atoi(strlevel);
    }
    if(callback){
        _backtrace_callback = callback;
    }
}

void memhooks_enable(uint8_t enable)
{
    _enable = enable;
}

uint8_t memhooks_is_enabled()
{
    return _enable;
}

memhooks_level_t memhooks_level()
{
    return _level;
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

#ifdef __linux__
void memhooks_backtrace(memhooks_type_t type)
{
    int backtrace_size;
    char **backtrace_strings;
    
    backtrace_size = backtrace((void **)_backtrace_buffer,BACKTRACE_BUFFER_SIZE);
    backtrace_strings = backtrace_symbols((void * const *)_backtrace_buffer,backtrace_size);

    if(_backtrace_callback)
        (*_backtrace_callback)(type, (const memhooks_count_t*)&_counts, (const char **)backtrace_strings, backtrace_size);

    free(backtrace_strings);
}

void *malloc(size_t size)
{
    if(_enable && _level >= MEMHOOKS_LEVEL_COUNT && !_on){
        _on = 1;
        _counts.malloc_count++;
        if(_level >= MEMHOOKS_LEVEL_BACKTRACE && _backtrace_callback){
            memhooks_backtrace(MEMHOOKS_TYPE_MALLOC);
        }
        _on = 0;
    }
    return __libc_malloc(size);
}

void free(void *ptr)
{
    if(_enable && _level >= MEMHOOKS_LEVEL_COUNT && !_on && ptr != NULL){
        _on = 1;
        _counts.free_count++;
        if(_level >= MEMHOOKS_LEVEL_BACKTRACE && _backtrace_callback){
            memhooks_backtrace(MEMHOOKS_TYPE_FREE);
        }
        _on = 0;
    }
    return __libc_free(ptr);
}
#endif

}