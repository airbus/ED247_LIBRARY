/* -*- mode: c++; c-basic-offset: 2 -*-  */
/******************************************************************************
 * The MIT Licence
 *
 * Copyright (c) 2021 Airbus Operations S.A.S
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *****************************************************************************/
#include "ed247_stream.h"
#include "ed247_stream_assistant.h"
#include "cpp_14.h"
#include <regex>


//
// Stream
//

ed247::Stream::Stream(const ed247::xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel, uint32_t sample_header_size):
  _configuration(configuration),
  _ed247_api_channel(ed247_api_channel),
  _recv_stack(_configuration->_sample_max_number, _configuration->_sample_max_size_bytes),
  _recv_working_sample(std::make_shared<StreamSample>(_configuration->_sample_max_size_bytes)),
  _send_stack(_configuration->_sample_max_number, _configuration->_sample_max_size_bytes),
  _send_working_sample(std::make_shared<StreamSample>(_configuration->_sample_max_size_bytes)),
  _working_sample(_configuration->_sample_max_size_bytes),
  _signals(std::make_shared<signal_list_t>()),
  _user_data(NULL)
{
  _max_size = _configuration->_sample_max_number * (_configuration->_sample_max_size_bytes + sample_header_size);
  if(_configuration->_data_timestamp._enable == ED247_YESNO_YES) {
    _max_size += sizeof(ed247_timestamp_t) +                              // First sample: full DTS
      sizeof(uint32_t) * (_configuration->_sample_max_number - 1);        // Next samples: DTS offset
  }
}

ed247::StreamSignals::StreamSignals(const xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel, signal_set_t& context_signal_set, uint32_t sample_header_size) :
  Stream(configuration, ed247_api_channel, sample_header_size)
{
  const xml::StreamSignals* sconfiguration = (const xml::StreamSignals*)configuration;
  for(auto& signal_configuration : sconfiguration->_signal_list) {
    signal_ptr_t signal = context_signal_set.create(signal_configuration.get(), this);
    _signals->push_back(signal);
  }
  _assistant = std::make_shared<StreamAssistant>(this);
}



namespace ed247
{


// Stream
  bool Stream::push_sample(const void * sample_data, uint32_t sample_size, const ed247_timestamp_t * data_timestamp, bool * full)
  {
    if(!(_configuration->_direction & ED247_DIRECTION_OUT)) {
      PRINT_ERROR("Stream '" << get_name() << "': Cannot write sample on a stream which is not an output one");
      return false;
    }
    if(sample_size > _configuration->_sample_max_size_bytes) {
      PRINT_ERROR("Stream '" << get_name() << "': Invalid sample size (" << sample_size << ")");
      return false;
    }
    StreamSample& sample = _send_stack.push_back();
    sample.copy(sample_data, sample_size);
    if(data_timestamp) sample.set_data_timestamp(*data_timestamp);
    if(full) *full = _send_stack.full();
    return true;
  }

  StreamSample& Stream::pop_sample(bool *empty)
  {
    StreamSample& result = _recv_stack.pop_front();
    if (empty) *empty = _recv_stack.empty();
    return result;
  }

  signal_list_t Stream::find_signals(std::string str_regex)
  {
    std::regex reg(str_regex);
    signal_list_t founds;
    for(auto signal : *_signals){
      if(std::regex_match(signal->get_name(), reg)){
        founds.push_back(signal);
      }
    }
    return founds;
  }

  signal_ptr_t Stream::get_signal(std::string str_name)
  {
    for(auto signal : *_signals){
      if(signal->get_name() == str_name) return signal;
    }
    return nullptr;
  }

  std::unique_ptr<StreamSample> Stream::allocate_sample() const
  {
    return std::unique_ptr<StreamSample>(new StreamSample(_configuration->_sample_max_size_bytes));
  }

  ed247_status_t Stream::check_sample_size(uint32_t sample_size) const
  {
    return sample_size <= _configuration->_sample_max_size_bytes ? ED247_STATUS_SUCCESS : ED247_STATUS_FAILURE;
  }


// StreamSet

  StreamSet::StreamSet():
    _streams(std::make_shared<stream_list_t>())
  {
  }

  StreamSet::StreamSet(std::shared_ptr<ed247::signal_set_t> & pool_signals):
    _streams(std::make_shared<stream_list_t>()),
    _pool_signals(pool_signals)
  {
  }

  stream_ptr_t StreamSet::get(const xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel)
  {
    PRINT_DEBUG("Create stream [" << configuration->_name << "] ...");

    std::string name{configuration->_name};
    if (std::find_if(_streams->begin(), _streams->end(), [&name](const stream_ptr_t & s){ return s->get_name() == name; }) != _streams->end()) {
      THROW_ED247_ERROR("Stream [" << name << "] already exists !");
    }

    ed247::stream_ptr_t stream;

    switch (configuration->_type) {
    case ED247_STREAM_TYPE_A429:      stream = std::make_shared<StreamA429>(configuration, ed247_api_channel);     break;
    case ED247_STREAM_TYPE_A664:      stream = std::make_shared<StreamA664>(configuration, ed247_api_channel);     break;
    case ED247_STREAM_TYPE_A825:      stream = std::make_shared<StreamA825>(configuration, ed247_api_channel);     break;
    case ED247_STREAM_TYPE_SERIAL:    stream = std::make_shared<StreamSERIAL>(configuration, ed247_api_channel);   break;
    case ED247_STREAM_TYPE_AUDIO:     stream = std::make_shared<StreamAUDIO>(configuration, ed247_api_channel);    break;

    case ED247_STREAM_TYPE_DISCRETE:  stream = std::make_shared<StreamDISCRETE>(configuration, ed247_api_channel, *_pool_signals.get()); break;
    case ED247_STREAM_TYPE_ANALOG:    stream = std::make_shared<StreamANALOG>(configuration, ed247_api_channel, *_pool_signals.get());   break;
    case ED247_STREAM_TYPE_NAD:       stream = std::make_shared<StreamNAD>(configuration, ed247_api_channel, *_pool_signals.get());      break;
    case ED247_STREAM_TYPE_VNAD:      stream = std::make_shared<StreamVNAD>(configuration, ed247_api_channel, *_pool_signals.get());     break;

    case ED247_STREAM_TYPE_ETHERNET:
    case ED247_STREAM_TYPE_VIDEO:
    case ED247_STREAM_TYPE_M1553:
      THROW_ED247_ERROR("Stream '" << configuration->_name << "': Unsupported stream type '" << configuration->_type << "'");
      break;

    case ED247_STREAM_TYPE__INVALID:
    case ED247_STREAM_TYPE__COUNT:
      THROW_ED247_ERROR("Stream '" << configuration->_name << "': Invalid stream type '" << configuration->_type << "'");
      break;
    }

    _streams->push_back(stream);
    return stream;
  }

  stream_list_t StreamSet::find(std::string strregex)
  {
    std::regex reg(strregex);
    stream_list_t founds;
    for(auto stream : *_streams){
      if(std::regex_match(stream->get_name(), reg)){
        founds.push_back(stream);
      }
    }
    return founds;
  }

  stream_ptr_t StreamSet::get(std::string str_name)
  {
    for(auto stream : *_streams){
      if(stream->get_name() == str_name) return stream;
    }
    return nullptr;
  }

  std::shared_ptr<stream_list_t> StreamSet::streams()
  {
    return _streams;
  }

  uint32_t StreamSet::size() const
  {
    return _streams->size();
  }
}


//
// StreamA429
//
ed247::StreamA429::StreamA429(const ed247::xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel) :
  Stream(configuration, ed247_api_channel, 0)
{
}

uint32_t ed247::StreamA429::encode(char * frame, uint32_t frame_size)
{
  if(_send_stack.size() == 0) return 0;
  uint32_t frame_index = 0;
  do{
    const StreamSample& sample = _send_stack.pop_front();

    // Write Data Timestamp and Precise Data Timestamp
    encode_data_timestamp(sample, frame, frame_size, frame_index);

    // Write sample data
    if((frame_index + sample.size()) > frame_size) {
      THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
    }
    memcpy(frame + frame_index, sample.data(), sample.size());
    frame_index += sample.size();
  }while(_send_stack.empty() == false);
  return frame_index;
}

bool ed247::StreamA429::decode(const char * frame, uint32_t frame_size, const ed247::FrameHeader * header)
{
  uint32_t frame_index = 0;
  static ed247_timestamp_t data_timestamp;
  static ed247_timestamp_t timestamp;
  while(frame_index < frame_size){
    // Read Data Timestamp if necessary
    if (decode_data_timestamp(frame, frame_size, frame_index, data_timestamp, timestamp) == false) return false;
    // Read sample data
    if((frame_size-frame_index) < _configuration->_sample_max_size_bytes) {
      PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
      return false;
    }
    StreamSample& sample = _recv_stack.push_back();
    sample.copy(frame+frame_index, _configuration->_sample_max_size_bytes);
    frame_index += sample.size();
    // Update data timestamp
    if(_configuration->_data_timestamp._enable == ED247_YESNO_YES){
      sample.set_data_timestamp(data_timestamp);
    }
    // Update simulation time
    sample.update_recv_timestamp();
    // Attach header
    if(header) {
      if (header->_recv_headers_iter == header->_recv_headers.end()) {
        sample.clear_frame_infos();
      } else {
        sample.update_frame_infos(header->_recv_headers_iter->component_identifier,
                                  header->_recv_headers_iter->sequence_number,
                                  header->_recv_headers_iter->transport_timestamp);
      }
    }
  }
  // Callbacks
  return run_callbacks();
}

//
// StreamA664
//

ed247::StreamA664::StreamA664(const ed247::xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel) :
  Stream(configuration,
         ed247_api_channel,
         (((const xml::A664Stream*)configuration)->_enable_message_size == ED247_YESNO_YES)? sizeof(A664_sample_size_t) : 0
    )
{
}

uint32_t ed247::StreamA664::encode(char * frame, uint32_t frame_size)
{
  auto enable_message_size = ((const xml::A664Stream*)_configuration)->_enable_message_size == ED247_YESNO_YES;
  uint32_t frame_index = 0;
  while (_send_stack.size() > 0) {
    const StreamSample& sample = _send_stack.pop_front();

    // Write Data Timestamp and Precise Data Timestamp
    encode_data_timestamp(sample, frame, frame_size, frame_index);

    // Write sample size
    if(enable_message_size){
      if((frame_index + sizeof(uint16_t)) > frame_size) {
        THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
      }
      if(sample.size() != (uint16_t)sample.size())
        THROW_ED247_ERROR("Stream '" << get_name() << "': Stream data size it too big for a 16 bits number ! (" << sample.size() << ")");
      uint16_t size = htons((uint16_t)sample.size());
      memcpy(frame + frame_index, &size, sizeof(uint16_t));
      frame_index += sizeof(uint16_t);
    }

    // Write sample data
    if((frame_index + sizeof(uint16_t)) > frame_size) {
      THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
    }
    memcpy(frame + frame_index, sample.data(), sample.size());
    frame_index += sample.size();

    // If message size is disabled, we cannot append more sample.
    // Remaining ones will be available for next encode() call.
    if (enable_message_size == false) break;
  };
  return frame_index;
}

bool ed247::StreamA664::decode(const char * frame, uint32_t frame_size, const ed247::FrameHeader * header)
{
  auto enable_message_size = ((const xml::A664Stream*)_configuration)->_enable_message_size == ED247_YESNO_YES;
  uint32_t frame_index = 0;
  uint32_t sample_size = 0;
  static ed247_timestamp_t data_timestamp;
  static ed247_timestamp_t timestamp;
  while(frame_index < frame_size){
    // Read Data Timestamp if necessary
    if (decode_data_timestamp(frame, frame_size, frame_index, data_timestamp, timestamp) == false) return false;
    // Sample size
    if(enable_message_size){
      // Read sample size
      if((frame_size-frame_index) < sizeof(uint16_t)) {
        PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
        return false;
      }
      sample_size = ntohs(*(uint16_t*)(frame+frame_index));
      frame_index += sizeof(uint16_t);
    }else{
      // Assume remaining data is the message
      sample_size = (frame_size-frame_index);
    }
    // Read sample data
    if((frame_size-frame_index) < sample_size) {
      PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
      return false;
    }

    PRINT_CRAZY("AFDX stream '" << get_name() << "' decoded. Size: " << sample_size);
    if (sample_size > _recv_stack.samples_capacity()) {
      PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
      return false;
    }

    StreamSample& sample = _recv_stack.push_back();
    sample.copy(frame+frame_index, sample_size);
    frame_index += sample.size();
    // Update data timestamp
    if(_configuration->_data_timestamp._enable == ED247_YESNO_YES){
      sample.set_data_timestamp(data_timestamp);
    }
    // Update simulation time
    sample.update_recv_timestamp();
    // Attach header
    if(header) {
      if (header->_recv_headers_iter == header->_recv_headers.end()) {
        sample.clear_frame_infos();
      } else {
        sample.update_frame_infos(header->_recv_headers_iter->component_identifier,
                                  header->_recv_headers_iter->sequence_number,
                                  header->_recv_headers_iter->transport_timestamp);
      }
    }
  }

  // Callbacks
  return run_callbacks();
}


//
// StreamA825
//

ed247::StreamA825::StreamA825(const ed247::xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel) :
  Stream(configuration, ed247_api_channel, sizeof(A825_sample_size_t))
{
}

uint32_t ed247::StreamA825::encode(char * frame, uint32_t frame_size)
{
  if(_send_stack.size() == 0) return 0;
  uint32_t frame_index = 0;
  do{
    const StreamSample& sample = _send_stack.pop_front();

    // Write Data Timestamp and Precise Data Timestamp
    encode_data_timestamp(sample, frame, frame_size, frame_index);

    // Write sample size
    if((frame_index + sizeof(uint8_t)) > frame_size) {
      THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
    }
    if(sample.size() != (uint8_t)sample.size())
      THROW_ED247_ERROR("Stream '" << get_name() << "': Stream data size it too big for a 8 bits number ! (" << sample.size() << ")");
    uint8_t size = (uint8_t)sample.size();
    memcpy(frame + frame_index, &size, sizeof(uint8_t));
    frame_index += sizeof(uint8_t);

    // Write sample data
    if((frame_index + sizeof(uint16_t)) > frame_size) {
      THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
    }
    memcpy(frame + frame_index, sample.data(), sample.size());
    frame_index += sample.size();
  }while(_send_stack.empty() == false);
  return frame_index;
}

bool ed247::StreamA825::decode(const char * frame, uint32_t frame_size, const ed247::FrameHeader * header)
{
  uint32_t frame_index = 0;
  uint32_t sample_size = 0;
  static ed247_timestamp_t data_timestamp;
  static ed247_timestamp_t timestamp;
  while(frame_index < frame_size){
    // Read Data Timestamp if necessary
    if (decode_data_timestamp(frame, frame_size, frame_index, data_timestamp, timestamp) == false) return false;
    // Read sample size
    if((frame_size-frame_index) < sizeof(uint8_t)) {
      PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
      return false;
    }
    sample_size = *(uint8_t*)(frame+frame_index);
    frame_index += sizeof(uint8_t);
    // Read sample data
    if((frame_size-frame_index) < sample_size) {
      PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
      return false;
    }
    if (sample_size > _recv_stack.samples_capacity()) {
      PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
      return false;
    }
    StreamSample& sample = _recv_stack.push_back();
    sample.copy(frame+frame_index, sample_size);
    frame_index += sample.size();
    // Update data timestamp
    if(_configuration->_data_timestamp._enable == ED247_YESNO_YES){
      sample.set_data_timestamp(data_timestamp);
    }
    // Update simulation time
    sample.update_recv_timestamp();
    // Attach header
    if(header) {
      if (header->_recv_headers_iter == header->_recv_headers.end()) {
        sample.clear_frame_infos();
      } else {
        sample.update_frame_infos(header->_recv_headers_iter->component_identifier,
                                  header->_recv_headers_iter->sequence_number,
                                  header->_recv_headers_iter->transport_timestamp);
      }
    }
  }
  // Callbacks
  return run_callbacks();
}


//
// StreamSERIAL
//

ed247::StreamSERIAL::StreamSERIAL(const ed247::xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel) :
  Stream(configuration, ed247_api_channel, sizeof(SERIAL_sample_size_t))
{
}

uint32_t ed247::StreamSERIAL::encode(char * frame, uint32_t frame_size)
{
  if(_send_stack.size() == 0) return 0;
  uint32_t frame_index = 0;
  do{
    const StreamSample& sample = _send_stack.pop_front();

    // Write Data Timestamp and Precise Data Timestamp
    encode_data_timestamp(sample, frame, frame_size, frame_index);

    // Write sample size
    if((frame_index + sizeof(uint16_t)) > frame_size) {
      THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
    }
    if(sample.size() != (uint16_t)sample.size())
      THROW_ED247_ERROR("Stream '" << get_name() << "': Stream data size it too big for a 16 bits number ! (" << sample.size() << ")");
    uint16_t size = (uint16_t)sample.size();
    memcpy(frame + frame_index, &size, sizeof(uint16_t));
    frame_index += sizeof(uint16_t);

    // Write sample data
    if((frame_index + sizeof(uint16_t)) > frame_size) {
      THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
    }
    memcpy(frame + frame_index, sample.data(), sample.size());
    frame_index += sample.size();
  }while(_send_stack.empty() == false);
  return frame_index;
}

bool ed247::StreamSERIAL::decode(const char * frame, uint32_t frame_size, const ed247::FrameHeader * header)
{
  uint32_t frame_index = 0;
  uint32_t sample_size = 0;
  static ed247_timestamp_t data_timestamp;
  static ed247_timestamp_t timestamp;
  while(frame_index < frame_size){
    // Read Data Timestamp if necessary
    if (decode_data_timestamp(frame, frame_size, frame_index, data_timestamp, timestamp) == false) return false;
    // Read sample size
    if((frame_size-frame_index) < sizeof(uint16_t)) {
      PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
      return false;
    }
    sample_size = *(uint16_t*)(frame+frame_index);
    frame_index += sizeof(uint16_t);
    // Read sample data
    if((frame_size-frame_index) < sample_size) {
      PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
      return false;
    }
    if (sample_size > _recv_stack.samples_capacity()) {
      PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
      return false;
    }
    StreamSample& sample = _recv_stack.push_back();
    sample.copy(frame+frame_index, sample_size);
    frame_index += sample.size();
    // Update data timestamp
    if(_configuration->_data_timestamp._enable == ED247_YESNO_YES){
      sample.set_data_timestamp(data_timestamp);
    }
    // Update simulation time
    sample.update_recv_timestamp();
    // Attach header
    if(header) {
      if (header->_recv_headers_iter == header->_recv_headers.end()) {
        sample.clear_frame_infos();
      } else {
        sample.update_frame_infos(header->_recv_headers_iter->component_identifier,
                                  header->_recv_headers_iter->sequence_number,
                                  header->_recv_headers_iter->transport_timestamp);
      }
    }
  }
  // Callbacks
  return run_callbacks();
}


//
// StreamAUDIO
//

ed247::StreamAUDIO::StreamAUDIO(const ed247::xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel) :
  Stream(configuration, ed247_api_channel, sizeof(AUDIO_sample_size_t))
{
}

uint32_t ed247::StreamAUDIO::encode(char * frame, uint32_t frame_size)
{
  if(_send_stack.size() == 0) return 0;
  uint32_t frame_index = 0;
  do{
    const StreamSample& sample = _send_stack.pop_front();

    // Write Data Timestamp and Precise Data Timestamp
    encode_data_timestamp(sample, frame, frame_size, frame_index);

    // Write sample size
    if((frame_index + sizeof(uint8_t)) > frame_size) {
      THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
    }
    if(sample.size() != (uint8_t)sample.size())
      THROW_ED247_ERROR("Stream '" << get_name() << "': Stream data size it too big for a 8 bits number ! (" << sample.size() << ")");
    uint8_t size = (uint8_t)sample.size();
    memcpy(frame + frame_index, &size, sizeof(uint8_t));
    frame_index += sizeof(uint8_t);

    // Write sample data
    if((frame_index + sizeof(uint16_t)) > frame_size) {
      THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
    }
    memcpy(frame + frame_index, sample.data(), sample.size());
    frame_index += sample.size();
  }while(_send_stack.empty() == false);
  return frame_index;
}

bool ed247::StreamAUDIO::decode(const char * frame, uint32_t frame_size, const ed247::FrameHeader * header)
{
  uint32_t frame_index = 0;
  uint32_t sample_size = 0;
  static ed247_timestamp_t data_timestamp;
  static ed247_timestamp_t timestamp;
  while(frame_index < frame_size){
    // Read Data Timestamp if necessary
    if (decode_data_timestamp(frame, frame_size, frame_index, data_timestamp, timestamp) == false) return false;
    // Read sample size
    if((frame_size-frame_index) < sizeof(uint8_t)) {
      PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
      return false;
    }
    sample_size = *(uint8_t*)(frame+frame_index);
    frame_index += sizeof(uint8_t);
    // Read sample data
    if((frame_size-frame_index) < sample_size) {
      PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
      return false;
    }
    if (sample_size > _recv_stack.samples_capacity()) {
      PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
      return false;
    }
    StreamSample& sample = _recv_stack.push_back();
    sample.copy(frame+frame_index, sample_size);
    frame_index += sample.size();
    // Update data timestamp
    if(_configuration->_data_timestamp._enable == ED247_YESNO_YES){
      sample.set_data_timestamp(data_timestamp);
    }
    // Update simulation time
    sample.update_recv_timestamp();
    // Attach header
    if(header) {
      if (header->_recv_headers_iter == header->_recv_headers.end()) {
        sample.clear_frame_infos();
      } else {
        sample.update_frame_infos(header->_recv_headers_iter->component_identifier,
                                  header->_recv_headers_iter->sequence_number,
                                  header->_recv_headers_iter->transport_timestamp);
      }
    }
  }
  // Callbacks
  return run_callbacks();
}

//
// StreamDISCRETE
//

ed247::StreamDISCRETE::StreamDISCRETE(const ed247::xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel, ed247::signal_set_t& context_signal_set) :
  StreamSignals(configuration, ed247_api_channel, context_signal_set, 0)
{
}

uint32_t ed247::StreamDISCRETE::encode(char * frame, uint32_t frame_size)
{
  if(_send_stack.size() == 0) return 0;
  uint32_t frame_index = 0;
  do{
    const StreamSample& sample = _send_stack.pop_front();

    // Write Data Timestamp and Precise Data Timestamp
    encode_data_timestamp(sample, frame, frame_size, frame_index);

    // Write sample data
    memcpy(frame + frame_index, sample.data(), sample.size());
    frame_index += sample.size();
  }while(_send_stack.empty() == false);
  return frame_index;
}

bool ed247::StreamDISCRETE::decode(const char * frame, uint32_t frame_size, const ed247::FrameHeader * header)
{
  uint32_t frame_index = 0;
  static ed247_timestamp_t data_timestamp;
  static ed247_timestamp_t timestamp;
  while(frame_index < frame_size){
    // Read Data Timestamp if necessary
    if (decode_data_timestamp(frame, frame_size, frame_index, data_timestamp, timestamp) == false) return false;
    // Read sample data
    if((frame_size-frame_index) < _configuration->_sample_max_size_bytes) {
      PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
      return false;
    }
    StreamSample& sample = _recv_stack.push_back();
    sample.copy(frame+frame_index, _configuration->_sample_max_size_bytes);
    frame_index += sample.size();
    // Update data timestamp
    if(_configuration->_data_timestamp._enable == ED247_YESNO_YES){
      sample.set_data_timestamp(data_timestamp);
    }
    // Update simulation time
    sample.update_recv_timestamp();
    // Attach header
    if(header) {
      if (header->_recv_headers_iter == header->_recv_headers.end()) {
        sample.clear_frame_infos();
      } else {
        sample.update_frame_infos(header->_recv_headers_iter->component_identifier,
                                  header->_recv_headers_iter->sequence_number,
                                  header->_recv_headers_iter->transport_timestamp);
      }
    }
  }
  // Callbacks
  return run_callbacks();
}


//
// StreamANALOG
//

ed247::StreamANALOG::StreamANALOG(const ed247::xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel, ed247::signal_set_t& context_signal_set) :
  StreamSignals(configuration, ed247_api_channel, context_signal_set, 0)
{
}

uint32_t ed247::StreamANALOG::encode(char * frame, uint32_t frame_size)
{
  if(_send_stack.size() == 0) return 0;
  uint32_t frame_index = 0;
  do{
    StreamSample& sample = _send_stack.pop_front();

    // Write Data Timestamp and Precise Data Timestamp
    encode_data_timestamp(sample, frame, frame_size, frame_index);

    PRINT_CRAZY("SWAP stream [" << _configuration->_name << "] ...");
    // SWAP
    for(auto signal : *_signals){
      *(uint32_t*)(sample.data_rw()+signal->get_byte_offset()) = bswap_32(*(uint32_t*)(sample.data_rw()+signal->get_byte_offset()));
    }
    PRINT_CRAZY("SWAP stream [" << _configuration->_name << "] ... OK");

    // Write sample data
    memcpy(frame + frame_index, sample.data(), sample.size());
    frame_index += sample.size();
  }while(_send_stack.empty() == false);
  return frame_index;
}

bool ed247::StreamANALOG::decode(const char * frame, uint32_t frame_size, const ed247::FrameHeader * header)
{
  uint32_t frame_index = 0;
  static ed247_timestamp_t data_timestamp;
  static ed247_timestamp_t timestamp;
  while(frame_index < frame_size){
    // Read Data Timestamp if necessary
    if (decode_data_timestamp(frame, frame_size, frame_index, data_timestamp, timestamp) == false) return false;
    // Read sample data
    if((frame_size-frame_index) < _configuration->_sample_max_size_bytes) {
      PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
      return false;
    }
    StreamSample& sample = _recv_stack.push_back();
    sample.copy(frame+frame_index, _configuration->_sample_max_size_bytes);
    frame_index += sample.size();

    PRINT_CRAZY("SWAP stream [" << _configuration->_name << "] ...");
    // SWAP
    for(auto signal : *_signals){
      *(uint32_t*)(sample.data_rw()+signal->get_byte_offset()) = bswap_32(*(uint32_t*)(sample.data_rw()+signal->get_byte_offset()));
    }
    PRINT_CRAZY("SWAP stream [" << _configuration->_name << "] ... OK");

    // Update data timestamp
    if(_configuration->_data_timestamp._enable == ED247_YESNO_YES){
      sample.set_data_timestamp(data_timestamp);
    }
    // Update simulation time
    sample.update_recv_timestamp();
    // Attach header
    if(header) {
      if (header->_recv_headers_iter == header->_recv_headers.end()) {
        sample.clear_frame_infos();
      } else {
        sample.update_frame_infos(header->_recv_headers_iter->component_identifier,
                                  header->_recv_headers_iter->sequence_number,
                                  header->_recv_headers_iter->transport_timestamp);
      }
    }
  }
  // Callbacks
  return run_callbacks();
}


//
// StreamNAD
//

ed247::StreamNAD::StreamNAD(const ed247::xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel, ed247::signal_set_t& context_signal_set) :
  StreamSignals(configuration, ed247_api_channel, context_signal_set, 0)
{
}

void swap_nad(void *sample_data, const ed247_nad_type_t & nad_type, const uint32_t & sample_element_length)
{
  // SWAP
  switch((uint8_t)nad_type){
  case ED247_NAD_TYPE_INT16:
  {
    for(uint32_t i = 0 ; i < sample_element_length ; i++){
      *((uint16_t*)sample_data+i) = bswap_16(*((uint16_t*)sample_data+i));
    }
  } break;
  case ED247_NAD_TYPE_INT32:
  {
    for(uint32_t i = 0 ; i < sample_element_length ; i++){
      *((uint32_t*)sample_data+i) = bswap_32(*((uint32_t*)sample_data+i));
    }
  } break;
  case ED247_NAD_TYPE_INT64:
  {
    for(uint32_t i = 0 ; i < sample_element_length ; i++){
      *((uint64_t*)sample_data+i) = bswap_64(*((uint64_t*)sample_data+i));
    }
  } break;
  case ED247_NAD_TYPE_UINT16:
  {
    for(uint32_t i = 0 ; i < sample_element_length ; i++){
      *((uint16_t*)sample_data+i) = bswap_16(*((uint16_t*)sample_data+i));
    }
  } break;
  case ED247_NAD_TYPE_UINT32:
  {
    for(uint32_t i = 0 ; i < sample_element_length ; i++){
      *((uint32_t*)sample_data+i) = bswap_32(*((uint32_t*)sample_data+i));
    }
  } break;
  case ED247_NAD_TYPE_UINT64:
  {
    for(uint32_t i = 0 ; i < sample_element_length ; i++){
      *((uint64_t*)sample_data+i) = bswap_64(*((uint64_t*)sample_data+i));
    }
  } break;
  case ED247_NAD_TYPE_FLOAT32:
  {
    for(uint32_t i = 0 ; i < sample_element_length ; i++){
      *((uint32_t*)sample_data+i) = bswap_32(*((uint32_t*)sample_data+i));
    }
  } break;
  case ED247_NAD_TYPE_FLOAT64:
  {
    for(uint32_t i = 0 ; i < sample_element_length ; i++){
      *((uint64_t*)sample_data+i) = bswap_64(*((uint64_t*)sample_data+i));
    }
  } break;
  default:
    break;
  }
}

uint32_t ed247::StreamNAD::encode(char * frame, uint32_t frame_size)
{
  if(_send_stack.size() == 0) return 0;
  uint32_t frame_index = 0;
  do{
    StreamSample& sample = _send_stack.pop_front();

    // Write Data Timestamp and Precise Data Timestamp
    encode_data_timestamp(sample, frame, frame_size, frame_index);

    PRINT_CRAZY("SWAP stream [" << _configuration->_name << "] ...");
    // SWAP
    for(auto signal : *_signals){
      void *sample_data = (void*)(sample.data_rw()+signal->get_byte_offset());
      uint32_t sample_element_length = signal->get_sample_max_size_bytes() / signal->get_nad_type_size();
      swap_nad(sample_data, signal->get_nad_type(), sample_element_length);
    }
    PRINT_CRAZY("SWAP stream [" << _configuration->_name << "] ... OK");

    // Write sample data
    memcpy(frame + frame_index, sample.data(), sample.size());
    frame_index += sample.size();
  }while(_send_stack.empty() == false);
  return frame_index;
}

bool ed247::StreamNAD::decode(const char * frame, uint32_t frame_size, const ed247::FrameHeader * header)
{
  uint32_t frame_index = 0;
  static ed247_timestamp_t data_timestamp;
  static ed247_timestamp_t timestamp;
  while(frame_index < frame_size){
    // Read Data Timestamp if necessary
    if (decode_data_timestamp(frame, frame_size, frame_index, data_timestamp, timestamp) == false) return false;
    // Read sample data
    if((frame_size-frame_index) < _configuration->_sample_max_size_bytes) {
      PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
      return false;
    }
    StreamSample& sample = _recv_stack.push_back();
    sample.copy(frame+frame_index, _configuration->_sample_max_size_bytes);
    frame_index += sample.size();

    PRINT_CRAZY("SWAP stream [" << _configuration->_name << "] ...");
    // SWAP
    for(auto signal : *_signals){
      void *sample_data = (void*)(sample.data_rw()+signal->get_byte_offset());
      uint32_t sample_element_length = signal->get_sample_max_size_bytes() / signal->get_nad_type_size();
      swap_nad(sample_data, signal->get_nad_type(), sample_element_length);
    }
    PRINT_CRAZY("SWAP stream [" << _configuration->_name << "] ... OK");

    // Update data timestamp
    if(_configuration->_data_timestamp._enable == ED247_YESNO_YES){
      sample.set_data_timestamp(data_timestamp);
    }
    // Update simulation time
    sample.update_recv_timestamp();
    // Attach header
    if(header) {
      if (header->_recv_headers_iter == header->_recv_headers.end()) {
        sample.clear_frame_infos();
      } else {
        sample.update_frame_infos(header->_recv_headers_iter->component_identifier,
                                  header->_recv_headers_iter->sequence_number,
                                  header->_recv_headers_iter->transport_timestamp);
      }
    }
  }
  // Callbacks
  return run_callbacks();
}


//
// StreamVNAD
//

ed247::StreamVNAD::StreamVNAD(const ed247::xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel, ed247::signal_set_t& context_signal_set) :
  StreamSignals(configuration, ed247_api_channel, context_signal_set, sizeof(VNAD_sample_size_t))
{
}

uint32_t ed247::StreamVNAD::encode(char * frame, uint32_t frame_size)
{
  if(_send_stack.size() == 0) return 0;
  uint32_t frame_index = 0;
  do{
    StreamSample& sample = _send_stack.pop_front();

    // Write Data Timestamp and Precise Data Timestamp
    encode_data_timestamp(sample, frame, frame_size, frame_index);

    PRINT_CRAZY("SWAP stream [" << _configuration->_name << "] ...");
    // SWAP
    uint32_t cursor = 0;
    uint32_t cursor_step = 0;
    uint32_t isignal = 0;
    uint16_t sample_size_bytes = 0;
    while(cursor < sample.size()){
      sample_size_bytes = ntohs(*(uint16_t*)(sample.data()+cursor));
      cursor += sizeof(uint16_t);
      cursor_step = (*_signals)[isignal]->get_nad_type_size();
      swap_nad((void*)(sample.data_rw()+cursor), (*_signals)[isignal]->get_nad_type(), sample_size_bytes/cursor_step);
      cursor += sample_size_bytes;
      isignal++;
    }
    PRINT_CRAZY("SWAP stream [" << _configuration->_name << "] ... OK");

    // Write sample size
    if((frame_index + sizeof(uint16_t) + sample.size()) > frame_size) {
      THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
    }
    if(sample.size() != (uint16_t)sample.size())
      THROW_ED247_ERROR("Stream '" << get_name() << "': Stream data size it too big for a 16 bits number ! (" << sample.size() << ")");
    uint16_t size = htons((uint16_t)sample.size());
    memcpy(frame + frame_index, &size, sizeof(uint16_t));
    frame_index += sizeof(uint16_t);
    // Write sample data
    memcpy(frame + frame_index, sample.data(), sample.size());
    frame_index += sample.size();
  }while(_send_stack.empty() == false);
  return frame_index;
}

bool ed247::StreamVNAD::decode(const char * frame, uint32_t frame_size, const ed247::FrameHeader* header)
{
  uint32_t frame_index = 0;
  uint32_t sample_size = 0;
  static ed247_timestamp_t data_timestamp;
  static ed247_timestamp_t timestamp;
  while(frame_index < frame_size){
    // Read Data Timestamp if necessary
    if (decode_data_timestamp(frame, frame_size, frame_index, data_timestamp, timestamp) == false) return false;
    // Read sample size
    if((frame_size-frame_index) < sizeof(uint16_t)) {
      PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
      return false;
    }
    sample_size = ntohs(*(uint16_t*)(frame+frame_index));
    frame_index += sizeof(uint16_t);
    // Read sample data
    if((frame_size-frame_index) < sample_size) {
      PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
      return false;
    }
    if (sample_size > _recv_stack.samples_capacity()) {
      PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
      return false;
    }
    StreamSample& sample = _recv_stack.push_back();
    sample.copy(frame+frame_index, sample_size);
    frame_index += sample.size();

    PRINT_CRAZY("SWAP stream [" << _configuration->_name << "] ...");
    // SWAP
    uint32_t cursor = 0;
    uint32_t cursor_step = 0;
    uint32_t isignal = 0;
    uint16_t sample_size_bytes = 0;
    while(cursor < sample.size()){
      sample_size_bytes = ntohs(*(uint16_t*)(sample.data()+cursor));
      cursor += sizeof(uint16_t);
      cursor_step = (*_signals)[isignal]->get_nad_type_size();
      swap_nad((void*)(sample.data_rw()+cursor), (*_signals)[isignal]->get_nad_type(), sample_size_bytes/cursor_step);
      cursor += sample_size_bytes;
      isignal++;
    }
    PRINT_CRAZY("SWAP stream [" << _configuration->_name << "] ... OK");

    // Update data timestamp
    if(_configuration->_data_timestamp._enable == ED247_YESNO_YES){
      sample.set_data_timestamp(data_timestamp);
    }
    // Update simulation time
    sample.update_recv_timestamp();
    // Attach header
    if(header) {
      if (header->_recv_headers_iter == header->_recv_headers.end()) {
        sample.clear_frame_infos();
      } else {
        sample.update_frame_infos(header->_recv_headers_iter->component_identifier,
                                  header->_recv_headers_iter->sequence_number,
                                  header->_recv_headers_iter->transport_timestamp);
      }
    }
  }
  // Callbacks
  return run_callbacks();
}


