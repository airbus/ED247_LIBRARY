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

#include "ed247.h"

#define ECIC_FILEPATH "../config/ecic_exchange_stream_event_recv.xml"

#include "sync_entity.h"

#define SYNCER_ID_SRC SYNCER_ID_SLAVE
#define SYNCER_ID_DST SYNCER_ID_MASTER

int check_status(ed247_context_t context, ed247_status_t status);
int process_streams(ed247_context_t context, ed247_stream_list_t streams);
ed247_status_t stream_receive(ed247_context_t context, ed247_stream_t stream);
ed247_status_t stream_receive_callback(ed247_context_t context, ed247_stream_t stream);

int processed = 0;

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
        status = ed247_load(ECIC_FILEPATH, NULL, &context);
    }else{
        fprintf(stdout,"Using provided ECIC configuration filepath: %s\n",argv[1]);
        status = ed247_load(argv[1], NULL, &context);
    }
    if(check_status(context, status)) return EXIT_FAILURE;

    // // Mode 0 : Register a callback triggered when new samples are available in a given stream
    status = ed247_register_recv_callback(context, &stream_receive_callback);
    if(check_status(context, status)) return EXIT_FAILURE;

    // Mode 1 : Wait for the first frame to be received
    // status = ed247_wait_frame(context, &streams, 30000000);
    // if(check_status(context, status)) return EXIT_FAILURE;
    // if(process_streams(context, streams)) return EXIT_FAILURE;
    // status = ed247_stream_list_free(streams);
    // if(check_status(context, status)) return EXIT_FAILURE;

    // // Mode 2 : Wait until the timeout is reached and process all received frame in the mean time
    // status = ed247_wait_during(context, &streams, 10000000);
    // if(check_status(context, status)) return EXIT_FAILURE;
    // if(process_streams(context, streams)) return EXIT_FAILURE;
    // status = ed247_stream_list_free(streams);
    // if(check_status(context, status)) return EXIT_FAILURE;

    sync_send(SYNCER_ID_DST);

    // Emulate a sleep function
    ed247_wait_frame(context, &streams, 10000000);

    // Unload
    status = ed247_unregister_recv_callback(context, &stream_receive_callback);
    if(check_status(context, status)) return EXIT_FAILURE;

    // Check if the event callback has been called
    if(processed != 1){
        fprintf(stderr,"# ED247 DATA NOT PROCESSED !\n");
        return EXIT_FAILURE;
    }
    
    status = ed247_unload(context);
    if(check_status(context, status)) return EXIT_FAILURE;

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
    bool empty = false;
    const void * sample;
    size_t sample_size;

    do{
        status = ed247_stream_pop_sample(stream, &sample, &sample_size, NULL, NULL, NULL, &empty);
        if(check_status(context, status) != ED247_STATUS_SUCCESS) return status;

        // Process sample
        // ...

    }while(!empty);

    return ED247_STATUS_SUCCESS;
}

ed247_status_t stream_receive_callback(ed247_context_t context, ed247_stream_t stream)
{
    _UNUSED(context);
    ed247_status_t status;
    bool empty = false;
    const void * sample;
    size_t sample_size;

    do{

        fprintf(stderr,"# POP SAMPLE\n");

        status = ed247_stream_pop_sample(stream, &sample, &sample_size, NULL, NULL, NULL, &empty);
        if(status == ED247_STATUS_FAILURE){
            fprintf(stderr,"# ERROR\n");
            return status;
        }

        // Process sample
        // ...
        processed = 1;

    }while(!empty);
    return status;
}