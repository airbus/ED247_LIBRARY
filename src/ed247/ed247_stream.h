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
#include "ed247_internals.h"
#include "ed247_xml.h"
#include "ed247_signal.h"
#include "ed247_sample.h"

#include <memory>
#include <tuple>
#include <map>

namespace ed247
{

  class Channel;
  class BaseStream;
  class signal;

  typedef std::shared_ptr<BaseStream> stream_ptr_t;
  typedef std::vector<stream_ptr_t>   stream_list_t;


  class CircularStreamSampleBuffer {
  public:
    uint32_t size() const
    {
      return _index_size;
    }

    std::shared_ptr<StreamSample> & next_write()
    {
      return _samples[_index_write];
    }

    bool increment()
    {
      if(_index_size < _sample_max_number){
        _index_write = (_index_write+1) % _samples.size();
        update_size();
        return _index_size == _sample_max_number;
      }else{
        _index_write = (_index_write+1) % _samples.size();
        _index_read = (_index_read+1) % _samples.size();
        update_size();
        return true;
      }
    }

    std::shared_ptr<StreamSample> & pop_front(bool * empty = nullptr) // True if empty after pop
    {
      if(size() == 0){
        if(empty) *empty = true;
        return _samples[_index_read];
      }else{
        uint32_t index_read = _index_read;
        _index_read = (_index_read+1) % _samples.size();
        update_size();
        if(empty) *empty = (size() == 0);
        return _samples[index_read];
      }
    }

    std::shared_ptr<StreamSample> front()
    {
      return size() > 0 ? _samples[_index_read] : nullptr;
    }

    std::shared_ptr<StreamSample> at(uint32_t index)
    {
      return index < size() ? _samples[(_index_read+index)%_samples.size()] : nullptr;
    }

    std::shared_ptr<StreamSample> back()
    {
      return size() > 0 ? _samples[_index_write == 0 ? (_samples.size()-1) : (_index_write-1)] : nullptr;
    }

    void allocate(uint32_t sample_max_size_bytes, uint32_t sample_max_number)
    {
      _sample_max_size_bytes = sample_max_size_bytes;
      _sample_max_number = sample_max_number;
      _samples.resize(sample_max_number+1);
      for(auto iter = _samples.begin() ; iter != _samples.end() ; iter++){
        (*iter) = std::make_shared<StreamSample>();
        (*iter)->allocate(_sample_max_size_bytes);
      }
      _index_read = 0;
      _index_write = 0;
      update_size();
    }

    std::vector<std::shared_ptr<StreamSample>> & samples()
    {
      return _samples;
    }

    bool full()
    {
      return size() >= _sample_max_number;
    }

  protected:
    std::vector<std::shared_ptr<StreamSample>> _samples;
    uint32_t _index_read{0};
    uint32_t _index_write{0};
    uint32_t _sample_max_size_bytes{0};
    uint32_t _sample_max_number{0};
    uint32_t _index_size{0};

    void update_size()
    {
      _index_size = _index_write >= _index_read ? (_index_write - _index_read) : (_samples.size() + _index_write - _index_read);
    }
  };

  template<ed247_stream_type_t ... E>
  struct StreamBuilder {
    StreamBuilder() {}
    stream_ptr_t create(const ed247_stream_type_t & type, const xml::Stream* configuration, std::shared_ptr<ed247::signal_set_t> & pool_signals);
  };

  template<ed247_stream_type_t T, ed247_stream_type_t ... E>
  struct StreamBuilder<T, E...> : public StreamBuilder<E...>, private StreamTypeChecker<T> {
    StreamBuilder() : StreamBuilder<E...>() {}
    stream_ptr_t create(const ed247_stream_type_t & type, const xml::Stream* configuration, std::shared_ptr<ed247::signal_set_t> & pool_signals);
  };

  template<ed247_stream_type_t T>
  struct StreamBuilder<T> : private StreamTypeChecker<T> {
    StreamBuilder() {}
    stream_ptr_t create(const ed247_stream_type_t & type, const xml::Stream* configuration, std::shared_ptr<ed247::signal_set_t> & pool_signals);
  };

  class FrameHeader;
  class BaseStream : public ed247_internal_stream_t, public std::enable_shared_from_this<BaseStream>
  {
  public:
    BaseStream():
      _user_data(NULL)
    {}
    BaseStream(const xml::Stream* configuration):
      _configuration(configuration),
      _signals(std::make_shared<signal_list_t>()),
      _user_data(NULL)
    {
    }

    virtual ~BaseStream(){}

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
    std::shared_ptr<StreamSample> pop_sample(bool *empty = nullptr);

    CircularStreamSampleBuffer & recv_stack()
    {
      return _recv_stack;
    }

    CircularStreamSampleBuffer & send_stack()
    {
      return _send_stack;
    }

    virtual ed247_status_t check_sample_size(uint32_t sample_size) const = 0;
    virtual std::unique_ptr<StreamSample> allocate_sample() const = 0;

    virtual uint32_t encode(char * frame, uint32_t frame_size) = 0;

    // return false if the frame has not been successfully decoded
    virtual bool decode(const char * frame, uint32_t frame_size, const FrameHeader * header = nullptr) = 0;

    Sample & buffer()
    {
      return _buffer;
    }

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

    std::shared_ptr<Channel> get_channel()
    {
      return _channel.lock();
    }

  protected:

    const xml::Stream* _configuration;
    std::weak_ptr<Channel> _channel;
    CircularStreamSampleBuffer _recv_stack;
    std::shared_ptr<StreamSample> _recv_working_sample; // Pointer on a recv_stack element
    CircularStreamSampleBuffer _send_stack;
    std::shared_ptr<StreamSample> _send_working_sample; // New element, not pointer on a member of send_stack
    Sample _buffer;
    StreamSample _working_sample;
    std::shared_ptr<signal_list_t> _signals;
    std::vector<std::pair<ed247_context_t,ed247_stream_recv_callback_t>> _callbacks;

  private:

    ed247_timestamp_t _data_timestamp;
    void *_user_data;

  protected:

    // Create elements in circular buffers (send/recv) according to the bus type
    virtual void allocate_stacks() = 0;

    virtual void allocate_buffer() = 0;

    virtual void allocate_working_sample() = 0;

    void register_channel(Channel & channel, ed247_direction_t direction);

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

    void encode_data_timestamp(const std::shared_ptr<StreamSample> & sample, char * frame, uint32_t frame_size, uint32_t & frame_index)
    {
      if(_configuration->_data_timestamp._enable == ED247_YESNO_YES){
        if(frame_index == 0){
          // Datatimestamp
          if((frame_index + sizeof(uint32_t) + sizeof(uint32_t)) > frame_size) {
            THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
          }
          _data_timestamp = sample->data_timestamp();
          *(uint32_t*)(frame+frame_index) = htonl(sample->data_timestamp().epoch_s);
          frame_index += sizeof(uint32_t);
          *(uint32_t*)(frame+frame_index) = htonl(sample->data_timestamp().offset_ns);
          frame_index += sizeof(uint32_t);
        }else if(_configuration->_data_timestamp._enable_sample_offset == ED247_YESNO_YES){
          // Precise Datatimestamp
          if((frame_index + sizeof(uint32_t)) > frame_size) {
            THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
          }
          *(uint32_t*)(frame+frame_index) =
            htonl((uint32_t)(((int32_t)sample->data_timestamp().epoch_s - (int32_t)_data_timestamp.epoch_s)*1000000000
                             + ((int32_t)sample->data_timestamp().offset_ns - (int32_t)_data_timestamp.offset_ns)));
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

  public:
    class Pool : public std::enable_shared_from_this<Pool>
    {
    public:
      Pool();
      Pool(std::shared_ptr<ed247::signal_set_t> & pool_signals);

      ~Pool(){};

      stream_ptr_t get(const xml::Stream* configuration);

      stream_list_t find(std::string str_regex);

      stream_ptr_t get(std::string str_name);

      std::shared_ptr<stream_list_t> streams();

      uint32_t size() const;

    private:
      std::shared_ptr<stream_list_t>    _streams;
      std::shared_ptr<ed247::signal_set_t> _pool_signals;
      StreamBuilder<
        ED247_STREAM_TYPE_A429,
        ED247_STREAM_TYPE_A664,
        ED247_STREAM_TYPE_A825,
        ED247_STREAM_TYPE_SERIAL,
        ED247_STREAM_TYPE_AUDIO,
        ED247_STREAM_TYPE_DISCRETE,
        ED247_STREAM_TYPE_ANALOG,
        ED247_STREAM_TYPE_NAD,
        ED247_STREAM_TYPE_VNAD> _builder;

    };
    class Builder
    {
    public:
      void build(std::shared_ptr<Pool> & pool, const xml::Stream* configuration, Channel & channel) const;
    };
    class Assistant : public ed247_internal_stream_assistant_t
    {
    public:

      Assistant() {}
      Assistant(stream_ptr_t stream):
        _stream(stream)
      {
        uint32_t capacity = 0;
        for(auto signal : *stream->_signals){
          auto send_sample = signal->allocate_sample();
          auto recv_sample = signal->allocate_sample();
          capacity += send_sample->capacity();
          if(signal->get_type() == ED247_SIGNAL_TYPE_VNAD)
            capacity += sizeof(uint16_t);
          if(_send_samples.size() <= signal->position())
            _send_samples.resize(signal->position()+1);
          _send_samples[signal->position()].first = signal;
          _send_samples[signal->position()].second = std::move(send_sample);
          if(_recv_samples.size() <= signal->position())
            _recv_samples.resize(signal->position()+1);
          _recv_samples[signal->position()].first = signal;
          _recv_samples[signal->position()].second = std::move(recv_sample);
        }
        _buffer.allocate(capacity);
      }

      stream_ptr_t get_stream()
      {
        return _stream;
      }

      bool is_valid()
      {
        auto type = _stream->get_configuration()->_type;
        return _stream ? (
          type == ED247_STREAM_TYPE_DISCRETE ||
          type == ED247_STREAM_TYPE_ANALOG ||
          type == ED247_STREAM_TYPE_NAD ||
          type == ED247_STREAM_TYPE_VNAD) : false;
      }

      bool write(const signal& signal, const void *data, uint32_t size)
      {
        if(!(_stream->get_configuration()->_direction & ED247_DIRECTION_OUT)) {
          PRINT_ERROR("Stream '" << _stream->get_name() << "': Cannot write Signal [" << signal.get_name() << "] to an non-output stream");
          return false;
        }
        if(signal.get_type() == ED247_SIGNAL_TYPE_VNAD){
          uint32_t sample_max_size = signal.get_vnad_max_number() * (signal.get_sample_max_size_bytes() + sizeof(uint16_t));
          if(size > sample_max_size) {
            PRINT_ERROR("Stream '" << _stream->get_name() << "': Cannot write Signal [" << signal.get_name() << "] "
                        "as Signal SampleMaxSizeBytes is [" << sample_max_size << "] and data to write is of size [" << size << "]");
            return false;
          }
        }
        auto & sample = _send_samples[signal.position()].second;
        if (sample->copy(data, size) == false) {
          PRINT_ERROR("Stream '" << _stream->get_name() << "': Cannot write Signal [" << signal.get_name() << "]: invalid size: " << size);
          return false;
        }
        return true;
      }

      bool read(const signal& signal, const void **data, uint32_t * size)
      {
        if(!(_stream->get_configuration()->_direction & ED247_DIRECTION_IN)) {
          PRINT_ERROR("Stream '" << _stream->get_name() << "': Cannot read Signal [" << signal.get_name() << "] from a non-input stream");
          return false;
        }
        auto & sample = _recv_samples[signal.position()].second;
        *data = sample->data();
        *size = sample->size();
        return true;
      }

      bool push(const ed247_timestamp_t * data_timestamp = nullptr, bool * full = nullptr)
      {
        if(!(_stream->get_configuration()->_direction & ED247_DIRECTION_OUT)) {
          PRINT_ERROR("Stream '" << _stream->get_name() << "': Cannot push to a non-output stream");
          return false;
        }
        encode();
        return _stream->push_sample(_buffer.data(), _buffer.size(), data_timestamp, full);
      }

      // return false if the signal has not been successfully decoded
      bool pop(const ed247_timestamp_t **data_timestamp = nullptr, const ed247_timestamp_t **recv_timestamp = nullptr,
               const ed247_sample_details_t **frame_infos = nullptr, bool *empty = nullptr)
      {
        if(!(_stream->get_configuration()->_direction & ED247_DIRECTION_IN)) {
          PRINT_ERROR("Stream '" << _stream->get_name() << "': Cannot pop from a non-input stream");
          return false;
        }
        auto sample = _stream->pop_sample(empty);
        if (!sample) return false;
        if(data_timestamp) *data_timestamp = &sample->data_timestamp();
        if(recv_timestamp) *recv_timestamp = &sample->recv_timestamp();
        if(frame_infos) *frame_infos = &sample->frame_infos();
        return decode(sample->data(), sample->size());
      };

      const Sample & buffer() { return _buffer; }

    private:
      void encode()
      {
        uint32_t buffer_index = 0;
        bool vnad_behaviour = false;
        for(auto & pair : _send_samples){
          if(!pair.first)
            continue;
          if(pair.first->get_type() == ED247_SIGNAL_TYPE_VNAD){
            *(uint16_t*)(_buffer.data_rw()+buffer_index) = (uint16_t)htons((uint16_t)pair.second->size());
            buffer_index += sizeof(uint16_t);
            vnad_behaviour = true;
          }else{
            if(pair.first->get_type() == ED247_SIGNAL_TYPE_ANALOG){
              buffer_index = (uint32_t)pair.first->get_byte_offset();
            }else if(pair.first->get_type() == ED247_SIGNAL_TYPE_DISCRETE){
              buffer_index = (uint32_t)pair.first->get_byte_offset();
            }else if(pair.first->get_type() == ED247_SIGNAL_TYPE_NAD){
              buffer_index = (uint32_t)pair.first->get_byte_offset();
            }else{
              THROW_ED247_ERROR("Signal [" << pair.first->get_name() << "] has not a valid type");
            }
          }
          memcpy(_buffer.data_rw()+buffer_index, pair.second->data(), pair.second->size());
          if(vnad_behaviour)
            buffer_index += pair.second->size();
          pair.second->reset();
        }
        if(vnad_behaviour)
          _buffer.set_size(buffer_index);
        else
          _buffer.set_size(_stream->get_configuration()->_sample_max_size_bytes);
      }

      // return false if the data has not been successfully decoded
      bool decode(const void * data, uint32_t size)
      {
        for(auto & pair : _recv_samples){
          pair.second->reset();
        }
        uint32_t buffer_index = 0;
        bool vnad_behaviour = false;
        for(auto & pair : _recv_samples){
          if(!pair.first)
            continue;
          uint32_t sample_size = 0;
          if(pair.first->get_type() == ED247_SIGNAL_TYPE_VNAD){
            if(buffer_index + sizeof(uint16_t) > size) {
              PRINT_ERROR("Signal '" << pair.first->get_name() << "': invalid VNAD size : " << size);
              return false;
            }
            sample_size = ntohs(*(uint16_t*)((const char*)data+buffer_index));
            buffer_index += sizeof(uint16_t);
            vnad_behaviour = true;
          }else{
            sample_size = pair.first->get_sample_max_size_bytes();
            if(pair.first->get_type() == ED247_SIGNAL_TYPE_ANALOG){
              buffer_index = (uint32_t)pair.first->get_byte_offset();
            }else if(pair.first->get_type() == ED247_SIGNAL_TYPE_DISCRETE){
              buffer_index = (uint32_t)pair.first->get_byte_offset();
            }else if(pair.first->get_type() == ED247_SIGNAL_TYPE_NAD){
              buffer_index = (uint32_t)pair.first->get_byte_offset();
            }else{
              PRINT_ERROR("Signal '" << pair.first->get_name() << "': Invalid type: " << pair.first->get_type());
              return false;
            }
          }
          pair.second->copy((const char*)data+buffer_index, sample_size);
          if(vnad_behaviour)
            buffer_index += sample_size;
        }
        return true;
      }

      stream_ptr_t _stream;
      std::vector<std::pair<signal_ptr_t, std::unique_ptr<Sample>>> _send_samples;
      std::vector<std::pair<signal_ptr_t, std::unique_ptr<Sample>>> _recv_samples;
      Sample _buffer;

      ED247_FRIEND_TEST();
    };

  protected:
    std::shared_ptr<Assistant> _assistant;

  public:
    std::shared_ptr<Assistant> get_assistant() { return _assistant; }

  };

  template<ed247_stream_type_t E>
  class Stream : public BaseStream, private StreamTypeChecker<E>
  {
  public:
    const ed247_stream_type_t type {E};

    using BaseStream::BaseStream;

    virtual uint32_t encode(char * frame, uint32_t frame_size) override final;

    // return false if the frame has not been successfully decoded
    virtual bool decode(const char * frame, uint32_t frame_size, const FrameHeader * header = nullptr) final;

    virtual ed247_status_t check_sample_size(uint32_t sample_size) const;

    virtual std::unique_ptr<StreamSample> allocate_sample() const final
    {
      return std::move(allocate_sample_impl());
    }

  protected:
    virtual void allocate_stacks() final;
    virtual void allocate_buffer() final;
    virtual void allocate_working_sample() final;

    template<ed247_stream_type_t T = E>
    typename std::enable_if<!StreamSignalTypeChecker<T>::value,std::unique_ptr<StreamSample>>::type
    allocate_sample_impl() const;

    template<ed247_stream_type_t T = E>
    typename std::enable_if<StreamSignalTypeChecker<T>::value,std::unique_ptr<StreamSample>>::type
    allocate_sample_impl() const;

  public:
    class Builder
    {
    public:
      template<ed247_stream_type_t T = E>
      typename std::enable_if<!StreamSignalTypeChecker<T>::value, std::shared_ptr<Stream<E>>>::type
      create(const xml::Stream* configuration,
             std::shared_ptr<ed247::signal_set_t> & pool_signals) const;

      template<ed247_stream_type_t T = E>
      typename std::enable_if<StreamSignalTypeChecker<T>::value, std::shared_ptr<Stream<E>>>::type
      create(const xml::Stream* configuration,
             std::shared_ptr<ed247::signal_set_t> & pool_signals) const;
    };
  };

};

#endif
