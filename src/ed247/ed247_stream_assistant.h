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

    virtual bool write(const Signal& signal, const void* data, uint32_t size) = 0;
    virtual bool read(const Signal& signal, const void** data, uint32_t* size) = 0;

    virtual bool push(const ed247_timestamp_t* data_timestamp, bool* full) = 0;
    virtual ed247_status_t pop(const ed247_timestamp_t** data_timestamp, const ed247_timestamp_t** recv_timestamp,
                               const ed247_sample_details_t** frame_details, bool* empty) = 0;

  protected:
    Stream* _stream;
    Sample  _buffer;  // WARN: buffer content depend on stream type and direction for performances reasons

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
