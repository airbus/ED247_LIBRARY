/* -*- mode: c++; c-basic-offset: 2 -*-  */
#include "ed247_header.h"
#include "ed247_logs.h"
#include "ed247_bswap.h"
#include <algorithm>


uint32_t ed247::FrameHeader::length()
{
  return _configuration._enable == ED247_YESNO_YES ? (sizeof(uint16_t)*2+sizeof(uint32_t)*2) : 0;
}

void ed247::FrameHeader::encode(char * frame, uint32_t frame_capacity, uint32_t & frame_index, ed247_uid_t component_identifier)
{
  if(_configuration._enable == ED247_YESNO_YES){
    if(_configuration._transport_timestamp == ED247_YESNO_YES) {
      ed247_get_transport_timestamp(&_send_header.transport_timestamp);
    }

    memset(frame+frame_index, 0, sizeof(uint16_t)+sizeof(uint16_t)+sizeof(uint32_t)+sizeof(uint32_t));
    if(frame_index + sizeof(uint16_t) > frame_capacity) {
      THROW_ED247_ERROR("Channel '" << _channel_name << "': Failed to write producer identifier in header (not enough space). Size: " << frame_capacity);
    }
    *(uint16_t*)(frame+frame_index) = htons(component_identifier);
    frame_index += sizeof(uint16_t);
    if(frame_index + sizeof(uint16_t) > frame_capacity) {
      THROW_ED247_ERROR("Channel '" << _channel_name << "': ailed to write sequence number in header (not enough space). Size: " << frame_capacity);
    }
    *(uint16_t*)(frame+frame_index) = htons(_send_header.sequence_number);
    _send_header.sequence_number++;
    frame_index += sizeof(uint16_t);
    if(_configuration._transport_timestamp == ED247_YESNO_YES){
      if(frame_index + sizeof(ed247_timestamp_t) > frame_capacity) {
        THROW_ED247_ERROR("Channel '" << _channel_name << "': Failed to write Transport timestamp in header (not enough space). Size: " << frame_capacity);
      }
      *(uint32_t*)(frame+frame_index) = htonl(_send_header.transport_timestamp.epoch_s);
      frame_index += sizeof(uint32_t);
      *(uint32_t*)(frame+frame_index) = htonl(_send_header.transport_timestamp.offset_ns);
      frame_index += sizeof(uint32_t);
    }else{
      memset(frame+frame_index, 0, 2*sizeof(uint32_t));
      frame_index += 2*sizeof(uint32_t);
    }
  }
}

bool ed247::FrameHeader::decode(const char * frame, uint32_t frame_size, uint32_t & frame_index)
{
  frame_index = 0;
  // Increment the regular number of the sequence number
  // This number shall be rewriten if the sequence number is present in the header
  // In case of frames without the sequence number, it allows to count the number of frames that are discarded (missformated/corrupted)
  if(_configuration._enable == ED247_YESNO_YES){
    static header_element_t recv_header;
    if(frame_index + sizeof(uint16_t) > frame_size) {
      PRINT_ERROR("Channel '" << _channel_name << "': Invalid frame size: " << frame_size);
      return false;
    }
    recv_header.component_identifier = ntohs(*(uint16_t*)(frame+frame_index));
    frame_index += sizeof(uint16_t);
    if(frame_index + sizeof(uint16_t) > frame_size) {
      PRINT_ERROR("Channel '" << _channel_name << "': Invalid frame size: " << frame_size);
      return false;
    }
    recv_header.sequence_number = ntohs(*(uint16_t*)(frame+frame_index));
    frame_index += sizeof(uint16_t);
    if(_configuration._transport_timestamp == ED247_YESNO_YES){
      if(frame_index + sizeof(ed247_timestamp_t) > frame_size) {
        PRINT_ERROR("Channel '" << _channel_name << "': Invalid frame size: " << frame_size);
        return false;
      }
      recv_header.transport_timestamp.epoch_s = ntohl(*(uint32_t*)(frame+frame_index));
      frame_index += sizeof(uint32_t);
      recv_header.transport_timestamp.offset_ns = ntohl(*(uint32_t*)(frame+frame_index));
      frame_index += sizeof(uint32_t);
    }else{
      recv_header.transport_timestamp.epoch_s = 0;
      recv_header.transport_timestamp.offset_ns = 0;
      frame_index += 2*sizeof(uint32_t);
    }
    // Check header elements
    uint16_t component_identifier = recv_header.component_identifier;
    _recv_headers_iter = std::find_if(_recv_headers.begin(), _recv_headers.end(), [&component_identifier](header_element_t & e)->bool{
      return e.component_identifier == component_identifier;
    });
    if(_recv_headers_iter == _recv_headers.end()){
      if(_recv_headers.size() >= _recv_headers.capacity()) {
        // Library limitation: max number of producer reached
        THROW_ED247_ERROR("Channel '" << _channel_name << "': No more producer allowed for reception. Max : " << _recv_headers.capacity());
      }
      recv_header.missed_frames = 0;
      _recv_headers.push_back(recv_header);
      _recv_headers_iter = _recv_headers.end()-1;
    }else{
      uint32_t missed = recv_header.sequence_number > _recv_headers_iter->sequence_number ?
        (recv_header.sequence_number - _recv_headers_iter->sequence_number - 1):
        (0xFFFF - _recv_headers_iter->sequence_number + recv_header.sequence_number);
      _recv_headers_iter->missed_frames = _recv_headers_iter->missed_frames > 0xFFFF - missed ?
        0xFFFF :
        (_recv_headers_iter->missed_frames + missed);
      _recv_headers_iter->sequence_number = recv_header.sequence_number;
      _recv_headers_iter->transport_timestamp = recv_header.transport_timestamp;
    }
  }
  return true;
}
