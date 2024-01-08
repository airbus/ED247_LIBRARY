/* -*- mode: c++; c-basic-offset: 2 -*-  */
#ifndef _ED247_STREAM_ASSISTANT_H_
#define _ED247_STREAM_ASSISTANT_H_
#include "ed247.h"
#include "ed247_xml.h"
#include "ed247_signal.h"
#include "ed247_sample.h"


// base structures for C API
struct ed247_internal_stream_assistant_t {
  virtual ~ed247_internal_stream_assistant_t() {}
};


namespace ed247
{
  class Stream;

  class StreamAssistant : public ed247_internal_stream_assistant_t
  {
  public:

    StreamAssistant(Stream* stream);
    virtual ~StreamAssistant();

    Stream* get_api_stream() { return _stream; }

    // Write a signal to the stream
    virtual bool write(const Signal& signal, const void* data, uint32_t size) = 0;
    // Read a signal from the stream
    virtual bool read(const Signal& signal, const void** data, uint32_t* size) = 0;

    // See stream::push_sample() for details
    virtual bool push(const ed247_timestamp_t* data_timestamp, bool* full) = 0;
    // See stream::pop_sample() for details
    virtual ed247_status_t pop(const ed247_timestamp_t** data_timestamp, const ed247_timestamp_t** recv_timestamp,
                               const ed247_sample_details_t** frame_details, bool* empty) = 0;

    // Check is some signals han been written since last push
    bool was_written() { return _was_written; }

    // Push data only if was_written(). See stream::push_sample() for details
    bool push_if_was_written(const ed247_timestamp_t* data_timestamp, bool* full);

  protected:
    Stream* _stream;
    Sample  _buffer;          // WARN: buffer content depend on stream type and direction for performances reasons
    bool    _was_written;     // true if some signals has been written since last push

    ED247_FRIEND_TEST();
  };


  class FixedStreamAssistant : public StreamAssistant
  {
  public:
    FixedStreamAssistant(Stream* stream);

    virtual bool write(const Signal& signal, const void* data, uint32_t size) override;
    virtual bool read(const Signal& signal, const void** data, uint32_t * size) override;

    virtual bool push(const ed247_timestamp_t* data_timestamp, bool* full) override;
    virtual ed247_status_t pop(const ed247_timestamp_t** data_timestamp, const ed247_timestamp_t** recv_timestamp,
                               const ed247_sample_details_t** frame_details, bool* empty) override;
  };

  class VNADStreamAssistant : public StreamAssistant
  {
  public:
    VNADStreamAssistant(Stream* stream);

    virtual bool write(const Signal& signal, const void* data, uint32_t size) override;
    virtual bool read(const Signal& signal, const void** data, uint32_t * size) override;

    virtual bool push(const ed247_timestamp_t* data_timestamp, bool* full) override;
    virtual ed247_status_t pop(const ed247_timestamp_t** data_timestamp, const ed247_timestamp_t** recv_timestamp,
                               const ed247_sample_details_t** frame_details, bool* empty) override;

  private:
    // signal position -> Sample.
    // Position may not be continuous so we cannot use a vector
    std::unordered_map<uint32_t, Sample> _signal_samples;
  };

}

#endif
