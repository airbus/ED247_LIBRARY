/* -*- mode: c++; c-basic-offset: 2 -*-  */
#ifndef _ED247_HEADER_H_
#define _ED247_HEADER_H_
#include "ed247.h"
#include "ed247_xml.h"

namespace ed247
{
  typedef uint16_t A664_sample_size_t;
  typedef uint8_t  A825_sample_size_t;
  typedef uint16_t SERIAL_sample_size_t;
  typedef uint16_t AUDIO_sample_size_t;
  typedef uint16_t VNAD_sample_size_t;


  class FrameHeader
  {
  public:
    typedef struct {
      uint16_t            component_identifier;
      uint16_t            sequence_number;
      ed247_timestamp_t   transport_timestamp;
      uint32_t            missed_frames;
    } header_element_t;

    static const uint16_t MAX_PID_SN_TRACKER { 64 };

    FrameHeader(const xml::Header & configuration, const std::string& channel_name):
      _send_header({0, 0, {0, 0}, 0}),
      _configuration(configuration),
      _channel_name(channel_name)
    {
      _recv_headers.reserve(MAX_PID_SN_TRACKER);
      _recv_headers_iter = _recv_headers.end();
    }
    FrameHeader(const FrameHeader & other):
      _send_header(other._send_header),
      _recv_headers(other._recv_headers),
      _recv_headers_iter(other._recv_headers_iter),
      _configuration(other._configuration)
    {}

    // Return false on error
    void encode(char * frame, uint32_t frame_capacity, uint32_t & frame_index, ed247_uid_t component_identifier);
    bool decode(const char * frame, uint32_t frame_size, uint32_t & frame_index);

    uint32_t length();

    bool operator == (const FrameHeader & other) const
    {
      return _send_header.component_identifier == other._send_header.component_identifier &&
        _send_header.sequence_number == other._send_header.sequence_number &&
        _send_header.transport_timestamp.epoch_s == other._send_header.transport_timestamp.epoch_s &&
        _send_header.transport_timestamp.offset_ns == other._send_header.transport_timestamp.offset_ns &&
        _send_header.missed_frames == other._send_header.missed_frames;
    }

    bool operator != (const FrameHeader & other) const
    {
      return !operator==(other);
    }

    uint32_t missed_frames()
    {
      uint32_t missed_frames = 0;
      for(auto & recv_header : _recv_headers){
        missed_frames += recv_header.missed_frames;
      }
      return missed_frames;
    }

    header_element_t _send_header;
    std::vector<header_element_t> _recv_headers;
    std::vector<header_element_t>::iterator _recv_headers_iter;

  private:
    xml::Header _configuration;
    std::string _channel_name;
  };
}

#endif
