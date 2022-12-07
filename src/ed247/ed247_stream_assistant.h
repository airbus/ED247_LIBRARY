/* -*- mode: c++; c-basic-offset: 2 -*-  */
#ifndef _ED247_STREAM_ASSISTANT_H_
#define _ED247_STREAM_ASSISTANT_H_
#include "ed247_bswap.h"

// base structures for C API
struct ed247_internal_stream_assistant_t {};


namespace ed247
{
  class StreamAssistant : public ed247_internal_stream_assistant_t
  {
  public:

    StreamAssistant(Stream* stream):
      _stream(stream),
      _buffer(stream->get_sample_max_size_bytes())
    {
      for(auto signal : stream->get_signals()){
        auto send_sample = signal->allocate_sample();
        auto recv_sample = signal->allocate_sample();
        if(_send_samples.size() <= signal->position())
          _send_samples.resize(signal->position()+1);
        _send_samples[signal->position()].first = signal;
        _send_samples[signal->position()].second = std::move(send_sample);
        if(_recv_samples.size() <= signal->position())
          _recv_samples.resize(signal->position()+1);
        _recv_samples[signal->position()].first = signal;
        _recv_samples[signal->position()].second = std::move(recv_sample);
      }
    }

    ed247_internal_stream_t* get_api_stream()
    {
      return _stream;
    }

    bool write(const Signal& signal, const void *data, uint32_t size)
    {
      if(!(_stream->get_direction() & ED247_DIRECTION_OUT)) {
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

    bool read(const Signal& signal, const void **data, uint32_t * size)
    {
      if(!(_stream->get_direction() & ED247_DIRECTION_IN)) {
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
      if(!(_stream->get_direction() & ED247_DIRECTION_OUT)) {
        PRINT_ERROR("Stream '" << _stream->get_name() << "': Cannot push to a non-output stream");
        return false;
      }
      encode();
      return _stream->push_sample(_buffer.data(), _buffer.size(), data_timestamp, full);
    }

    ed247_status_t pop(const ed247_timestamp_t **data_timestamp = nullptr, const ed247_timestamp_t **recv_timestamp = nullptr,
                       const ed247_sample_details_t **frame_details = nullptr, bool *empty = nullptr)
    {
      if((_stream->get_direction() & ED247_DIRECTION_IN) == 0) {
        PRINT_ERROR("Stream '" << _stream->get_name() << "': Cannot pop from a non-input stream");
        return ED247_STATUS_FAILURE;
      }

      if (_stream->get_incoming_sample_number() == 0) {
        PRINT_CRAZY("Stream '" << _stream->get_name() << "': no data received.");
        return ED247_STATUS_NODATA;
      }

      StreamSample& sample = _stream->pop_sample(empty);
      if(data_timestamp) *data_timestamp = &sample.data_timestamp();
      if(recv_timestamp) *recv_timestamp = &sample.recv_timestamp();
      if(frame_details) *frame_details = &sample.frame_details();
      if (decode(sample.data(), sample.size())) {
        return ED247_STATUS_SUCCESS;
      } else {
        return ED247_STATUS_FAILURE;
      }
    };

    const Sample & buffer() { return _buffer; }

  private:
    void swap_copy(const char *source_data, char* dest_data, uint32_t size, const ed247_nad_type_t& nad_type)
    {
      uint32_t pos = 0;
      while (pos < size) {
        switch(nad_type) {
        case ED247_NAD_TYPE_INT8:
        case ED247_NAD_TYPE_UINT8:
          memcpy(dest_data + pos, source_data + pos, 1);
          break;
        case ED247_NAD_TYPE_INT16:
          *((uint16_t*)dest_data + pos) = bswap_16(*((uint16_t*)source_data + pos));
          break;
        case ED247_NAD_TYPE_INT32:
          *((uint32_t*)dest_data + pos) = bswap_32(*((uint32_t*)source_data + pos));
          break;
        case ED247_NAD_TYPE_INT64:
          *((uint64_t*)dest_data + pos) = bswap_64(*((uint64_t*)source_data + pos));
          break;
        case ED247_NAD_TYPE_UINT16:
          *((uint16_t*)dest_data + pos) = bswap_16(*((uint16_t*)source_data + pos));
          break;
        case ED247_NAD_TYPE_UINT32:
          *((uint32_t*)dest_data + pos) = bswap_32(*((uint32_t*)source_data + pos));
          break;
        case ED247_NAD_TYPE_UINT64:
          *((uint64_t*)dest_data + pos) = bswap_64(*((uint64_t*)source_data + pos));
          break;
        case ED247_NAD_TYPE_FLOAT32:
          *((uint32_t*)dest_data + pos) = bswap_32(*((uint32_t*)source_data + pos));
          break;
        case ED247_NAD_TYPE_FLOAT64:
          *((uint64_t*)dest_data + pos) = bswap_64(*((uint64_t*)source_data + pos));
          break;
        default:
          THROW_ED247_ERROR("Unexpected NAD type: " << nad_type);
        }
        pos += ed247::xml::Signal::get_nad_type_size(nad_type);
      }
    }

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

        swap_copy(pair.second->data(), _buffer.data_rw() + buffer_index, pair.second->size(), pair.first->get_nad_type());

        if(vnad_behaviour)
          buffer_index += pair.second->size();
        pair.second->reset();
      }
      if(vnad_behaviour)
        _buffer.set_size(buffer_index);
      else
        _buffer.set_size(_stream->get_sample_max_size_bytes());
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

        pair.second->set_size(sample_size);
        swap_copy((const char*)data + buffer_index, pair.second->data_rw(), sample_size, pair.first->get_nad_type());

        if(vnad_behaviour)
          buffer_index += sample_size;
      }
      return true;
    }

    Stream* _stream;
    std::vector<std::pair<signal_ptr_t, std::unique_ptr<Sample>>> _send_samples;
    std::vector<std::pair<signal_ptr_t, std::unique_ptr<Sample>>> _recv_samples;
    Sample _buffer;

    ED247_FRIEND_TEST();
  };

}

#endif
