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

#include "ed247.h"

#define ECIC_FILEPATH "../config/ecic_exchange_signal_wait_frame_recv.xml"

#include "sync_entity.h"

#define SYNCER_ID_SRC SYNCER_ID_SLAVE
#define SYNCER_ID_DST SYNCER_ID_MASTER

int check_status(ed247_context_t context, ed247_status_t status);
int process_streams(ed247_context_t context, ed247_stream_list_t streams);
ed247_status_t stream_receive(ed247_context_t context, ed247_stream_t stream);
ed247_status_t stream_receive_callback(ed247_context_t context, ed247_stream_t stream);

int main(int argc, char *argv[])
{
    ed247_status_t          status;
    ed247_context_t         context;
    ed247_stream_list_t     streams;

    sync_init(SYNCER_ID_SRC);

    sync_sync(SYNCER_ID_DST);

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

    // Mode 0 : Register a callback triggered when new samples are available in a given stream
    // status = ed247_register_recv_callback(context, &stream_receive_callback);
    // if(check_status(context, status)) return EXIT_FAILURE;

    sync_send(SYNCER_ID_DST);

    // Mode 1 : Wait for the first frame to be received
    status = ed247_wait_frame(context, &streams, 10000000);

    if(check_status(context, status)) return EXIT_FAILURE;
    if(process_streams(context, streams)) return EXIT_FAILURE;
    status = ed247_stream_list_free(streams);
    if(check_status(context, status)) return EXIT_FAILURE;

    // Mode 2 : Wait until the timeout is reached and process all received frame in the mean time
    // status = ed247_wait_during(context, &streams, 10000000);
    // if(check_status(context, status)) return EXIT_FAILURE;
    // if(process_streams(context, streams)) return EXIT_FAILURE;
    // status = ed247_stream_list_free(streams);
    // if(check_status(context, status)) return EXIT_FAILURE;

    // Unload
    // status = ed247_unregister_recv_callback(context, &stream_receive_callback);
    // if(check_status(context, status)) return EXIT_FAILURE;

    status = ed247_unload(context);
    if(check_status(context,status)) return EXIT_FAILURE;

    sync_stop();

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

int process_streams(ed247_context_t context, ed247_stream_list_t streams)
{
    ed247_status_t status;
    ed247_stream_t stream;

    while(ed247_stream_list_next(streams, &stream) == ED247_STATUS_SUCCESS && stream != NULL){
        status = stream_receive(context, stream);
        if(check_status(context, status)) return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

ed247_status_t stream_receive(ed247_context_t context, ed247_stream_t stream)
{
    ed247_status_t status;
    ed247_signal_t signal;
    ed247_signal_list_t signals;
    bool empty = false;
    const void * signal_sample;
    size_t signal_sample_size;
    ed247_stream_assistant_t assistant;

    status = ed247_stream_get_assistant(stream, &assistant);
    if(check_status(context,status)) return status;

    status = ed247_find_stream_signals(stream,".*",&signals);
    if(check_status(context,status)) return status;

    do{
        status = ed247_stream_assistant_pop_sample(assistant, NULL, NULL, NULL, &empty);
        if(check_status(context, status)) return status;

        while(ed247_signal_list_next(signals, &signal) == ED247_STATUS_SUCCESS && signal != NULL){
            status = ed247_stream_assistant_read_signal(assistant, signal, &signal_sample, &signal_sample_size);
            if(check_status(context, status)) return status;

            // Process signal sample
            // ...

        }

    }while(!empty);

    return ED247_STATUS_SUCCESS;
}

ed247_status_t stream_receive_callback(ed247_context_t context, ed247_stream_t stream)
{
    ed247_status_t status;
    ed247_signal_t signal;
    ed247_signal_list_t signals;
    bool empty = false;
    const void * signal_sample;
    size_t signal_sample_size;
    ed247_stream_assistant_t assistant;

    status = ed247_stream_get_assistant(stream, &assistant);
    if(status == ED247_STATUS_FAILURE) return status;

    status = ed247_find_stream_signals(stream,".*",&signals);
    if(status == ED247_STATUS_FAILURE) return status;

    do{
        status = ed247_stream_assistant_pop_sample(assistant, NULL, NULL, NULL, &empty);
        if(status == ED247_STATUS_FAILURE) return status;

        while(ed247_signal_list_next(signals, &signal) == ED247_STATUS_SUCCESS && signal != NULL){
            status = ed247_stream_assistant_read_signal(assistant, signal, &signal_sample, &signal_sample_size);
            if(status == ED247_STATUS_FAILURE) return status;

            // Process signal sample
            // ...

        }

    }while(!empty);
    return status;
}
