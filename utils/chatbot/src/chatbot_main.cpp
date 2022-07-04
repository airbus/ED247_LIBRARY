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
#include <list>

#include <ed247.h>
#include "ed247_logs.h"
#include "sync_entity.h"
#include "string.h"

struct Stream;

struct Signal {

    ~Signal(){
        if(sample)free(sample);
    }

    Stream *stream;
    ed247_signal_t signal;
    const ed247_signal_info_t *info;
    void *sample = nullptr;
    size_t sample_size = 0;
};

struct Stream {

    ~Stream(){
        if(sample)free(sample);
        std::list<Signal*>::iterator it;
        for(it = signals.begin(); it != signals.end(); it++) delete *it;
    }

    ed247_stream_t stream;
    ed247_stream_assistant_t assistant;
    const ed247_stream_info_t *info;
    void *sample = nullptr;
    size_t sample_size = 0;
    std::list<Signal*> signals;
};

int check_status(ed247_context_t context, ed247_status_t status);

int main(int argc, char *argv[])
{
    ed247_status_t              status = ED247_STATUS_SUCCESS;
    ed247_log_level_t           log_level = ED247_LOG_LEVEL_UNSET;
    ed247_context_t             context = nullptr;
    ed247_stream_list_t         streams = nullptr;
    ed247_stream_t              stream = nullptr;
    ed247_signal_list_t         signals = nullptr;
    ed247_signal_t              signal = nullptr;
    ed247_stream_assistant_t    assistant = nullptr;
    std::list<Stream*>          list;
    
    std::string filepath = "";
    uint32_t timestep_ms = 0;
    uint64_t loop_count = 0;

    // Retrieve arguments
    if(argc != 4){
        PRINT_ERROR("chatbot <ecic_filepath> <timestep_ms> <loop_count>");
        return EXIT_FAILURE;
    }

    status = ed247_get_log_level(&log_level);
    if(status != ED247_STATUS_SUCCESS) return EXIT_FAILURE;

    filepath = std::string(argv[1]);
    PRINT_INFO("ECIC filepath: " << filepath);
    timestep_ms = atoi(argv[2]);
    PRINT_INFO("Timestep: " << timestep_ms << " ms");
    loop_count = atol(argv[3]);
    PRINT_INFO("Loop count: " << loop_count);

    status = ed247_load_file(filepath.c_str(), &context);
    if(check_status(context, status)) return EXIT_FAILURE;

    // Retrieve streams
    status = ed247_find_streams(context, NULL, &streams);
    if(check_status(context, status)) return EXIT_FAILURE;
    Stream *st;
    Signal *si;
    while(ed247_stream_list_next(streams, &stream) == ED247_STATUS_SUCCESS && stream != NULL){
        st = new Stream();
        st->stream = stream;
        status = ed247_stream_get_info(stream, &st->info);
        PRINT_INFO("Initialize stream [" << std::string(st->info->name) << "]");
        if(check_status(context,status)) return EXIT_FAILURE;
        status = ed247_stream_allocate_sample(st->stream, &st->sample, &st->sample_size);
        if(check_status(context,status)) return EXIT_FAILURE;
        status = ed247_stream_get_assistant(stream, &assistant);
        if(status == ED247_STATUS_SUCCESS){
            st->assistant = assistant;
            status = ed247_stream_find_signals(stream, NULL, &signals);
            if(check_status(context,status)) return EXIT_FAILURE;
            while(ed247_signal_list_next(signals, &signal) == ED247_STATUS_SUCCESS && signal != NULL){
                si = new Signal();
                si->stream = st;
                si->signal = signal;
                status = ed247_signal_get_info(signal, &si->info);
                if(check_status(context,status)) return EXIT_FAILURE;
                PRINT_INFO("Initialize stream [" << std::string(st->info->name) << "] / signal [" << std::string(si->info->name) << "]");
                status = ed247_signal_allocate_sample(si->signal, &si->sample, &si->sample_size);
                if(check_status(context,status)) return EXIT_FAILURE;
                st->signals.push_back(si);
            }
        }
        list.push_back(st);
    }

    uint64_t start, stop;
    ed247_timestamp_t timestamp;
    timestamp.epoch_s = 0;
    timestamp.offset_ns = 0;
    std::list<Stream*>::iterator it;
    std::list<Signal*>::iterator itsig;
    for(uint64_t loop = 0 ; loop < loop_count ; loop++){
        start = synchro::get_time_us();
        timestamp.epoch_s += (timestamp.offset_ns+1) >= 1000*1000 ? 1 : 0;
        timestamp.offset_ns = (timestamp.offset_ns+1) >= 1000*1000 ? timestamp.offset_ns + 1 : 0;
        for(it = list.begin() ; it != list.end() ; it++){
            st = *it;
            if(st->info->direction == ED247_DIRECTION_IN) continue;
            PRINT_INFO("Update stream [" << std::string(st->info->name) << "]");
            for(uint32_t i = 0 ; i < st->info->sample_max_number ; i++){
                uint32_t value = i + loop;
                PRINT_INFO("Update stream [" << std::string(st->info->name) << "] / sample [" << i << "]");
                if(st->signals.empty()){
                    if (st->sample_size == 4) {
                      *(uint32_t*)st->sample = value;
                    } else {
                      memset(st->sample, value, st->sample_size);
                    }

                    PRINT_INFO("Update stream [" << std::string(st->info->name) << "] / sample [" << i << "]: push [" << value << "]");
                    status = ed247_stream_push_sample(st->stream, st->sample, st->sample_size, &timestamp, NULL);
                    if(check_status(context,status)) return EXIT_FAILURE;
                }else{
                    for(itsig = st->signals.begin() ; itsig != st->signals.end() ; itsig++){
                        si = *itsig;
                        if (si->sample_size == 4) {
                          *(uint32_t*)si->sample = value;
                        } else {
                          memset(si->sample, value, si->sample_size);
                        }
                        PRINT_INFO("Update stream [" << std::string(st->info->name) << "] / sample [" << i << "] / signal [" << std::string(si->info->name) << "]: push [" << value << "]");
                        status = ed247_stream_assistant_write_signal(st->assistant, si->signal, si->sample, si->sample_size);
                        if(check_status(context,status)) return EXIT_FAILURE;
                    }
                    status = ed247_stream_assistant_push_sample(st->assistant, &timestamp, NULL);
                    if(check_status(context,status)) return EXIT_FAILURE;
                }
            }
        }
        status = ed247_send_pushed_samples(context);
        if(check_status(context,status)) return EXIT_FAILURE;
        stop = synchro::get_time_us();
        PRINT_INFO("Elapsed [" << stop-start << "] us / Timestep [" << timestep_ms*1000 << "] us");
        if((uint32_t)(stop-start) < timestep_ms*1000) synchro::sleep_us(timestep_ms*1000-(stop-start));
    }

    // Unload
    if (signals) {
      status = ed247_signal_list_free(signals);
      if(check_status(context, status)) return EXIT_FAILURE;
    }
    if (streams) {
      status = ed247_stream_list_free(streams);
      if(check_status(context, status)) return EXIT_FAILURE;
    }
    if (context) {
      status = ed247_unload(context);
      if(check_status(context,status)) return EXIT_FAILURE;
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
