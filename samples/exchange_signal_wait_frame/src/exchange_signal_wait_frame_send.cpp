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

#include <ed247.h>

#define ECIC_FILEPATH "../config/ecic_exchange_signal_wait_frame_send.xml"

#include "sync_entity.h"

#define SYNCER_ID_SRC SYNCER_ID_MASTER
#define SYNCER_ID_DST SYNCER_ID_SLAVE

int check_status(ed247_context_t context, ed247_status_t status);

int main(int argc, char *argv[])
{
    ed247_status_t              status;
    ed247_context_t             context;
    ed247_stream_list_t         streams;
    ed247_stream_t              stream;
    ed247_signal_list_t         signals;
    ed247_signal_t              signal;
    ed247_stream_assistant_t    assistant;

    sync_init(SYNCER_ID_SRC);

    sync_sync(SYNCER_ID_DST);

    // Library information
    fprintf(stdout,"# Implementation name:    %s\n",ed247_get_implementation_name());
    fprintf(stdout,"# Implementation version: %s\n",ed247_get_implementation_version());

    // Loading
    if(argc < 2){
        fprintf(stdout,"Missing the first argument, use default ECIC configuration filepath: %s\n",ECIC_FILEPATH);
        status = ed247_load(ECIC_FILEPATH, NULL, &context);
    }else{
        fprintf(stdout,"Using provided ECIC configuration filepath: %s\n",argv[1]);
        status = ed247_load(argv[1], NULL, &context);
    }
    if(check_status(context, status)) return EXIT_FAILURE;

    // Stream
    status = ed247_find_streams(context,"Stream",&streams);
    if(check_status(context,status)) return EXIT_FAILURE;
    status = ed247_stream_list_next(streams,&stream);
    if(check_status(context,status)) return EXIT_FAILURE;

    // Assistant
    status = ed247_stream_get_assistant(stream, &assistant);
    if(check_status(context,status)) return EXIT_FAILURE;

    // Signal
    status = ed247_find_stream_signals(stream,".*",&signals);
    if(check_status(context,status)) return EXIT_FAILURE;
    status = ed247_signal_list_next(signals,&signal);
    if(check_status(context,status)) return EXIT_FAILURE;

    void * signal_sample;
    size_t signal_sample_size;
    status = ed247_signal_allocate_sample(signal, &signal_sample, &signal_sample_size);
    if(check_status(context,status)) return EXIT_FAILURE;

    // Write & push signal samples
    uint32_t i;
    for(i = 0 ; i < 10 ; i++){
        *(uint8_t*)signal_sample = i % 2;
        // Update stream sample
        status = ed247_stream_assistant_write_signal(assistant, signal, signal_sample, signal_sample_size);
        if(check_status(context, status)) return EXIT_FAILURE;
        status = ed247_stream_assistant_push_sample(assistant, NULL, NULL);
        if(check_status(context, status)) return EXIT_FAILURE;
    }

    free(signal_sample);

    sync_wait(SYNCER_ID_DST);

    // Send them
    status = ed247_send_pushed_samples(context);
    if(check_status(context,status)) return EXIT_FAILURE;

    // Unload
    status = ed247_signal_list_free(signals);
    if(check_status(context, status)) return EXIT_FAILURE;
    status = ed247_stream_list_free(streams);
    if(check_status(context, status)) return EXIT_FAILURE;
    status = ed247_unload(context);
    if(check_status(context,status)) return EXIT_FAILURE;

    sync_stop();

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