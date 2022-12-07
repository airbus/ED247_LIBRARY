/* -*- mode: c++; c-basic-offset: 2 -*-  */
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
#ifndef _ED247_STREAM_H_
#define _ED247_STREAM_H_
#include "ed247_xml.h"
#include "ed247_signal.h"
#include "ed247_sample.h"


// base structures for C API
struct ed247_internal_stream_t {};
struct ed247_internal_channel_t;
struct ed247_internal_stream_assistant_t;

namespace ed247
{
  class Stream : public ed247_internal_stream_t
  {
  public:

    Stream(const xml::Stream*        configuration,
           ed247_internal_channel_t* ed247_api_channel,            // Parent for ed247_stream_get_channel()
           uint32_t                  sample_size_size);            // Size of the sample size header field (that depend on stream type)

    virtual ~Stream();

    // No implicit copy.
    Stream(const Stream & other) = delete;
    Stream& operator = (const Stream & other) = delete;


    // configuration accessors
    const std::string& get_name() const        { return _configuration->_name;                  }
    const std::string& get_icd() const         { return _configuration->_icd;                   }
    const std::string& get_comment() const     { return _configuration->_comment;               }
    ed247_uid_t get_uid() const                { return _configuration->_uid;                   }
    ed247_stream_type_t get_type() const       { return _configuration->_type;                  }
    ed247_direction_t get_direction() const    { return _configuration->_direction;             }
    uint32_t get_sample_max_size_bytes() const { return _configuration->_sample_max_size_bytes; }
    uint32_t get_sample_max_number() const     { return _configuration->_sample_max_number;     }
    uint32_t get_max_size() const              { return _max_size;                              }


    // implementation of API method ed247_stream_get_channel()
    ed247_internal_channel_t* get_api_channel() { return _ed247_api_channel; }


    // Handle user-data
    void set_user_data(void *user_data)  { _user_data = user_data; }
    void get_user_data(void **user_data) { *user_data = _user_data; }


    // Signals specific part
    bool is_signal_based() const                           { return _configuration->is_signal_based(); }
    virtual uint32_t get_sampling_period_us()              { return 0;                                 }
    ed247_internal_stream_assistant_t* get_api_assistant() { return _assistant.get();                  }
    signal_list_t& get_signals()                           { return _signals;                          }
    signal_list_t find_signals(std::string str_regex);
    signal_ptr_t get_signal(std::string str_name);


    // Handing samples
    uint32_t get_incoming_sample_number() { return _recv_stack.size(); }
    uint32_t get_outgoing_sample_number() { return _send_stack.size(); }

    // Append sample_data to send stack
    // If send stack is full, the oldest pushed sample will be silently dropped. This is not an error.
    // If data_timestamp provided, sample will be dated if enabled by configuration
    // If full provided, will be set to true if send stack is full after this appending
    // Return false on fatal error. (stack full is not an error)
    bool push_sample(const void* sample_data, uint32_t sample_size,
                     const ed247_timestamp_t* data_timestamp,
                     bool* full);

    // Return the oldest received sample and mark it as removed.
    // Set empty to true if incoming stack is empty after the pop.
    // If stack is empty before the pop, an arbitrary, but valid, sample will be returned. (see get_incoming_sample_number())
    StreamSample& pop_sample(bool* empty);

    // Encode each pushed sample of the stream in frame.
    // Return encoded length.
    uint32_t encode(char* frame, uint32_t frame_size);

    // Decode the given frame and fill internal samples.
    // Return false on error (the rest of the frame cannot be decoded)
    bool decode(const char* frame, uint32_t frame_size, const ed247_sample_details_t& frame_details);

    // Callback managment (Can we remove this ugly API ?)
    ed247_status_t register_callback(ed247_context_t context, ed247_stream_recv_callback_t callback);
    ed247_status_t unregister_callback(ed247_context_t context, ed247_stream_recv_callback_t callback);
    bool run_callbacks();


  protected:
    const xml::Stream*                                  _configuration;
    uint32_t                                            _sample_size_size;
    uint32_t                                            _sample_first_header_size;
    uint32_t                                            _sample_next_header_size;
    uint32_t                                            _max_size;
    signal_list_t                                       _signals;
    std::unique_ptr<ed247_internal_stream_assistant_t>  _assistant;
    StreamSampleRingBuffer                              _recv_stack;
    StreamSampleRingBuffer                              _send_stack;

  private:
    ed247_internal_channel_t*                           _ed247_api_channel;
    void*                                               _user_data;

    // Callback managment (Can we remove this ugly API ?)
    struct CallbackData {
      ed247_context_t              context;
      ed247_stream_recv_callback_t callback;
    };
    std::vector<CallbackData>  _callbacks;

    ED247_FRIEND_TEST();
  };


  //
  // Streams with signals
  //
  struct StreamSignals: public Stream {
    StreamSignals(const xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel,
                  signal_set_t& context_signal_set, uint32_t sample_size_size);
    uint32_t get_sampling_period_us() override { return ((xml::StreamSignals*)_configuration)->_sampling_period_us; }

  };

  typedef std::shared_ptr<Stream>                       stream_ptr_t;
  typedef std::vector<stream_ptr_t>                     stream_list_t;
  typedef std::unordered_map<std::string, stream_ptr_t> stream_map_t;


  //
  // Global list of streams
  //
  class StreamSet
  {
  public:
    StreamSet(ed247::signal_set_t& pool_signals) : _pool_signals(pool_signals) {}
    stream_ptr_t create(const xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel);

    stream_ptr_t get(std::string name);
    stream_list_t find(std::string regex);

    stream_map_t& streams()  { return _streams;        }
    uint32_t size()          { return _streams.size(); }

  private:
    stream_map_t          _streams;
    ed247::signal_set_t&  _pool_signals;
  };
}

#endif
