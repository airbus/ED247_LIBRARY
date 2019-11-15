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

/************
 * Includes *
 ************/

#include <ed247_test.h>

#include <ed247_memhooks.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

/***********
 * Defines *
 ***********/

#define TEST_ENTITY_MASTER_ID 1
#define TEST_ENTITY_SLAVE_ID 2

#include <iostream>

#ifdef __linux__
    #include <sys/select.h>
    #include <sched.h>
    #include <sys/mman.h>
    #define SCHEDULER_PRIORITY 90
#endif

/********
 * Test *
 ********/

class PerfosStreamsContext : public TestContext {};
class PerfosSignalsContext : public TestContext {};

void display_results(uint64_t loop_count);

void com_send_callback();
void com_recv_callback();

typedef struct {
    uint64_t update;
    uint64_t send;
    uint64_t recv;
    uint64_t get;
    uint64_t all;
} timer_data_t;

timer_data_t * times;
unsigned loop;
int64_t counter;
uint64_t counter_send;
uint64_t counter_recv;
void *sample;
size_t sample_size;
uint64_t timer_start;
uint64_t timer_stop;
uint64_t tic;
uint64_t tac;

uint64_t signal_counter;
void signal_handler(int signo){
    signal_counter++;
}

typedef struct {
    ed247_stream_t stream;
    ed247_stream_assistant_t assistant;
    ed247_signal_t * signals;
    size_t signals_size;
} stream_t;

stream_t *streams_send;
size_t streams_send_size;
stream_t *streams_recv;
size_t streams_recv_size;

size_t channels_send_size;
size_t channels_recv_size;

bool fill_structures(ed247_context_t context, bool has_signals)
{
    ed247_status_t status;
    ed247_channel_list_t channels;
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    ed247_stream_assistant_t assistant;
    size_t istream;
    ed247_signal_list_t signals;
    ed247_signal_t signal;
    size_t isignal;

    // Channels SEND
    status = ed247_find_channels(context, "Channel_M2S_.*", &channels);
    if(status != ED247_STATUS_SUCCESS) return false;
    status = ed247_channel_list_size(channels, &channels_send_size);
    if(status != ED247_STATUS_SUCCESS) return false;
    std::cout << "# [SEND] # Channels [" << channels_send_size << "]" << std::endl;
    status = ed247_channel_list_free(channels);
    if(status != ED247_STATUS_SUCCESS) return false;

    // Channels RECV
    status = ed247_find_channels(context, "Channel_S2M_.*", &channels);
    if(status != ED247_STATUS_SUCCESS) return false;
    status = ed247_channel_list_size(channels, &channels_recv_size);
    if(status != ED247_STATUS_SUCCESS) return false;
    std::cout << "# [RECV] # Channels [" << channels_recv_size << "]" << std::endl;
    status = ed247_channel_list_free(channels);
    if(status != ED247_STATUS_SUCCESS) return false;
    
    // Streams SEND
    status = ed247_find_streams(context, "Stream_M2S_.*", &streams);
    if(status != ED247_STATUS_SUCCESS) return false;
    status = ed247_stream_list_size(streams, &streams_send_size);
    if(status != ED247_STATUS_SUCCESS) return false;
    std::cout << "# [SEND] ### Streams [" << streams_send_size << "]" << std::endl;
    streams_send = (stream_t*)malloc(sizeof(stream_t)*streams_send_size);
    istream = 0;
    while(ed247_stream_list_next(streams, &stream) == ED247_STATUS_SUCCESS && stream != NULL){
        streams_send[istream].stream = stream;
        if(has_signals){
            status = ed247_stream_get_assistant(stream, &assistant);
            if(status != ED247_STATUS_SUCCESS) return false;
            streams_send[istream].assistant = assistant;
            status = ed247_stream_get_signals(stream, &signals);
            if(status != ED247_STATUS_SUCCESS) return false;
            status = ed247_signal_list_size(signals, &streams_send[istream].signals_size);
            if(status != ED247_STATUS_SUCCESS) return false;
            std::cout << "# [SEND] ##### Signals [" << streams_send[istream].signals_size << "]" << std::endl;
            streams_send[istream].signals = (ed247_signal_t*)malloc(sizeof(ed247_signal_t)*streams_send[istream].signals_size);
            isignal = 0;
            while(ed247_signal_list_next(signals, &signal) == ED247_STATUS_SUCCESS && signal != NULL){
                streams_send[istream].signals[isignal] = signal;
                isignal++;
            }
            status = ed247_signal_list_free(signals);
            if(status != ED247_STATUS_SUCCESS) return false;
        }
        istream++;
    }
    status = ed247_stream_list_free(streams);
    if(status != ED247_STATUS_SUCCESS) return false;

    // Streams RECV
    status = ed247_find_streams(context, "Stream_S2M_.*", &streams);
    if(status != ED247_STATUS_SUCCESS) return false;
    status = ed247_stream_list_size(streams, &streams_recv_size);
    if(status != ED247_STATUS_SUCCESS) return false;
    std::cout << "# [RECV] ### Streams [" << streams_recv_size << "]" << std::endl;
    streams_recv = (stream_t*)malloc(sizeof(stream_t)*streams_recv_size);
    istream = 0;
    while(ed247_stream_list_next(streams, &stream) == ED247_STATUS_SUCCESS && stream != NULL){
        streams_recv[istream].stream = stream;
        if(has_signals){
            status = ed247_stream_get_assistant(stream, &assistant);
            if(status != ED247_STATUS_SUCCESS) return false;
            streams_recv[istream].assistant = assistant;
            status = ed247_stream_get_signals(stream, &signals);
            if(status != ED247_STATUS_SUCCESS) return false;
            status = ed247_signal_list_size(signals, &streams_recv[istream].signals_size);
            if(status != ED247_STATUS_SUCCESS) return false;
            std::cout << "# [RECV] ##### Signals [" << streams_recv[istream].signals_size << "]" << std::endl;
            streams_recv[istream].signals = (ed247_signal_t*)malloc(sizeof(ed247_signal_t)*streams_recv[istream].signals_size);
            isignal = 0;
            while(ed247_signal_list_next(signals, &signal) == ED247_STATUS_SUCCESS && signal != NULL){
                streams_recv[istream].signals[isignal] = signal;
                isignal++;
            }
            status = ed247_signal_list_free(signals);
            if(status != ED247_STATUS_SUCCESS) return false;
        }
        istream++;
    }
    status = ed247_stream_list_free(streams);
    if(status != ED247_STATUS_SUCCESS) return false;

    return true;
}

TEST_P(PerfosStreamsContext, PingPongStreams)
{
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    const ed247_stream_info_t *info;
    std::ostringstream oss;
    std::string msg;
    uint64_t loop_count;

    std::cout << "# FILEPATH [" << GetParam().filepath << "]" << std::endl;

    // Retrieve loop count
    std::string str_loop_count = test::get_env_variable("TEST_LOOP_COUNT");
    if(str_loop_count.empty()) str_loop_count = "10";
    std::istringstream iss(str_loop_count);
    iss >> loop_count;
    std::cout << "# LOOP COUNT [" << loop_count << "]" << std::endl;
    times = (timer_data_t*)malloc((size_t)loop_count*sizeof(timer_data_t));

#ifdef __linux__
    if(mlockall(MCL_CURRENT|MCL_FUTURE) != 0)
        std::cout << "WARNING: Executable shall be launched with administrator rights" << std::endl;
    struct sched_param param;
    param.sched_priority = SCHEDULER_PRIORITY;
    if(sched_setscheduler(0, SCHED_FIFO, &param) != 0)
        std::cout << "WARNING: Executable shall be launched with administrator rights" << std::endl;
#endif

    ASSERT_TRUE(fill_structures(_context, false));

    // Samples
    ASSERT_EQ(ed247_find_streams(_context, "Stream_M2S_1_1", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(stream != NULL);
    ASSERT_EQ(ed247_stream_get_info(stream, &info), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_allocate_sample(stream, &sample, &sample_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);

    // Callback
    // ASSERT_EQ(ed247_register_com_send_callback(_context, &com_send_callback), ED247_STATUS_SUCCESS);
    // ASSERT_EQ(ed247_register_com_recv_callback(_context, &com_recv_callback), ED247_STATUS_SUCCESS);

    // Checkpoint
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°1" << std::endl;
    TestSend(); TestWait();

    counter_send = counter_recv = 0;

#ifdef __linux__
    signal_counter = 0;
    struct sigaction s;
    s.sa_handler = SIG_IGN;
    ASSERT_EQ(sigaction(SIGPROF, &s, NULL), 0);
#endif

    for(loop = 0 ; loop < loop_count ; loop++)
    {
        // std::cout << "LOOP [" << loop << "]" << std::endl;

        // Prepare message
        oss.str("");
        oss << std::setw(info->sample_max_size_bytes) << std::setfill('0') << loop;
        msg = oss.str();
        memcpy(sample, msg.c_str(), info->sample_max_size_bytes);

        // Checkpoint
        // std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint [" << loop << "]" << std::endl;
        TestSend(); TestWait();

        memhooks_reset_count();
        memhooks_enable(true);

        // Start timer
        timer_start = test::get_time_us();
        tic = timer_start;

        // Update
        for(size_t istream = 0 ; istream < streams_send_size ; istream++){
            stream = streams_send[istream].stream;
            for(unsigned i = 0 ; i < info->sample_max_number ; i++){
                ASSERT_EQ(ed247_stream_push_sample(stream, sample, sample_size, NULL, NULL), ED247_STATUS_SUCCESS);
            }
        }

        tac = test::get_time_us();
        times[loop].update = tac - tic;

        // Send
        tic = tac;
        // std::cout << "SEND" << std::endl;
        ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
        // std::cout << "SEND: OK" << std::endl;
        
        // Recv
        counter = channels_recv_size;
        while(counter > 0){
            // std::cout << "WAIT" << std::endl;
            ASSERT_EQ(ed247_wait_frame(_context, &streams, 10000000), ED247_STATUS_SUCCESS)
                << "# LOOP [" << loop << "] RECV [" << counter_recv << "] SEND [" << counter_send << "]";
            // std::cout << "WAIT: OK" << std::endl;
            counter--;
        }
        
        // Retrieve data
        tac = test::get_time_us();
        times[loop].recv = tac - tic;
        tic = tac;
        for(size_t istream = 0 ; istream < streams_recv_size ; istream++){
            stream = streams_recv[istream].stream;
            bool empty = false;
            const void * recv_sample;
            size_t recv_sample_size;
            do{
                ASSERT_EQ(ed247_stream_pop_sample(stream, &recv_sample, &recv_sample_size, NULL, NULL, NULL, &empty),ED247_STATUS_SUCCESS);
                ASSERT_EQ(memcmp(sample, recv_sample, recv_sample_size), 0);
            }while(!empty);
        }

        // Stop timer
        timer_stop = test::get_time_us();
        tac = timer_stop;
        times[loop].get = tac - tic;
        times[loop].all = timer_stop - timer_start;

        memhooks_enable(false);
#ifdef __linux__
        memhooks_count_t count;
        memhooks_get_count(&count);
        ASSERT_EQ(count.malloc_count, 0);
#endif
    }
    
    // Checkpoint
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint °2" << std::endl;
    TestSend(); TestWait();

    // End
    TestSend();

    free(sample);

#ifdef __linux__
    param.sched_priority = 0;
    if(sched_setscheduler(0, SCHED_OTHER, &param) != 0)
        std::cout << "WARNING: Executable shall be launched with administrator rights" << std::endl;
#endif

    display_results(loop_count);

    // Callback
    // ASSERT_EQ(ed247_unregister_com_send_callback(_context, &com_send_callback), ED247_STATUS_SUCCESS);
    // ASSERT_EQ(ed247_unregister_com_recv_callback(_context, &com_recv_callback), ED247_STATUS_SUCCESS);
}

void *signal_sample;
size_t signal_sample_size;

TEST_P(PerfosSignalsContext, PingPongSignals)
{
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    ed247_stream_assistant_t assistant;
    ed247_signal_list_t signals;
    ed247_signal_t signal;
    const ed247_stream_info_t *info;
    std::ostringstream oss;
    std::string msg;
    uint64_t loop_count;

    std::cout << "# FILEPATH [" << GetParam().filepath << "]" << std::endl;

    std::string str_loop_count = test::get_env_variable("TEST_LOOP_COUNT");
    if(str_loop_count.empty()) str_loop_count = "10";
    std::istringstream iss(str_loop_count);
    iss >> loop_count;
    std::cout << "# LOOP COUNT [" << loop_count << "]" << std::endl;
    times = (timer_data_t*)malloc((size_t)loop_count*sizeof(timer_data_t));

#ifdef __linux__
    if(mlockall(MCL_CURRENT|MCL_FUTURE) != 0)
        std::cout << "WARNING: Executable shall be launched with administrator rights" << std::endl;
    struct sched_param param;
    param.sched_priority = SCHEDULER_PRIORITY;
    if(sched_setscheduler(0, SCHED_FIFO, &param) != 0)
        std::cout << "WARNING: Executable shall be launched with administrator rights" << std::endl;
#endif

    ASSERT_TRUE(fill_structures(_context, true));

    // Samples
    ASSERT_EQ(ed247_find_streams(_context, "Stream_M2S_1_1", &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    ASSERT_TRUE(stream != NULL);
    ASSERT_EQ(ed247_stream_get_info(stream, &info), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_allocate_sample(stream, &sample, &sample_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);

    // Sample
    ASSERT_EQ(ed247_find_signals(_context, "Signal_M2S_.*", &signals), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_signal_list_next(signals, &signal), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_signal_allocate_sample(signal, &signal_sample, &signal_sample_size), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_signal_list_free(signals), ED247_STATUS_SUCCESS);

    // Callback
    // ASSERT_EQ(ed247_register_recv_callback(_context, &signal_recv_callback), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_register_com_send_callback(_context, &com_send_callback), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_register_com_recv_callback(_context, &com_recv_callback), ED247_STATUS_SUCCESS);

    // Checkpoint
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°1" << std::endl;
    TestSend(); TestWait();

    counter_send = counter_recv = 0;

#ifdef __linux__
    signal_counter = 0;
    struct sigaction s;
    s.sa_handler = SIG_IGN;
    ASSERT_EQ(sigaction(SIGPROF, &s, NULL), 0);
#endif

    for(loop = 0 ; loop < loop_count ; loop++)
    {
        // std::cout << "LOOP [" << loop << "]" << std::endl;

        // Prepare message
        oss.str("");
        oss << std::setw(signal_sample_size) << std::setfill('0') << loop;
        msg = oss.str();
        memcpy(signal_sample, msg.c_str(), signal_sample_size);

        // Checkpoint
        // std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°" << loop << std::endl;
        TestSend(); TestWait();

        // test::sleep_us(1000);

        memhooks_reset_count();
        memhooks_enable(true);

        // Start timer
        timer_start = test::get_time_us();
        tic = timer_start;

        // Update
        for(size_t istream = 0 ; istream < streams_send_size ; istream++){
            stream = streams_send[istream].stream;
            assistant = streams_send[istream].assistant;
            for(unsigned i = 0 ; i < info->sample_max_number ; i++){
                for(size_t isignal = 0 ; isignal < streams_send[istream].signals_size ; isignal++){
                    signal = streams_send[istream].signals[isignal];
                    if(ed247_stream_assistant_write_signal(assistant, signal, signal_sample, signal_sample_size) != ED247_STATUS_SUCCESS) ASSERT_TRUE(false);
                }
                if(ed247_stream_assistant_push_sample(assistant, NULL, NULL) != ED247_STATUS_SUCCESS) ASSERT_TRUE(false);
            }
        }
        
        tac = test::get_time_us();
        times[loop].update = tac - tic;

        // Send
        tic = tac;
        // std::cout << "SEND" << std::endl;
        if(ed247_send_pushed_samples(_context) != ED247_STATUS_SUCCESS) ASSERT_TRUE(false);
        // std::cout << "SEND: OK" << std::endl;
        
        // Recv
        counter = channels_recv_size;
        while(counter > 0){
            // std::cout << "WAIT" << std::endl;
            if(ed247_wait_frame(_context, &streams, 10000000) != ED247_STATUS_SUCCESS)
                ASSERT_TRUE(false) << "# LOOP [" << loop << "] RECV [" << counter_recv << "] SEND [" << counter_send << "]";
            // std::cout << "WAIT: OK" << std::endl;
            counter--;
        }

        // Retrieve data
        tac = test::get_time_us();
        times[loop].recv = tac - tic;
        tic = tac;
        for(size_t istream = 0 ; istream < streams_recv_size ; istream++){
            stream = streams_recv[istream].stream;
            assistant = streams_recv[istream].assistant;
            const void * recv_signal_sample;
            size_t recv_signal_sample_size;
            bool empty = false;
            do{
                if(ed247_stream_assistant_pop_sample(assistant, NULL, NULL, NULL, &empty) != ED247_STATUS_SUCCESS) ASSERT_TRUE(false);
                for(size_t isignal = 0 ; isignal < streams_recv[istream].signals_size ; isignal++){
                    signal = streams_recv[istream].signals[isignal];
                    if(ed247_stream_assistant_read_signal(assistant, signal, &recv_signal_sample, &recv_signal_sample_size) != ED247_STATUS_SUCCESS) ASSERT_TRUE(false);
                    // ASSERT_EQ(memcmp(signal_sample, recv_signal_sample, recv_signal_sample_size), 0);
                    if(*(const char*)signal_sample != *(const char*)recv_signal_sample) ASSERT_TRUE(false);
                }
            }while(!empty);
        }

        // Stop timer
        timer_stop = test::get_time_us();
        tac = timer_stop;
        times[loop].get = tac - tic;
        times[loop].all = timer_stop - timer_start;

        memhooks_enable(false);
#ifdef __linux__
        memhooks_count_t count;
        memhooks_get_count(&count);
        ASSERT_EQ(count.malloc_count, 0);
#endif
    }
    
    // Checkpoint
    std::cout << "TEST ENTITY [" << GetParam().src_id << "]: Checkpoint n°2" << std::endl;
    TestSend(); TestWait();

    // End
    TestSend();

    free(sample);

#ifdef __linux__
    param.sched_priority = 0;
    if(sched_setscheduler(0, SCHED_OTHER, &param) != 0)
        std::cout << "WARNING: Executable shall be launched with administrator rights" << std::endl;
#endif

    std::cout << "# SIGNAL HANDLER CALLED [" << signal_counter << "] TIMES" << std::endl;
    display_results(loop_count);

    // Callback
    // ASSERT_EQ(ed247_unregister_com_send_callback(_context, &com_send_callback), ED247_STATUS_SUCCESS);
    // ASSERT_EQ(ed247_unregister_com_recv_callback(_context, &com_recv_callback), ED247_STATUS_SUCCESS);
}

void com_send_callback()
{
    counter_send++;
    tac = test::get_time_us();
    times[loop].send = tac - tic;
}

void com_recv_callback()
{
    counter_recv++;
    tic = test::get_time_us();
    tac = 0;
}

void display_results(uint64_t loop_count)
{
    timer_data_t time = {0, 0, 0, 0, 0};
    timer_data_t max = {0, 0, 0, 0, 0};
    timer_data_t min = {0, 0, 0, 0, 0};
    unsigned begin = 5;
    unsigned count = 0;
    for(unsigned i = begin ; i < loop_count ; i++){
        time.update += times[i].update;
        max.update = max.update == 0 ? times[i].update : times[i].update > max.update ? times[i].update : max.update;
        min.update = min.update == 0 ? times[i].update : times[i].update < min.update ? times[i].update : min.update;
        time.send += times[i].send;
        max.send = max.send == 0 ? times[i].send : times[i].send > max.send ? times[i].send : max.send;
        min.send = min.send == 0 ? times[i].send : times[i].send < min.send ? times[i].send : min.send;
        time.recv += times[i].recv;
        max.recv = max.recv == 0 ? times[i].recv : times[i].recv > max.recv ? times[i].recv : max.recv;
        min.recv = min.recv == 0 ? times[i].recv : times[i].recv < min.recv ? times[i].recv : min.recv;
        time.get += times[i].get;
        max.get = max.get == 0 ? times[i].get : times[i].get > max.get ? times[i].get : max.get;
        min.get = min.get == 0 ? times[i].get : times[i].get < min.get ? times[i].get : min.get;
        time.all += times[i].all;
        max.all = max.all == 0 ? times[i].all : times[i].all > max.all ? times[i].all : max.all;
        min.all = min.all == 0 ? times[i].all : times[i].all < min.all ? times[i].all : min.all;
        count++;
    }
    time.update /= count;
    time.send /= count;
    time.recv /= count;
    time.get /= count;
    time.all /= count;

    std::cout << "## Performances" << std::endl;
    std::cout << "## Update  [" << std::right << std::setw(4) << std::setfill('0') << min.update << "] < [" << std::right << std::setw(4) << std::setfill('0') << time.update << "] < [" << std::setw(4) << std::setfill('0') << max.update << "] us" << std::endl;
    std::cout << "## Send    [" << std::right << std::setw(4) << std::setfill('0') << min.send << "] < [" << std::right << std::setw(4) << std::setfill('0') << time.send << "] < [" << std::setw(4) << std::setfill('0') << max.send << "] us" << std::endl;
    std::cout << "## Receive [" << std::right << std::setw(4) << std::setfill('0') << min.recv << "] < [" << std::right << std::setw(4) << std::setfill('0') << time.recv << "] < [" << std::setw(4) << std::setfill('0') << max.recv << "] us" << std::endl;
    std::cout << "## Get     [" << std::right << std::setw(4) << std::setfill('0') << min.get << "] < [" << std::right << std::setw(4) << std::setfill('0') << time.get << "] < [" << std::setw(4) << std::setfill('0') << max.get << "] us" << std::endl;
    std::cout << "## All     [" << std::right << std::setw(4) << std::setfill('0') << min.all << "] < [" << std::right << std::setw(4) << std::setfill('0') << time.all << "] < [" << std::setw(4) << std::setfill('0') << max.all << "] us" << std::endl;

    free(times);
}

std::vector<TestParams> streams_files = {
    {TEST_ENTITY_MASTER_ID, TEST_ENTITY_SLAVE_ID, std::string(CONFIG_PATH"/ft_perfos/run_dis_master.xml")},
    {TEST_ENTITY_MASTER_ID, TEST_ENTITY_SLAVE_ID, std::string(CONFIG_PATH"/ft_perfos/run_a429_master.xml")}
};

std::vector<TestParams> signals_files = {
    {TEST_ENTITY_MASTER_ID, TEST_ENTITY_SLAVE_ID, std::string(CONFIG_PATH"/ft_perfos/run_dis_master.xml")}
};

INSTANTIATE_TEST_CASE_P(FT_PERFOS_STREAMS, PerfosStreamsContext, ::testing::ValuesIn(streams_files));
INSTANTIATE_TEST_CASE_P(FT_PERFOS_SIGNALS, PerfosSignalsContext, ::testing::ValuesIn(signals_files));

int main(int argc, char **argv)
{
    ed247_set_log_level(ED247_LOG_LEVEL_ERROR);
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}