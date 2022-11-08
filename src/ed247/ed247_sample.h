/* -*- mode: c++; c-basic-offset: 2 -*-  */
/* sample : store a payload in a preallocated memory */
#ifndef _ED247_SAMPLE_H_
#define _ED247_SAMPLE_H_
#include <cstdint>

namespace ed247
{
  class BaseSample
  {
  public:
    BaseSample() : _data(nullptr), _size(0), _capacity(0) { }
    BaseSample(const BaseSample & other) = delete;
    BaseSample& operator = (const BaseSample & other) = delete;

    // Manage internal memory
    void allocate(uint32_t capacity);
    void deallocate();
    bool allocated() const { return _data != nullptr; }

    // Accessor
    const uint32_t& size() const     { return _size;      }
    const uint32_t& capacity() const { return _capacity;  }
    const char* data() const         { return _data;      }
    bool empty() const               { return _size == 0; }

    // Fill data & size. Return false if capacity() is too small.
    bool copy(const char* data, const uint32_t& size);
    bool copy(const void* data, const uint32_t& size) { return copy((const char*)data, size); }

    // Erase data
    void reset() { _size = 0; }

    // Direct access. You have to check capacity() yourself.
    char* data_rw()                       { return _data; }
    void set_size(const uint32_t & size)  { _size = size; }

  private:
    char*    _data;
    uint32_t _size;
    uint32_t _capacity;
  };

}

#endif
