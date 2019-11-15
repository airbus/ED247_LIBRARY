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

#ifndef _ED247_MEMHOOKS_H_
#define _ED247_MEMHOOKS_H_

#include "ed247_logs.h"
#include "memhooks.h"

namespace ed247
{

class MemoryHooksManager
{
    public:
        static MemoryHooksManager & getInstance() {
            static MemoryHooksManager instance;
            return instance;
        }
        static void backtrace(
            memhooks_type_t type,
            const memhooks_count_t *count,
            const char ** backtraces,
            int size);
        bool isEnabled() const;
        void setEnable(const bool & enable);

        void print_memory_counts();

        uint64_t get_malloc_count();
        uint64_t get_free_count();

    private:
        MemoryHooksManager();
        ~MemoryHooksManager();
};

}

#endif