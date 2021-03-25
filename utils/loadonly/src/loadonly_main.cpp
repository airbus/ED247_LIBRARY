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

#include <stdio.h>
#include <iostream>
#include <string>

#include <ed247.h>

int check_status(ed247_context_t context, ed247_status_t status);

int main(int argc, char *argv[])
{
    ed247_status_t              status;
    ed247_log_level_t           log_level;
    ed247_context_t             context;
    
    std::string filepath = "";

    // Retrieve arguments
    if(argc != 2){
        std::cerr << "loadonly <ecic_filepath>" << std::endl;
        return EXIT_FAILURE;
    }

    status = ed247_get_log_level(&log_level);
    if(status != ED247_STATUS_SUCCESS) return EXIT_FAILURE;

    filepath = std::string(argv[1]);
    if(log_level >= ED247_LOG_LEVEL_INFO) std::cout << "ECIC filepath: " << filepath << std::endl;

    status = ed247_load(filepath.c_str(), NULL, &context);
    if(check_status(context, status)) return EXIT_FAILURE;

    status = ed247_unload(context);
    if(check_status(context,status)) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int check_status(ed247_context_t context, ed247_status_t status)
{
    if(status != ED247_STATUS_SUCCESS){
        fprintf(stderr,"# ED247 ERROR (%s): %s\n",
            ed247_status_string(status),
            libed247_errors());
        ed247_unload(context);
        return EXIT_FAILURE;
    }else{
        return EXIT_SUCCESS;
    }
}