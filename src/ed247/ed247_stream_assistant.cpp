/* -*- mode: c++; c-basic-offset: 2 -*-  */
#include "ed247_stream_assistant.h"
#include "ed247_bswap.h"
#include "ed247_stream.h"
#include "ed247_logs.h"

namespace {
  // Swap a payload of signals
  // Size is the length of the payload (so the number of signals is size/signal_size)
  void swap_copy(const char* source_data, char* dest_data, uint32_t size, const ed247_nad_type_t& nad_type)
  {
    uint32_t element_size = ed247::xml::Signal::get_nad_type_size(nad_type);

    if (element_size == 1) {
      memcpy(dest_data, source_data, size);
    }
    else
    {
      uint32_t pos = 0;
      while (pos < size) {
        switch(element_size) {
        case 2:
          *(uint16_t*)(dest_data + pos) = bswap_16(*(uint16_t*)(source_data + pos));
          break;
        case 4:
          *(uint32_t*)(dest_data + pos) = bswap_32(*(uint32_t*)(source_data + pos));
          break;
        case 8:
          *(uint64_t*)(dest_data + pos) = bswap_64(*(uint64_t*)(source_data + pos));
          break;
        default:
          THROW_ED247_ERROR("Unexpected NAD size: " << element_size);
        }
        pos += element_size;
      }
    }
  }
}


ed247::StreamAssistant::StreamAssistant(ed247::Stream* stream):
  _stream(stream),
  _buffer(stream->get_sample_max_size_bytes())
{
  MEMCHECK_NEW(this, "StreamAssistant");
}

ed247::StreamAssistant::~StreamAssistant()
{
  MEMCHECK_DEL(this, "StreamAssistant");
}

//
// Fixed StreamAssistant
//

ed247::FixedStreamAssistant::FixedStreamAssistant(ed247::Stream* stream):
  StreamAssistant(stream)
{
  _buffer.set_size(_stream->get_sample_max_size_bytes());
}

bool ed247::FixedStreamAssistant::write(const ed247::Signal& signal, const void* data, uint32_t size)
{
  if (size > signal.get_sample_max_size_bytes()) {
    PRINT_ERROR("Stream '" << _stream->get_name() << "': Cannot write Signal [" << signal.get_name() << "]: invalid size: " << size);
    return false;
  }

  swap_copy((const char*) data, _buffer.data_rw() + signal.get_byte_offset(), size, signal.get_nad_type());

  return true;
}

bool ed247::FixedStreamAssistant::push(const ed247_timestamp_t* data_timestamp, bool* full)
{
  if(!(_stream->get_direction() & ED247_DIRECTION_OUT)) {
    PRINT_ERROR("Stream '" << _stream->get_name() << "': Cannot push to a non-output stream");
    return false;
  }
  return _stream->push_sample(_buffer.data(), _buffer.size(), data_timestamp, full);
}


bool ed247::FixedStreamAssistant::read(const ed247::Signal& signal, const void** data, uint32_t* size)
{
  *data = _buffer.data() + signal.get_byte_offset();
  *size = signal.get_sample_max_size_bytes();
  return true;
}


ed247_status_t ed247::FixedStreamAssistant::pop(const ed247_timestamp_t** data_timestamp, const ed247_timestamp_t** recv_timestamp,
                                                const ed247_sample_details_t** frame_details, bool* empty)
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

  for(auto signal : _stream->get_signals()) {
    uint32_t byte_offset = signal->get_byte_offset();
    swap_copy(sample.data() + byte_offset, _buffer.data_rw() + byte_offset, signal->get_sample_max_size_bytes(), signal->get_nad_type());
  }

  return ED247_STATUS_SUCCESS;
}



//
// VNAD StreamAssistant
//


ed247::VNADStreamAssistant::VNADStreamAssistant(ed247::Stream* stream):
  StreamAssistant(stream)
{
  for(auto signal : stream->get_signals()) {
    _signal_samples.emplace(std::make_pair<uint32_t, Sample>(signal->get_vnad_position(), Sample(signal->get_sample_max_size_bytes())));
  }
}

bool ed247::VNADStreamAssistant::write(const ed247::Signal& signal, const void* data, uint32_t size)
{
  if (_signal_samples[signal.get_vnad_position()].copy(data, size) == false) {
    PRINT_ERROR("Stream '" << _stream->get_name() << "': Cannot write Signal [" << signal.get_name() << "]: invalid size: " << size);
    return false;
  }

  return true;
}

bool ed247::VNADStreamAssistant::push(const ed247_timestamp_t* data_timestamp, bool* full)
{
  if(!(_stream->get_direction() & ED247_DIRECTION_OUT)) {
    PRINT_ERROR("Stream '" << _stream->get_name() << "': Cannot push to a non-output stream");
    return false;
  }

  uint32_t buffer_index = 0;
  for(auto signal : _stream->get_signals())
  {
    Sample& signal_sample = _signal_samples[signal->get_vnad_position()];

    *(uint16_t*)(_buffer.data_rw() + buffer_index) = (uint16_t)htons((uint16_t)signal_sample.size());
    buffer_index += sizeof(uint16_t);

    swap_copy(signal_sample.data(), _buffer.data_rw() + buffer_index, signal_sample.size(), signal->get_nad_type());
    buffer_index += signal_sample.size();

    signal_sample.reset();
  }

  _buffer.set_size(buffer_index);
  return _stream->push_sample(_buffer.data(), _buffer.size(), data_timestamp, full);
}

bool ed247::VNADStreamAssistant::read(const ed247::Signal& signal, const void** data, uint32_t* size)
{
  Sample& signal_sample = _signal_samples[signal.get_vnad_position()];
  *data = signal_sample.data();
  *size = signal_sample.size();
  return true;
}

ed247_status_t ed247::VNADStreamAssistant::pop(const ed247_timestamp_t** data_timestamp, const ed247_timestamp_t** recv_timestamp,
                                               const ed247_sample_details_t** frame_details, bool* empty)
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

  uint32_t buffer_index = 0;
  for(auto signal: _stream->get_signals())
  {
    uint32_t signal_size = ntohs(*(uint16_t*)((const char*)sample.data() + buffer_index));
    buffer_index += sizeof(uint16_t);

    if (signal_size > signal->get_sample_max_size_bytes()) {
      PRINT_ERROR("Signal '" << signal->get_name() << "': invalid VNAD size : " << signal_size);
      return ED247_STATUS_FAILURE;
    }

    Sample& signal_sample = _signal_samples[signal->get_vnad_position()];
    swap_copy((const char*)sample.data() + buffer_index, signal_sample.data_rw(), signal_size, signal->get_nad_type());
    signal_sample.set_size(signal_size);
    buffer_index += signal_size;
  }

  return ED247_STATUS_SUCCESS;
}
