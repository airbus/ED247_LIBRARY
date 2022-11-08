/* -*- mode: c++; c-basic-offset: 2 -*-  */
#include "ed247_sample.h"
#include "ed247_logs.h"


bool ed247::BaseSample::copy(const char* data, const uint32_t& size)
{
  if (size > _capacity) {
    PRINT_DEBUG("ERROR: Cannot copy payload: internal buffer is too small (" << size << " > " << _capacity << ")");
    return false;
  }
  _size = size;
  memcpy(_data, data, _size);
  return true;
}

void ed247::BaseSample::allocate(uint32_t capacity)
{
  _capacity = capacity;
  if(_data != nullptr || _size != 0) THROW_ED247_ERROR("Sample already allocated");
  if(!_capacity) THROW_ED247_ERROR("Cannot allocate a sample with a capacity of 0");
  _data = new char[capacity];
  if(!_data) THROW_ED247_ERROR("Failed to allocate sample [" << _capacity <<"] !");
  memset(_data, 0, _capacity);
  _size = 0;
}

void ed247::BaseSample::deallocate()
{
  if(_data) delete[] _data;
  _data = nullptr;
}
