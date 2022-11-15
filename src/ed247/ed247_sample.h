/* -*- mode: c++; c-basic-offset: 2 -*-  */
/* sample : store a payload in a preallocated memory */
#ifndef _ED247_SAMPLE_H_
#define _ED247_SAMPLE_H_
#include "ed247_logs.h"

namespace ed247
{

  //
  // Simple preallocated buffer
  //
  class Sample
  {
  public:
    Sample() : _data(nullptr), _size(0), _capacity(0)
    {
      // TODO: validate free (curently streams are not freed, so neither their samples)
      // MEMCHECK_NEW(this, "Sample");
    }
    Sample(uint32_t capacity) : _data(nullptr), _size(0), _capacity(0)
    {
      // TODO: validate free (curently streams are not freed, so neither their samples)
      // MEMCHECK_NEW(this, "Sample");
      allocate(capacity);
    }

    ~Sample()
    {
      // TODO: validate free (curently streams are not freed, so neither their samples)
      // MEMCHECK_DEL(this, "Sample");
      delete[] _data;
    }

    Sample(const Sample & other) = delete;
    Sample& operator = (const Sample & other) = delete;

    // Manage internal memory
    void allocate(uint32_t capacity);
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

  //
  // Preallocated buffer with stream informations
  //
  class StreamSample : public Sample
  {
  public:
    StreamSample() :
      _data_timestamp(LIBED247_TIMESTAMP_DEFAULT),
      _recv_timestamp(LIBED247_TIMESTAMP_DEFAULT),
      _frame_infos(LIBED247_SAMPLE_DETAILS_DEFAULT)
    {}

    StreamSample(uint32_t capacity) :
      Sample(capacity),
      _data_timestamp(LIBED247_TIMESTAMP_DEFAULT),
      _recv_timestamp(LIBED247_TIMESTAMP_DEFAULT),
      _frame_infos(LIBED247_SAMPLE_DETAILS_DEFAULT)
    {}

    StreamSample(const StreamSample & other) = delete;
    StreamSample & operator = (const StreamSample & other) = delete;

    using Sample::copy;

    // Copy a sample. Return false if capacity() is too small.
    bool copy(const StreamSample & sample);

    void set_data_timestamp(const ed247_timestamp_t& data_timestamp) { _data_timestamp = data_timestamp; }
    void set_recv_timestamp(const ed247_timestamp_t& recv_timestamp) { _recv_timestamp = recv_timestamp; }
    void set_frame_infos(const ed247_sample_details_t& frame_infos)  { _frame_infos = frame_infos;       }
    const ed247_timestamp_t& data_timestamp() const   { return _data_timestamp; }
    const ed247_timestamp_t& recv_timestamp() const   { return _recv_timestamp; }
    const ed247_sample_details_t& frame_infos() const { return _frame_infos;    }

    // Set recive timestamp to now using default date function or user provided one
    void update_recv_timestamp() { ed247_get_receive_timestamp(&_recv_timestamp); }

    // Update frame information (ECID, SN, transport timestamp)
    void update_frame_infos(ed247_uid_t              component_identifier,
                            uint16_t                 sequence_number,
                            const ed247_timestamp_t& transport_timestamp);
    void clear_frame_infos();

  protected:
    ed247_timestamp_t      _data_timestamp;
    ed247_timestamp_t      _recv_timestamp;
    ed247_sample_details_t _frame_infos;
  };

}

#endif
