/* -*- mode: c++; c-basic-offset: 2 -*-  */
/* sample : store a payload in a preallocated memory */
#ifndef _ED247_SAMPLE_H_
#define _ED247_SAMPLE_H_
#include "ed247.h"
#include <vector>

namespace ed247
{

  //
  // Simple preallocated buffer
  //
  class Sample
  {
  public:
    Sample(uint32_t capacity);
    ~Sample();

    // Empty Ctor. Call allocate() before any other functions.
    Sample();

    // No implicit copy. Use copy() methods.
    Sample(const Sample & other) = delete;
    Sample& operator = (const Sample & other) = delete;

    // Can be moved
    Sample(Sample&& other);

    // Allocate data. Shall be called only once and only if Empty Ctor is used.
    void allocate(uint32_t capacity);

    const uint32_t& size() const     { return _size;      }
    const uint32_t& capacity() const { return _capacity;  }
    const char* data() const         { return _data;      }
    bool empty() const               { return _size == 0; }

    // Fill data & size. Return false if capacity() is too small.
    bool copy(const char* data, const uint32_t& size);
    bool copy(const void* data, const uint32_t& size) { return copy((const char*)data, size); }

    // Direct access. You have to check capacity() yourself.
    char* data_rw()                       { return _data; }
    void set_size(const uint32_t & size)  { _size = size; }
    void reset()                          { _size = 0;    }

  private:
    char*    _data;
    uint32_t _size;
    uint32_t _capacity;
  };

  //
  // Preallocated buffer with stream informations
  //
  class StreamSample : public Sample
  {
  public:
    StreamSample(uint32_t capacity) :
      Sample(capacity),
      _data_timestamp(LIBED247_TIMESTAMP_DEFAULT),
      _recv_timestamp(LIBED247_TIMESTAMP_DEFAULT),
      _frame_details(LIBED247_SAMPLE_DETAILS_DEFAULT)
    {}

    // No implicit copy. Use copy() methods.
    StreamSample(const StreamSample & other) = delete;
    StreamSample & operator = (const StreamSample & other) = delete;

    // Can be moved
    StreamSample(StreamSample&& other) :
      Sample(std::move(other)),
      _data_timestamp(std::move(other._data_timestamp)),
      _recv_timestamp(std::move(other._recv_timestamp)),
      _frame_details(std::move(other._frame_details))
    {}


    using Sample::copy;

    // Copy a sample. Return false if capacity() is too small.
    bool copy(const StreamSample & sample);

    void set_data_timestamp(const ed247_timestamp_t& data_timestamp)    { _data_timestamp = data_timestamp; }
    void set_recv_timestamp(const ed247_timestamp_t& recv_timestamp)    { _recv_timestamp = recv_timestamp; }
    void set_frame_details(const ed247_sample_details_t& frame_details) { _frame_details = frame_details;   }
    const ed247_timestamp_t& data_timestamp() const     { return _data_timestamp; }
    const ed247_timestamp_t& recv_timestamp() const     { return _recv_timestamp; }
    const ed247_sample_details_t& frame_details() const { return _frame_details;  }

    // Set recive timestamp to now using default date function or user provided one
    void update_recv_timestamp() { ed247_get_receive_timestamp(&_recv_timestamp); }

  protected:
    ed247_timestamp_t      _data_timestamp;
    ed247_timestamp_t      _recv_timestamp;
    ed247_sample_details_t _frame_details;

  private:
    using Sample::allocate; // Delete
  };


  //
  // preallocated ring buffer
  //
  class StreamSampleRingBuffer {
  public:
    StreamSampleRingBuffer(uint32_t capacity, uint32_t samples_capacity);
    ~StreamSampleRingBuffer();

    uint32_t capacity() const         { return _samples.size();                }
    uint32_t samples_capacity() const { return _samples_capacity;              }
    uint32_t size() const             { return _index_size;                    }
    bool empty() const                { return _index_size == 0;               }
    bool full() const                 { return _index_size >= _samples.size(); }

    // "push" a new sample.
    // If ring buffer is full, override the oldest sample.
    // Since memory is preallocated, return a reference to the "new" sample.
    // Returned sample is not reseted but its content sahll be ignored.
    StreamSample& push_back();

    // Pop the oldest sample.
    // if ring buffer is empty, return an arbitrary sample. (i.e. call empty() before)
    StreamSample& pop_front();

    // Return the oldest sample without removing it.
    // if ring buffer is empty, return an arbitrary sample. (i.e. call empty() before)
    StreamSample& front() { return _samples[_index_read]; }

    // Return the last pushed sample
    // if ring buffer is empty, return an arbitrary sample. (i.e. call empty() before)
    StreamSample& back()
    {
      return _samples[_index_write == 0 ? (_samples.size()-1) : (_index_write-1)];
    }

    // Return the oldest + index sample
    // index is not checked. May have undefined behavior.
    StreamSample& at(uint32_t index)
    {
      return _samples[(_index_read + index) % _samples.size()];
    }

  private:
    std::vector<StreamSample>  _samples;
    uint32_t                   _samples_capacity;
    uint32_t                   _index_read;
    uint32_t                   _index_write;
    uint32_t                   _index_size;
  };

}

#endif
