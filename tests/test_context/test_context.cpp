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

#include "test_context.h"
#include "sync_entity.h"
#ifdef __unix__
    #include "memhooks.h"
#endif

#ifdef _WIN32
    #include <windows.h>
#endif

#ifdef __unix__
void backtrace(
    memhooks_type_t type,
    const memhooks_count_t *count,
    const char ** backtraces,
    int size)
{
    (void)type;
    (void)count;
    int i;

    std::cout << "### BACKTRACE" << std::endl;

    for(i = 0 ; i < size ; i++){
        std::cout << "# " << backtraces[i] << std::endl;
    }

    std::cout << "###" << std::endl;
}
#endif

void memhooks_section_start()
{
    std::cout << "### REAL TIME SECTION [START]" << std::endl;
#ifdef __unix__
    memhooks_initialize(&backtrace);
    memhooks_reset_count();
    memhooks_enable(true);
#endif
}

bool memhooks_section_stop()
{
#ifdef __unix__
    memhooks_enable(false);
#endif
    std::cout << "### REAL TIME SECTION [STOP]" << std::endl;
#ifdef __unix__
    memhooks_count_t count;
    memhooks_get_count(&count);
    std::cout << "### MEMHOOKS - COUNT ###" << std::endl;
    std::cout << "# Malloc: " << count.malloc_count << std::endl;
    std::cout << "# Free:   " << count.free_count << std::endl;
    std::cout << "###" << std::endl;
    // return count.malloc_count == 0 && count.free_count == 0;
    return count.malloc_count == 0;
#else
    return true;
#endif
}