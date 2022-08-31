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
#include <inttypes.h>
#include <stdlib.h>

#include <ed247.h>

#define ECIC_FILEPATH "../config/ecic_walk_configuration.xml"

int check_status(ed247_context_t context, ed247_status_t status);
ed247_status_t dump_component(ed247_context_t context);
ed247_status_t dump_channel(ed247_channel_t channel);
ed247_status_t dump_stream(ed247_stream_t stream);
ed247_status_t dump_signal(ed247_signal_t signal);

// The prupose of this example is to show how to walk through the configuration tree of elements only.
// This example do not contain data exchange loops.

int main(int argc, char* argv[])
{
    ed247_status_t          status;
    ed247_context_t         context;
    ed247_channel_list_t    channels;
    ed247_channel_t         channel;
    ed247_stream_list_t     streams;
    ed247_stream_t          stream;
    ed247_signal_list_t     signals;
    ed247_signal_t          signal;

    // Library information
    fprintf(stdout,"# Implementation name:    %s\n",ed247_get_implementation_name());
    fprintf(stdout,"# Implementation version: %s\n",ed247_get_implementation_version());

    // Loading
    if(argc < 2){
        fprintf(stdout,"Missing the first argument, use default ECIC configuration filepath: %s\n",ECIC_FILEPATH);
        status = ed247_load_file(ECIC_FILEPATH, &context);
    }else{
        fprintf(stdout,"Using provided ECIC configuration filepath: %s\n",argv[1]);
        status = ed247_load_file(argv[1], &context);
    }
    if(check_status(context,status)) return EXIT_FAILURE;

    // Component
    if(dump_component(context)) return EXIT_FAILURE;

    // Channels
    status = ed247_find_channels(context,".*",&channels);
    if(check_status(context,status)) return EXIT_FAILURE;

    while(ed247_channel_list_next(channels,&channel) == ED247_STATUS_SUCCESS && channel != NULL){

        // Channel
        if(dump_channel(channel)) return EXIT_FAILURE;

        // Streams
        status = ed247_channel_find_streams(channel,".*",&streams);
        if(check_status(context,status)) return EXIT_FAILURE;

        while(ed247_stream_list_next(streams,&stream) == ED247_STATUS_SUCCESS && stream != NULL){

            // Stream
            if(dump_stream(stream)) return EXIT_FAILURE;

            switch(ed247_stream_get_type(stream)){
                case ED247_STREAM_TYPE_DISCRETE:
                case ED247_STREAM_TYPE_ANALOG:
                case ED247_STREAM_TYPE_NAD:
                case ED247_STREAM_TYPE_VNAD:

                    // Signals
                    status = ed247_stream_find_signals(stream,".*",&signals);
                    if(check_status(context,status)) return EXIT_FAILURE;

                    while(ed247_signal_list_next(signals,&signal) == ED247_STATUS_SUCCESS && signal != NULL){

                        // Signal
                        if(dump_signal(signal)) return EXIT_FAILURE;

                    }

                    status = ed247_signal_list_free(signals);
                    if(check_status(context,status)) return EXIT_FAILURE;

                    break;
                default:
                    break;
            }

        }

        status = ed247_stream_list_free(streams);
        if(check_status(context,status)) return EXIT_FAILURE;
        
    }

    // Unload
    status = ed247_channel_list_free(channels);
    if(check_status(context,status)) return EXIT_FAILURE;
    status = ed247_unload(context);
    if(check_status(context,status)) return EXIT_FAILURE;

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

ed247_status_t dump_component(ed247_context_t context)
{
    fprintf(stdout,"# ED247 Component\n\
Name: %s\n\
Type: %s\n\
Idenfitier: %"PRIu16"\n\
Standard: %s\n\
Revision: %s\n",
            ed247_component_get_name(context),
            ed247_component_type_string(ed247_component_get_type(context)),
            ed247_component_get_identifier(context),
            ed247_standard_string(ed247_component_get_standard_revision(context)),
            ed247_component_get_comment(context));

    return ED247_STATUS_SUCCESS;
}

ed247_status_t dump_channel(ed247_channel_t channel)
{
    fprintf(stdout,"## ED247 Channel\n\
Name: %s\n\
Comment: %s\n\
FrameFormat/StandardRevision: %s\n",
            ed247_channel_get_name(channel),
            ed247_channel_get_comment(channel),
            ed247_standard_string(ed247_channel_get_frame_standard_revision(channel))
      );

    return ED247_STATUS_SUCCESS;
}

ed247_status_t dump_stream(ed247_stream_t stream)
{
    fprintf(stdout,"### ED247 Stream\n\
Name: %s\n\
UID: %"PRIu16"\n\
ICD: %s\n\
Comment: %s\n",
            ed247_stream_get_name(stream),
            ed247_stream_get_uid(stream),
            ed247_stream_get_icd(stream),
            ed247_stream_get_comment(stream));

    return ED247_STATUS_SUCCESS;
}

ed247_status_t dump_signal(ed247_signal_t signal)
{
  if(ed247_signal_get_type(signal) == ED247_SIGNAL_TYPE_DISCRETE){
    fprintf(stdout,"#### ED247 DISCRETE Signal\n\
Name: %s\n\
ICD: %s\n\
ByteOffset: %u\n\
Comment: %s\n",
            ed247_signal_get_name(signal),
            ed247_signal_get_icd(signal),
            ed247_signal_get_byte_offset(signal),
            ed247_signal_get_comment(signal));
  }else if(ed247_signal_get_type(signal) == ED247_SIGNAL_TYPE_ANALOG){
    fprintf(stdout,"#### ED247 ANALOG Signal\n\
Name: %s\n\
ICD: %s\n\
ElectricalUnit: %s\n\
ByteOffset: %u\n\
Comment: %s\n",
            ed247_signal_get_name(signal),
            ed247_signal_get_icd(signal),
            ed247_signal_analogue_get_electrical_unit(signal),
            ed247_signal_get_byte_offset(signal),
            ed247_signal_get_comment(signal));
  }else if(ed247_signal_get_type(signal) == ED247_SIGNAL_TYPE_NAD){
    fprintf(stdout,"#### ED247 NAD Signal\n\
Name: %s\n\
ICD: %s\n\
Unit: %s\n\
Type: %s\n\
ByteOffset: %u\n\
Comment: %s\n",
            ed247_signal_get_name(signal),
            ed247_signal_get_icd(signal),
            ed247_signal_nad_get_unit(signal),
            ed247_nad_type_string(ed247_signal_nad_get_type(signal)),
            ed247_signal_get_byte_offset(signal),
            ed247_signal_get_comment(signal));
    uint32_t i = 0;
    fprintf(stdout,"Dimensions:");
    for(i = 0;i < ed247_signal_nad_get_dimensions_count(signal) ; i++){
      fprintf(stdout," %u",ed247_signal_nad_get_dimension(signal, i));
    }
  }else if(ed247_signal_get_type(signal) == ED247_SIGNAL_TYPE_VNAD){
    fprintf(stdout,"#### ED247 VNAD Signal\n\
Name: %s\n\
ICD: %s\n\
Unit: %s\n\
Type: %s\n\
MaxNumber: %u\n\
Comment: %s\n",
            ed247_signal_get_name(signal),
            ed247_signal_get_icd(signal),
            ed247_signal_nad_get_unit(signal),
            ed247_nad_type_string(ed247_signal_nad_get_type(signal)),
            ed247_signal_vnad_get_max_number(signal),
            ed247_signal_get_comment(signal));
    }else{
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
