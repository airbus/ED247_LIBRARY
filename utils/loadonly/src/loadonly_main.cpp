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

#include <stdio.h>
#include <string>

#include <ed247.h>
#include <ed247_logs.h>

int check_status(ed247_context_t context, ed247_status_t status);

int main(int argc, char *argv[])
{
    ed247_status_t              status = ED247_STATUS_SUCCESS;
    ed247_log_level_t           log_level = ED247_LOG_LEVEL_UNSET;
    ed247_context_t             context = nullptr;
    
    std::string filepath = "";

    // Retrieve arguments
    if(argc != 2){
        PRINT_ERROR("loadonly <ecic_filepath>");
        return EXIT_FAILURE;
    }

    status = ed247_get_log_level(&log_level);
    if(status != ED247_STATUS_SUCCESS) return EXIT_FAILURE;

    filepath = std::string(argv[1]);
    PRINT_INFO("ECIC filepath: " << filepath);

    status = ed247_load_file(filepath.c_str(), &context);
    if(check_status(context, status)) return EXIT_FAILURE;

    status = ed247_unload(context);
    if(check_status(context,status)) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int check_status(ed247_context_t context, ed247_status_t status)
{
    if(status != ED247_STATUS_SUCCESS){
       PRINT_ERROR("ED247 status: " << ed247_status_string(status));
      ed247_unload(context);
      return EXIT_FAILURE;
    }else{
      return EXIT_SUCCESS;
    }
}
