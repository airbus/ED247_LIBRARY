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
#include "ed247_header.h"
#include "ed247_bswap.h"
#include <algorithm>
#include <memory>
#include <map>


// base structures for C API
struct ed247_internal_stream_t {};
struct ed247_internal_channel_t;
struct ed247_internal_stream_assistant_t;

namespace ed247
{
  class Stream : public ed247_internal_stream_t, public std::enable_shared_from_this<Stream>
  {
  public:

    Stream(const xml::Stream*        configuration,
           ed247_internal_channel_t* ed247_api_channel,            // Parent for ed247_stream_get_channel()
           uint32_t                  sample_header_size);          // Size of the sample header (that depend on stream type)

    virtual ~Stream(){}

    void set_user_data(void *user_data)
    {
      _user_data = user_data;
    }

    void get_user_data(void **user_data)
    {
      *user_data = _user_data;
    }

    bool is_signal_based() const { return _configuration->is_signal_based(); }

    const xml::Stream * get_configuration() const
    {
      return _configuration;
    }

    std::string get_name() const
    {
      return _configuration ? std::string(_configuration->_name) : std::string();
    }

    // Return false on failure
    bool push_sample(const void * sample_data, uint32_t sample_size, const ed247_timestamp_t * data_timestamp = nullptr, bool * full = nullptr);

    // Return an invalid shared_ptr on error (empty is not an error)
    StreamSample& pop_sample(bool *empty = nullptr);

    StreamSampleRingBuffer & recv_stack()
    {
      return _recv_stack;
    }

    StreamSampleRingBuffer & send_stack()
    {
      return _send_stack;
    }

    ed247_status_t check_sample_size(uint32_t sample_size) const;
    std::unique_ptr<StreamSample> allocate_sample() const;

    virtual uint32_t encode(char * frame, uint32_t frame_size) = 0;

    // return false if the frame has not been successfully decoded
    virtual bool decode(const char * frame, uint32_t frame_size, const FrameHeader * header = nullptr) = 0;

    uint32_t get_max_size() const { return _max_size; }

    signal_list_t find_signals(std::string str_regex);

    signal_ptr_t get_signal(std::string str_name);

    std::shared_ptr<signal_list_t> signals() { return _signals; };

    ed247_status_t register_callback(ed247_context_t context, ed247_stream_recv_callback_t callback)
    {
      auto it = std::find_if(_callbacks.begin(), _callbacks.end(),
                             [&context, &callback](const std::pair<ed247_context_t,ed247_stream_recv_callback_t> & element){
                               return element.first == context && element.second == callback;
                             });
      if(it != _callbacks.end()){
        return ED247_STATUS_FAILURE;
      }
      _callbacks.push_back(std::make_pair(context, callback));
      return ED247_STATUS_SUCCESS;
    }

    ed247_status_t unregister_callback(ed247_context_t context, ed247_stream_recv_callback_t callback)
    {
      auto it = std::find_if(_callbacks.begin(), _callbacks.end(),
                             [&context, &callback](const std::pair<ed247_context_t,ed247_stream_recv_callback_t> & element){
                               return element.first == context && element.second == callback;
                             });
      if(it == _callbacks.end()){
        return ED247_STATUS_FAILURE;
      }
      _callbacks.erase(it);
      return ED247_STATUS_SUCCESS;
    }

    // implementation of ed247_stream_get_channel()
    ed247_internal_channel_t* get_api_channel()
    {
      return _ed247_api_channel;
    }

    bool run_callbacks()
    {
      for(auto & pcallback : _callbacks)
      {
        if(pcallback.second) {
          ed247_status_t status = (*pcallback.second)(pcallback.first, this);
          if (status != ED247_STATUS_SUCCESS) {
            PRINT_DEBUG("User callback fail with return code: " << status);
            return false;
          }
        } else {
          PRINT_WARNING("Invalid user callback!");
          return false;
        }
      }
      return true;
    }

    void encode_data_timestamp(const StreamSample& sample, char * frame, uint32_t frame_size, uint32_t & frame_index)
    {
      if(_configuration->_data_timestamp._enable == ED247_YESNO_YES){
        if(frame_index == 0){
          // Datatimestamp
          if((frame_index + sizeof(uint32_t) + sizeof(uint32_t)) > frame_size) {
            THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
          }
          _data_timestamp = sample.data_timestamp();
          *(uint32_t*)(frame+frame_index) = htonl(sample.data_timestamp().epoch_s);
          frame_index += sizeof(uint32_t);
          *(uint32_t*)(frame+frame_index) = htonl(sample.data_timestamp().offset_ns);
          frame_index += sizeof(uint32_t);
        }else if(_configuration->_data_timestamp._enable_sample_offset == ED247_YESNO_YES){
          // Precise Datatimestamp
          if((frame_index + sizeof(uint32_t)) > frame_size) {
            THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
          }
          *(uint32_t*)(frame+frame_index) =
            htonl((uint32_t)(((int32_t)sample.data_timestamp().epoch_s - (int32_t)_data_timestamp.epoch_s)*1000000000
                             + ((int32_t)sample.data_timestamp().offset_ns - (int32_t)_data_timestamp.offset_ns)));
          frame_index += sizeof(int32_t);
        }
      }
    }

    // return false if the frame has not been successfully decoded
    bool decode_data_timestamp(const char * frame, const uint32_t & frame_size, uint32_t & frame_index, ed247_timestamp_t & data_timestamp, ed247_timestamp_t & timestamp)
    {
      if(_configuration->_data_timestamp._enable == ED247_YESNO_YES){
        if(frame_index == 0){
          // Data Timestamp
          if((frame_size-frame_index) < sizeof(ed247_timestamp_t)) {
            PRINT_ERROR("Stream '" << get_name() << "': Invalid received frame size: " << frame_size);
            return false;
          }
          timestamp.epoch_s = ntohl(*(uint32_t*)(frame+frame_index));
          frame_index += sizeof(uint32_t);
          timestamp.offset_ns = ntohl(*(uint32_t*)(frame+frame_index));
          frame_index += sizeof(uint32_t);
          data_timestamp = timestamp;
          _working_sample.set_data_timestamp(data_timestamp);
        }else if(_configuration->_data_timestamp._enable_sample_offset == ED247_YESNO_YES){
          // Precise Data Timestamp
          int32_t offset_ns = (int32_t)ntohl(*(uint32_t*)(frame+frame_index));
          frame_index += sizeof(uint32_t);
          timestamp = data_timestamp;
          timestamp.epoch_s += offset_ns / 1000000000;
          timestamp.offset_ns += (offset_ns - (offset_ns / 1000000000) * 1000000000);
          _working_sample.set_data_timestamp(timestamp);
        }else{
          // No precise data timestamp, use the first one
          _working_sample.set_data_timestamp(data_timestamp);
        }
      }
      return true;
    }

    ed247_internal_stream_assistant_t* get_api_assistant() { return _assistant.get(); }


    signal_list_t& get_signals() { return *_signals; }

  protected:
    const xml::Stream* _configuration;
    ed247_internal_channel_t* _ed247_api_channel;
    StreamSampleRingBuffer _recv_stack;
    std::shared_ptr<StreamSample> _recv_working_sample; // Pointer on a recv_stack element
    StreamSampleRingBuffer _send_stack;
    std::shared_ptr<StreamSample> _send_working_sample; // New element, not pointer on a member of send_stack
    uint32_t _max_size;
    StreamSample _working_sample;
    std::shared_ptr<signal_list_t> _signals;
    std::vector<std::pair<ed247_context_t,ed247_stream_recv_callback_t>> _callbacks;
    ed247_timestamp_t _data_timestamp;
    void *_user_data;
    std::shared_ptr<ed247_internal_stream_assistant_t> _assistant;
  };

  typedef std::shared_ptr<Stream> stream_ptr_t;
  typedef std::vector<stream_ptr_t>   stream_list_t;

  class StreamSet : public std::enable_shared_from_this<StreamSet>
  {
  public:
    StreamSet();
    StreamSet(std::shared_ptr<ed247::signal_set_t> & pool_signals);

    ~StreamSet(){};

    stream_ptr_t get(const xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel);

    stream_list_t find(std::string str_regex);

    stream_ptr_t get(std::string str_name);

    std::shared_ptr<stream_list_t> streams();

    uint32_t size() const;

  private:
    std::shared_ptr<stream_list_t>    _streams;
    std::shared_ptr<ed247::signal_set_t> _pool_signals;
  };


  //
  // Streams specialization
  //

  struct StreamSignals: public Stream {
    StreamSignals(const xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel, signal_set_t& context_signal_set, uint32_t sample_header_size);
  };

  struct StreamA429 : public Stream {
    StreamA429(const xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel);
    virtual uint32_t encode(char* frame, uint32_t frame_size) override final;
    virtual bool decode(const char* frame, uint32_t frame_size, const FrameHeader* header = nullptr) override final;
  };
  struct StreamA664 : public Stream {
    StreamA664(const xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel);
    virtual uint32_t encode(char* frame, uint32_t frame_size) override final;
    virtual bool decode(const char* frame, uint32_t frame_size, const FrameHeader* header = nullptr) override final;
  };
  struct StreamA825 : public Stream {
    StreamA825(const xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel);
    virtual uint32_t encode(char* frame, uint32_t frame_size) override final;
    virtual bool decode(const char* frame, uint32_t frame_size, const FrameHeader* header = nullptr) override final;
  };
  struct StreamSERIAL : public Stream {
    StreamSERIAL(const xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel);
    virtual uint32_t encode(char* frame, uint32_t frame_size) override final;
    virtual bool decode(const char* frame, uint32_t frame_size, const FrameHeader* header = nullptr) override final;
  };
  struct StreamAUDIO : public Stream {
    StreamAUDIO(const xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel);
    virtual uint32_t encode(char* frame, uint32_t frame_size) override final;
    virtual bool decode(const char* frame, uint32_t frame_size, const FrameHeader* header = nullptr) override final;
  };

  struct StreamDISCRETE : public StreamSignals {
    StreamDISCRETE(const xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel, signal_set_t& context_signal_set);
    virtual uint32_t encode(char* frame, uint32_t frame_size) override final;
    virtual bool decode(const char* frame, uint32_t frame_size, const FrameHeader* header = nullptr) override final;
  };
  struct StreamANALOG : public StreamSignals {
    StreamANALOG(const xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel, signal_set_t& context_signal_set);
    virtual uint32_t encode(char* frame, uint32_t frame_size) override final;
    virtual bool decode(const char* frame, uint32_t frame_size, const FrameHeader* header = nullptr) override final;
  };
  struct StreamNAD : public StreamSignals {
    StreamNAD(const xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel, signal_set_t& context_signal_set);
    virtual uint32_t encode(char* frame, uint32_t frame_size) override final;
    virtual bool decode(const char* frame, uint32_t frame_size, const FrameHeader* header = nullptr) override final;
  };
  struct StreamVNAD : public StreamSignals {
    StreamVNAD(const xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel, signal_set_t& context_signal_set);
    virtual uint32_t encode(char* frame, uint32_t frame_size) override final;
    virtual bool decode(const char* frame, uint32_t frame_size, const FrameHeader* header = nullptr) override final;
  };

}

#endif
