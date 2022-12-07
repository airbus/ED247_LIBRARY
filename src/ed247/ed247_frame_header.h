/* -*- mode: c++; c-basic-offset: 2 -*-  */
#ifndef _ED247_HEADER_H_
#define _ED247_HEADER_H_
#include "ed247.h"
#include "ed247_xml.h"

namespace ed247
{
  class FrameHeader
  {
  public:
    FrameHeader(const xml::Header& configuration, ed247_uid_t ec_id, const std::string& channel_name);

    void encode(char* frame, uint32_t frame_size, uint32_t& frame_index /*inout*/);
    bool decode(const char* frame, uint32_t frame_size, uint32_t& frame_index /*inout*/);

    uint32_t get_size() const                                    { return (_configuration._enable == ED247_YESNO_YES)? header_size : 0; }
    const ed247_sample_details_t& get_recv_frame_details() const { return _recv_frame_details;                                          }
    uint16_t get_next_serial_number() const                      { return _send_sn;                                                     }

  private:
    xml::Header            _configuration;
    ed247_uid_t            _ec_id_big_endian;
    std::string            _channel_name;
    ed247_sample_details_t _recv_frame_details;
    uint16_t               _send_sn;

    // size in bytes if header present
    static const uint32_t header_size;
  };
}

#endif
