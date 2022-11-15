/* -*- mode: c++; c-basic-offset: 2 -*-  */
#include "ed247_sample.h"

bool ed247::Sample::copy(const char* data, const uint32_t& size)
{
  if (size > _capacity) {
    PRINT_DEBUG("ERROR: Cannot copy payload: internal buffer is too small (" << size << " > " << _capacity << ")");
    return false;
  }
  _size = size;
  memcpy(_data, data, _size);
  return true;
}

void ed247::Sample::allocate(uint32_t capacity)
{
  _capacity = capacity;
  if(_data != nullptr || _size != 0) THROW_ED247_ERROR("Sample already allocated");
  if(!_capacity) THROW_ED247_ERROR("Cannot allocate a sample with a capacity of 0");
  _data = new char[capacity];
  if(!_data) THROW_ED247_ERROR("Failed to allocate sample [" << _capacity <<"] !");
  memset(_data, 0, _capacity);
  _size = 0;
}



bool ed247::StreamSample::copy(const StreamSample & sample)
{
  if (Sample::copy(sample.data(), sample.size()) == false) return false;
  set_data_timestamp(sample.data_timestamp());
  set_recv_timestamp(sample.recv_timestamp());
  set_frame_infos(sample.frame_infos());
  return true;
}


void ed247::StreamSample::clear_frame_infos()
{
  _frame_infos.component_identifier = 0;
  _frame_infos.sequence_number = 0;
  _frame_infos.transport_timestamp.epoch_s = 0;
  _frame_infos.transport_timestamp.offset_ns = 0;
}

void ed247::StreamSample::update_frame_infos(ed247_uid_t              component_identifier,
                                             uint16_t                 sequence_number,
                                             const ed247_timestamp_t& transport_timestamp)
{
  _frame_infos.component_identifier = component_identifier;
  _frame_infos.sequence_number = sequence_number;
  _frame_infos.transport_timestamp = transport_timestamp;
}
