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

#include "ed247.h"
#include "synchronizer.h"
#include <stdio.h>
#include <cstdlib>

#define ECIC_FILEPATH "../config/ecic_exchange_stream_event_send.xml"

#define SYNCER_ID_SRC 1
#define SYNCER_ID_DST 2

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

int check_status(ed247_context_t context, ed247_status_t status);

int main(int argc, char *argv[])
{
    ed247_status_t          status;
    ed247_context_t         context;
    ed247_stream_list_t     streams;
    ed247_stream_t          stream;

    synchro_init(SYNCER_ID_SRC);

    synchro_signal(SYNCER_ID_DST);

    // Library information
    fprintf(stdout, "# Implementation name:    %s\n", ed247_get_implementation_name());
    fprintf(stdout, "# Implementation version: %s\n", ed247_get_implementation_version());

    // Loading
    if(argc < 2){
        fprintf(stdout,"Missing the first argument, use default ECIC configuration filepath: %s\n",ECIC_FILEPATH);
        status = ed247_load_file(ECIC_FILEPATH, &context);
    }else{
        fprintf(stdout,"Using provided ECIC configuration filepath: %s\n",argv[1]);
        status = ed247_load_file(argv[1], &context);
    }
    if(check_status(context, status)) return EXIT_FAILURE;

    // Stream
    status = ed247_find_streams(context, "Stream", &streams);
    if(check_status(context, status)) return EXIT_FAILURE;
    status = ed247_stream_list_next(streams, &stream);
    if(check_status(context, status)) return EXIT_FAILURE;

    void * sample;
    uint32_t sample_size;
    status = ed247_stream_allocate_sample(stream, &sample, &sample_size);
    if(check_status(context,status)) return EXIT_FAILURE;

    // Write & push samples on the stream
    uint32_t i;
    for(i = 0 ; i < 10 ; i++){
        snprintf((char*)sample, sample_size, "Message: %u", i);
        // Push sample and send them if the stack is full. When sent, the internal buffer is released
        status = ed247_stream_push_sample(stream, sample, sample_size, NULL, NULL);
        if(check_status(context,status)) return EXIT_FAILURE;
    }

    free(sample);

    synchro_signal(SYNCER_ID_DST);

    // If the stream still contains samples, send them manually
    status = ed247_send_pushed_samples(context);
    if(check_status(context,status)) return EXIT_FAILURE;

    // Unload
    status = ed247_stream_list_free(streams);
    if(check_status(context,status)) return EXIT_FAILURE;
    status = ed247_unload(context);
    if(check_status(context,status)) return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int check_status(ed247_context_t context, ed247_status_t status)
{
    if(status != ED247_STATUS_SUCCESS){
      fprintf(stderr, "ED247 status: %s\n", ed247_status_string(status));
      ed247_unload(context);
      return EXIT_FAILURE;
    }else{
      return EXIT_SUCCESS;
    }
}
