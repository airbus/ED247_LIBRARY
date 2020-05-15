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

#include "ed247_memhooks.h"

#ifdef __linux__
    #include <execinfo.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/syscall.h>
#endif

namespace ed247
{

MemoryHooksManager::MemoryHooksManager()
{
    // Memhooks initialization
    memhooks_initialize(&(MemoryHooksManager::backtrace));
}

void MemoryHooksManager::backtrace(
    memhooks_type_t type,
    const memhooks_count_t *count,
    const char ** backtraces,
    int size)
{
    int i;

    LOG_DEBUG() << "### MEMHOOKS - BACKTRACE ###" << LOG_END;

    #ifdef __linux__
        LOG_DEBUG() << "# PID [" << std::setw(8) << (int)getpid() << "] " <<
            "PPID [" << std::setw(8) << (int)getppid() << "] " <<
            "TID [" << std::setw(8) << (int)syscall(SYS_gettid) << "]" << LOG_END;
    #endif

    for(i = 0 ; i < size ; i++){
        LOG_DEBUG() << "# " << backtraces[i] << LOG_END;
    }

    LOG_DEBUG() << "###" << LOG_END;
}

MemoryHooksManager::~MemoryHooksManager()
{
    print_memory_counts();
}

bool MemoryHooksManager::isEnabled() const
{
    return memhooks_is_enabled() == 1;
}

void MemoryHooksManager::setEnable(const bool & enable)
{
    memhooks_enable((uint8_t)enable);
}

void MemoryHooksManager::print_memory_counts()
{
    static memhooks_count_t count;
    memhooks_get_count(&count);

    LOG_DEBUG() << "### MEMHOOKS - COUNT ###" << LOG_END;
    LOG_DEBUG() << "# Malloc: " << count.malloc_count << LOG_END;
    LOG_DEBUG() << "# Free:   " << count.free_count << LOG_END;
    LOG_DEBUG() << "###" << LOG_END;

}

uint64_t MemoryHooksManager::get_malloc_count()
{
    static memhooks_count_t count;
    memhooks_get_count(&count);
    return count.malloc_count;
}

uint64_t MemoryHooksManager::get_free_count()
{
    static memhooks_count_t count;
    memhooks_get_count(&count);
    return count.free_count;
}

}