/* -*- mode: c++; c-basic-offset: 2 -*-  */
#include "ed247_frame_header.h"
#include "ed247_logs.h"
#include "ed247_bswap.h"


const uint32_t ed247::FrameHeader::header_size =
  sizeof(uint16_t) +                             // EC Identifier
  sizeof(uint16_t) +                             // Sequence Number
  sizeof(uint32_t) + sizeof(uint32_t);           // Transport Timestamp


ed247::FrameHeader::FrameHeader(const xml::Header& configuration, ed247_uid_t ec_id, const std::string& channel_name) :
  _configuration(configuration),
  _ec_id_big_endian(htons(ec_id)),
  _channel_name(channel_name),
  _recv_frame_details(LIBED247_SAMPLE_DETAILS_DEFAULT),
  _send_sn(0)
{
}

void ed247::FrameHeader::encode(char* frame, uint32_t frame_size, uint32_t& frame_index)
{
  if(_configuration._enable == ED247_YESNO_YES) {
    if ((frame_size - frame_index) < header_size) {
      THROW_ED247_ERROR("Channel '" << _channel_name << "': Not enougth space to write header.");
    }

    *(uint16_t*)(frame + frame_index) = _ec_id_big_endian;
    frame_index += sizeof(uint16_t);

    *(uint16_t*)(frame + frame_index) = htons(_send_sn);
    frame_index += sizeof(uint16_t);
    _send_sn++;

    if(_configuration._transport_timestamp == ED247_YESNO_YES) {
      ed247_timestamp_t transport_timestamp;
      ed247_get_transport_timestamp(&transport_timestamp);
      *(uint32_t*)(frame + frame_index                   ) = htonl(transport_timestamp.epoch_s);
      *(uint32_t*)(frame + frame_index + sizeof(uint32_t)) = htonl(transport_timestamp.offset_ns);
    } else {
      memset(frame + frame_index, 0, 2 * sizeof(uint32_t));
    }
    frame_index += 2 * sizeof(uint32_t);
  }
}

bool ed247::FrameHeader::decode(const char * frame, uint32_t frame_size, uint32_t & frame_index)
{
  if(_configuration._enable == ED247_YESNO_YES) {
    if ((frame_size - frame_index) < header_size) {
      PRINT_ERROR("Channel '" << _channel_name << "': Received frame is too small. Size: " << frame_size);
      return false;
    }

    _recv_frame_details.component_identifier = ntohs(*(uint16_t*)(frame + frame_index));
    frame_index += sizeof(uint16_t);

    _recv_frame_details.sequence_number = ntohs(*(uint16_t*)(frame + frame_index));
    frame_index += sizeof(uint16_t);

    if(_configuration._transport_timestamp == ED247_YESNO_YES) {
      _recv_frame_details.transport_timestamp.epoch_s   = ntohl(*(uint32_t*)(frame + frame_index                   ));
      _recv_frame_details.transport_timestamp.offset_ns = ntohl(*(uint32_t*)(frame + frame_index + sizeof(uint32_t)));
    }
    frame_index += 2 * sizeof(uint32_t);
  }
  return true;
}
