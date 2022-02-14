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
#include <fstream>
#include <iomanip>

#include <ed247.h>
#include "ed247_logs.h"
#include "sync_entity.h"

int check_status(ed247_context_t context, ed247_status_t status);

void help() {
  std::cout <<
    "USAGE: dumper <ecic_file> <output_file> <timeout_ms>" << std::endl <<
    "       dumper <ecic_file> <timeout_ms>                # Dump on STDOUT" << std::endl;
}

std::ofstream output_stream;
std::ostream& output() { return output_stream.is_open() ? output_stream : std::cout; }

int main(int argc, char *argv[])
{
    ed247_status_t              status = ED247_STATUS_SUCCESS;
    ed247_context_t             context = nullptr;
    ed247_stream_list_t         streams = nullptr;
    ed247_stream_t              stream = nullptr;
    const ed247_stream_info_t   *stream_info = nullptr;
    const void                  *sample = nullptr;
    size_t                      sample_size = 0;
    const ed247_timestamp_t     *data_timestamp = nullptr;
    const ed247_timestamp_t     *recv_timestamp = nullptr;
    const ed247_sample_info_t   *info = nullptr;

    std::string ecic_file = std::string();
    std::string output_file = std::string();
    int timeout_arg = 0;

    // Retrieve arguments
    if (argc == 3) {
      timeout_arg = 2;
    } else if (argc == 4) {
      output_stream.open(argv[2]);
      if (output_stream.is_open()) {
        SAY("Dump to file " << argv[2]);
      } else {
        PRINT_ERROR("Cannot open file '" << argv[2] << "'");
      }
      timeout_arg = 3;
    } else {
      help();
      return EXIT_FAILURE;
    }

    ecic_file = std::string(argv[1]);
    status = ed247_load_file(ecic_file.c_str(), &context);
    if(check_status(context, status)) return EXIT_FAILURE;
    PRINT_INFO("ECIC file : '" << ecic_file << "'");

    char* last;
    uint32_t timeout_ms = strtol(argv[timeout_arg], &last, 10);
    if (*last){
      PRINT_ERROR("Invalid timout argument: '" << argv[timeout_arg] << "'");
      return EXIT_FAILURE;
    }
    PRINT_INFO("Timeout : '" << timeout_ms << "'");


    output() << "ComponentIdentifier;"
        << "SequenceNumber;"
        << "TransportTimestampEpochS;"
        << "TransportTimestampOffsetNs;"
        << "Stream;"
        << "ReceiveTimestampEpochS;"
        << "ReceiveTimestampOffsetNs;"
        << "DataTimestampEpochS;"
        << "DataTimestampOffsetNs;"
        << "StreamData;"
        << "Signal;"
        << "SignalData"
        << std::endl;

    uint64_t start, stop;
    start = synchro::get_time_us();
    int32_t timeout_us = timeout_ms*1000;
    do {
        stop = synchro::get_time_us();
        PRINT_DEBUG("Timeout: " << timeout_us << " " << timeout_us - (stop-start));
        status = ed247_wait_frame(context, &streams, timeout_us - (stop-start));
        if(status == ED247_STATUS_SUCCESS){
            // Process streams
            while(ed247_stream_list_next(streams, &stream) == ED247_STATUS_SUCCESS && stream != NULL){
                status = ed247_stream_get_info(stream, &stream_info);
                if(check_status(context, status)) return status;
                if(stream_info->type == ED247_STREAM_TYPE_A664 ||
                    stream_info->type == ED247_STREAM_TYPE_A429 ||
                    stream_info->type == ED247_STREAM_TYPE_A825 ||
                    stream_info->type == ED247_STREAM_TYPE_M1553 ||
                    stream_info->type == ED247_STREAM_TYPE_SERIAL ||
                    stream_info->type == ED247_STREAM_TYPE_AUDIO ||
                    stream_info->type == ED247_STREAM_TYPE_VIDEO ||
                    stream_info->type == ED247_STREAM_TYPE_ETHERNET){
                    status = ed247_stream_pop_sample(stream, &sample, &sample_size, &data_timestamp, &recv_timestamp, &info, NULL);
                    if(check_status(context, status)) return status;
                    output() << info->component_identifier << ";"
                        << info->sequence_number << ";"
                        << info->transport_timestamp.epoch_s << ";"
                        << info->transport_timestamp.offset_ns << ";"
                        << std::string(stream_info->name) << ";"
                        << data_timestamp->epoch_s << ";"
                        << data_timestamp->offset_ns << ";"
                        << recv_timestamp->epoch_s << ";"
                        << recv_timestamp->offset_ns << ";";
                    for(size_t i = 0 ; i < sample_size ; i++){
                        if(i > 0) output() << " ";
                        output() << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)((unsigned char*)sample)[i];
                    }
                    output() << std::dec << ";";
                    output() << "" << ";" << "" << std::endl; // Signal & SignalData columns
                }else if(stream_info->type == ED247_STREAM_TYPE_ANALOG ||
                    stream_info->type == ED247_STREAM_TYPE_DISCRETE ||
                    stream_info->type == ED247_STREAM_TYPE_NAD ||
                    stream_info->type == ED247_STREAM_TYPE_VNAD){
                    ed247_stream_assistant_t assistant;
                    status = ed247_stream_get_assistant(stream, &assistant);
                    if(check_status(context, status)) return status;
                    status = ed247_stream_assistant_pop_sample(assistant, &data_timestamp, &recv_timestamp, &info, NULL);
                    if(check_status(context, status)) return status;
                    ed247_signal_list_t signals;
                    ed247_signal_t signal;
                    status = ed247_stream_get_signals(stream, &signals);
                    if(check_status(context, status)) return status;
                    while(ed247_signal_list_next(signals, &signal) == ED247_STATUS_SUCCESS && signal != NULL){
                        const ed247_signal_info_t   *signal_info;
                        status = ed247_signal_get_info(signal, &signal_info);
                        if(check_status(context, status)) return status;
                        const void * signal_sample;
                        size_t signal_sample_size;
                        status = ed247_stream_assistant_read_signal(assistant, signal, &signal_sample, &signal_sample_size);
                        if(check_status(context, status)) return status;
                        output() << info->component_identifier << ";"
                            << info->sequence_number << ";"
                            << info->transport_timestamp.epoch_s << ";"
                            << info->transport_timestamp.offset_ns << ";"
                            << std::string(stream_info->name) << ";"
                            << data_timestamp->epoch_s << ";"
                            << data_timestamp->offset_ns << ";"
                            << recv_timestamp->epoch_s << ";"
                            << recv_timestamp->offset_ns << ";"
                            << "" << ";"
                            << std::string(signal_info->name) << ";"; // Stream data
                        for(size_t i = 0 ; i < signal_sample_size ; i++){
                            if(i > 0) output() << " ";
                            output() << std::hex << std::setfill('0') << std::setw(2) << (unsigned int)((unsigned char*)signal_sample)[i];
                        }
                        output() << std::dec << std::endl;
                    }
                }
            }
        }
    }while(status != ED247_STATUS_FAILURE && status != ED247_STATUS_TIMEOUT);

    status = ed247_unload(context);
    if(check_status(context,status)) return EXIT_FAILURE;

    if (output_stream.is_open()) {
      output_stream.close();
    }

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
