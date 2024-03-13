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
#include "ed247_context.h"
#include "ed247_client_list.h"
#include "ed247_bswap.h"
#include "ed247_logs.h"
#include <regex>


static const uint32_t SECOND_TO_NANO = 1000 * 1000 * 1000;

typedef uint16_t A664_sample_size_t;
typedef uint8_t  A825_sample_size_t;
typedef uint16_t SERIAL_sample_size_t;
typedef uint16_t AUDIO_sample_size_t;
typedef uint16_t VNAD_sample_size_t;
typedef uint16_t ETHERNET_sample_size_t;

//
// Client lists (ed247.h interface)
//
namespace ed247 {
  using ClientSignalList = ed247::client_list_container<ed247_internal_signal_list_t,
                                                        ed247::Signal,
                                                        ed247::signal_list_t,
                                                        ed247::ContextOwned::True>;
}


//
// Stream initialization
//
ed247::Stream::Stream(Context* context, const ed247::xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel, uint32_t sample_size_size):
  _context(context),
  _configuration(configuration),
  _sample_size_size(sample_size_size),
  _client_signals(ed247::ClientSignalList::wrap(_signals)),
  _recv_stack(_configuration->_sample_max_number, _configuration->_sample_max_size_bytes),
  _send_stack(_configuration->_sample_max_number, _configuration->_sample_max_size_bytes),
  _ed247_api_channel(ed247_api_channel),
  _user_data(NULL)
{
  MEMCHECK_NEW(this, "Stream " << configuration->_name);

  _sample_first_header_size = _sample_next_header_size = sample_size_size;
  if (_configuration->_data_timestamp._enable == ED247_YESNO_YES) {
    _sample_first_header_size += 2 * sizeof(uint32_t);
    if (_configuration->_data_timestamp._enable_sample_offset == ED247_YESNO_YES) {
      _sample_next_header_size += sizeof(uint32_t);
    }
  }

  _max_size = _configuration->_sample_max_size_bytes + _sample_first_header_size;
  _max_size += (_configuration->_sample_max_number - 1) * (_configuration->_sample_max_size_bytes + _sample_next_header_size);
}

ed247::Stream::~Stream()
{
  MEMCHECK_DEL(this, "Stream " << _configuration->_name);
}

//
// Stream Signals part
//

ed247::StreamSignals::StreamSignals(Context* context, const xml::Stream* configuration,
                                    ed247_internal_channel_t* ed247_api_channel, uint32_t sample_size_size) :
  Stream(context, configuration, ed247_api_channel, sample_size_size)
{
  const xml::StreamSignals* sconfiguration = (const xml::StreamSignals*)configuration;
  for(auto& signal_configuration : sconfiguration->_signal_list) {
    signal_ptr_t signal = _context->get_signal_set().create(signal_configuration.get(), this);
    _signals.push_back(signal);
  }
  if (get_type() == ED247_STREAM_TYPE_VNAD) {
    _assistant = std::unique_ptr<StreamAssistant>(new VNADStreamAssistant(this));
  } else {
    _assistant = std::unique_ptr<StreamAssistant>(new FixedStreamAssistant(this));
  }
}

ed247::signal_list_t ed247::Stream::Stream::find_signals(std::string str_regex)
{
  std::regex reg(str_regex);
  signal_list_t founds;
  for(auto signal: _signals){
    if(std::regex_match(signal->get_name(), reg)){
      founds.push_back(signal);
    }
  }
  return founds;
}

ed247::signal_ptr_t ed247::Stream::Stream::get_signal(std::string str_name)
{
  for(auto signal : _signals){
    if(signal->get_name() == str_name) return signal;
  }
  return nullptr;
}


//
// samples push/pop
//
bool ed247::Stream::push_sample(const void * sample_data, uint32_t sample_size, const ed247_timestamp_t * data_timestamp, bool * full)
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

ed247::StreamSample& ed247::Stream::pop_sample(bool* empty)
{
  StreamSample& result = _recv_stack.pop_front();
  if (empty) *empty = _recv_stack.empty();
  return result;
}


//
// Encode stream to frame
//
uint32_t ed247::Stream::encode(char* frame, uint32_t frame_size)
{
  uint32_t frame_index = 0;
  ed247_timestamp_t first_sample_dts = { 0, 0 };

  while (_send_stack.empty() == false)
  {
    const StreamSample& sample = _send_stack.pop_front();
    const uint32_t sample_size = ((frame_index == 0)? _sample_first_header_size : _sample_next_header_size) + sample.size();

    if (frame_index + sample_size > frame_size) {
      // TODO: instead of throwing an exception, the issue might be raised up to the channel
      // to allow it to split the content in several streams
      THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
    }

    // Encode DST
    if (_configuration->_data_timestamp._enable == ED247_YESNO_YES) {
      ed247_timestamp_t sample_dst = sample.data_timestamp();
      if (frame_index == 0) {
        first_sample_dts = sample_dst;
        *(uint32_t*)(frame + frame_index                   ) = htonl(sample_dst.epoch_s);
        *(uint32_t*)(frame + frame_index + sizeof(uint32_t)) = htonl(sample_dst.offset_ns);
        frame_index += 2 * sizeof(uint32_t);
      }
      else if (_configuration->_data_timestamp._enable_sample_offset == ED247_YESNO_YES) {
        uint32_t delta_ns =
          ((int32_t)sample_dst.epoch_s - (int32_t)first_sample_dts.epoch_s) * SECOND_TO_NANO +
          (int32_t)sample_dst.offset_ns - (int32_t)first_sample_dts.offset_ns;
        *(uint32_t*)(frame + frame_index) = htonl(delta_ns);
        frame_index += sizeof(int32_t);
      }
    }

    // Encode sample size
    // Note: sample.size() can be encoded in the target integer. This has been checked on xml load.
    switch (_sample_size_size) {
    case 0: break;

    case sizeof(uint8_t):
      *(uint8_t*)(frame + frame_index) = (uint8_t) sample.size();
      frame_index += sizeof(uint8_t);
      break;

    case sizeof(uint16_t):
      *(uint16_t*)(frame + frame_index) = htons((uint16_t) sample.size());
      frame_index += sizeof(uint16_t);
      break;

    default:
      THROW_ED247_ERROR("Samples cannot be encoded on " << _sample_size_size << " bytes. (ED247 library bug)");
    }

    // Encode the frame
    memcpy(frame + frame_index, sample.data(), sample.size());
    frame_index += sample.size();

    // Hack ED247LIB-27
    // If message size is disabled, we cannot append more than one sample.
    // Remaining ones will be available for next encode() call.
    if (get_type() == ED247_STREAM_TYPE_A664) {
      auto enable_message_size = ((const xml::A664Stream*)_configuration)->_enable_message_size == ED247_YESNO_YES;
      if (enable_message_size == false) break;
    }
  }
  return frame_index;
}


//
// Encode stream from frame
//
bool ed247::Stream::decode(const char* frame, uint32_t frame_size, const ed247_sample_details_t& frame_details)
{
  uint32_t frame_index = 0;
  ed247_timestamp_t first_sample_dts = { 0, 0 };

  while(frame_index < frame_size) {
    //
    // Check header size and decode datatimestamp
    //
    ed247_timestamp_t sample_dts(first_sample_dts);

    if(frame_index == 0) {
      // First sample
      if((frame_size - frame_index) < _sample_first_header_size) {
        PRINT_ERROR("Stream '" << get_name() << "': Received frame is too small. Size: " << frame_size);
        return false;
      }

      if(_configuration->_data_timestamp._enable == ED247_YESNO_YES) {
        first_sample_dts.epoch_s   = ntohl(*(uint32_t*)(frame + frame_index                   ));
        first_sample_dts.offset_ns = ntohl(*(uint32_t*)(frame + frame_index + sizeof(uint32_t)));
        sample_dts = first_sample_dts;
        frame_index += 2 * sizeof(uint32_t);
      }

    } else {
      // Next samples
      if((frame_size - frame_index) < _sample_next_header_size) {
        PRINT_ERROR("Stream '" << get_name() << "': Received frame is too small. Size: " << frame_size);
        return false;
      }

      if(_configuration->_data_timestamp._enable_sample_offset == ED247_YESNO_YES) {
        int32_t delta_ns = (int32_t)ntohl(*(uint32_t*)(frame + frame_index));
        sample_dts.epoch_s += delta_ns / SECOND_TO_NANO;
        sample_dts.offset_ns += (delta_ns - (delta_ns / SECOND_TO_NANO) * SECOND_TO_NANO);
        frame_index += sizeof(uint32_t);
      }
    }

    //
    // Decode sample size and validate it
    //
    uint32_t sample_size = 0;
    switch (_sample_size_size) {
    case 0:
      if (_configuration->_sample_size_fixed) {
        // Size fixed by configuration
        sample_size = _configuration->_sample_max_size_bytes;
        if (frame_size - frame_index < sample_size) {
          PRINT_ERROR("Stream '" << get_name() << "': Received frame is too small. Size: " << frame_size);
          return false;
        }
      } else {
        // No size information: the whole frame is the sample
        sample_size = frame_size - frame_index;
        if (sample_size > _configuration->_sample_max_size_bytes) {
          PRINT_ERROR("Stream '" << get_name() << "': Received frame is too small. Size: " << frame_size);
          return false;
        }
      }
      break;

    case sizeof(uint8_t):
      sample_size = *(uint8_t*)(frame + frame_index);
      frame_index += sizeof(uint8_t);
      if (sample_size > _configuration->_sample_max_size_bytes) {
        PRINT_ERROR("Stream '" << get_name() << "': Invalid received frame. Size in header  is invalid: " << sample_size);
        return false;
      }
      if (frame_size - frame_index < sample_size) {
        PRINT_ERROR("Stream '" << get_name() << "': Received frame is too small. Size: " << frame_size);
        return false;
      }
      break;

    case sizeof(uint16_t):
      sample_size = ntohs(*(uint16_t*)(frame + frame_index));
      frame_index += sizeof(uint16_t);
      if (sample_size > _configuration->_sample_max_size_bytes) {
        PRINT_ERROR("Stream '" << get_name() << "': Invalid received frame. Size in header is invalid: " << sample_size);
        return false;
      }
      if (frame_size - frame_index < sample_size) {
        PRINT_ERROR("Stream '" << get_name() << "': Received frame is too small. Size: " << frame_size);
        return false;
      }
      break;

    default:
      THROW_ED247_ERROR("Samples cannot be decoded for size " << _sample_size_size << " bytes. (ED247 library bug)");
    }


    //
    // Add the new sample
    //
    StreamSample& sample = _recv_stack.push_back();

    sample.set_data_timestamp(sample_dts);
    sample.update_recv_timestamp();

    sample.copy(frame + frame_index, sample_size);
    frame_index += sample.size();

    sample.set_frame_details(frame_details);
  }

  return run_callbacks();
}


//
// Stream callbacks
//

ed247_status_t ed247::Stream::register_callback(ed247_context_t context, ed247_stream_recv_callback_t callback)
{
  if(callback == nullptr) {
    return ED247_STATUS_FAILURE;
  }
  auto it = std::find_if(_callbacks.begin(), _callbacks.end(),
                         [&context, &callback](CallbackData & element) {
                           return element.context == context && element.callback == callback;
                         });
  if(it != _callbacks.end()){
    return ED247_STATUS_FAILURE;
  }
  _callbacks.push_back((CallbackData) { context, callback });
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247::Stream::unregister_callback(ed247_context_t context, ed247_stream_recv_callback_t callback)
{
  auto it = std::find_if(_callbacks.begin(), _callbacks.end(),
                         [&context, &callback](CallbackData & element) {
                           return element.context == context && element.callback == callback;
                         });
  if(it == _callbacks.end()){
    return ED247_STATUS_FAILURE;
  }
  _callbacks.erase(it);
  return ED247_STATUS_SUCCESS;
}

bool ed247::Stream::run_callbacks()
{
  for(auto & pcallback : _callbacks)
  {
    ed247_status_t status = (*pcallback.callback)(pcallback.context, this);
    if (status != ED247_STATUS_SUCCESS) {
      PRINT_DEBUG("User callback fail with return code: " << status);
      return false;
    }
  }
  return true;
}


//
// StreamSet
//
ed247::StreamSet::StreamSet(ed247::Context* context) :
  _context(context)
{
  MEMCHECK_NEW(this, "StreamSet");
}

ed247::StreamSet::~StreamSet()
{
  MEMCHECK_DEL(this, "StreamSet");
}

ed247::stream_ptr_t ed247::StreamSet::create(const ed247::xml::Stream* configuration, ed247_internal_channel_t* ed247_api_channel)
{
  PRINT_DEBUG("Create stream [" << configuration->_name << "] ...");

  if (_streams.find(configuration->_name) != _streams.end()) {
    THROW_ED247_ERROR("Stream [" << configuration->_name << "] already exists !");
  }

  ed247::stream_ptr_t stream;

  switch (configuration->_type) {
  case ED247_STREAM_TYPE_A429:
    stream = std::make_shared<Stream>(_context, configuration, ed247_api_channel, 0);
    break;

  case ED247_STREAM_TYPE_A664:
    stream = std::make_shared<Stream>(_context, configuration, ed247_api_channel,
                                      (((const xml::A664Stream*)configuration)->_enable_message_size == ED247_YESNO_YES)? sizeof(A664_sample_size_t) : 0);
    break;

  case ED247_STREAM_TYPE_A825:
    stream = std::make_shared<Stream>(_context, configuration, ed247_api_channel, sizeof(A825_sample_size_t));
    break;

  case ED247_STREAM_TYPE_SERIAL:
    stream = std::make_shared<Stream>(_context, configuration, ed247_api_channel, sizeof(SERIAL_sample_size_t));
    break;

  case ED247_STREAM_TYPE_ETHERNET:
    stream = std::make_shared<Stream>(_context, configuration, ed247_api_channel,
                                      (((const xml::ETHStream*)configuration)->_enable_message_size == ED247_YESNO_YES)? sizeof(ETHERNET_sample_size_t) : 0);
    break;

  case ED247_STREAM_TYPE_AUDIO:
    stream = std::make_shared<Stream>(_context, configuration, ed247_api_channel, sizeof(AUDIO_sample_size_t));
    break;

  case ED247_STREAM_TYPE_DISCRETE:
    stream = std::make_shared<StreamSignals>(_context, configuration, ed247_api_channel, 0);
    break;

  case ED247_STREAM_TYPE_ANALOG:
    stream = std::make_shared<StreamSignals>(_context, configuration, ed247_api_channel, 0);
    break;

  case ED247_STREAM_TYPE_NAD:
    stream = std::make_shared<StreamSignals>(_context, configuration, ed247_api_channel, 0);
    break;

  case ED247_STREAM_TYPE_VNAD:
    stream = std::make_shared<StreamSignals>(_context, configuration, ed247_api_channel, sizeof(VNAD_sample_size_t));
    break;

  case ED247_STREAM_TYPE_VIDEO:
  case ED247_STREAM_TYPE_M1553:
    THROW_ED247_ERROR("Stream '" << configuration->_name << "': Unsupported stream type '" << configuration->_type << "'");
    break;

  case ED247_STREAM_TYPE__INVALID:
  case ED247_STREAM_TYPE__COUNT:
    THROW_ED247_ERROR("Stream '" << configuration->_name << "': Invalid stream type '" << configuration->_type << "'");
    break;
  }

  // Store signal based streams for fast access
  if (stream->is_signal_based()) {
    if (stream->get_direction() == ED247_DIRECTION_OUT) {
      _streams_signals_output.push_back(stream);
    } else {
      _streams_signals_input.push_back(stream);
    }
  }

  // Store all streams
  auto result = _streams.emplace(std::make_pair(configuration->_name, stream));
  if (result.second == false) THROW_ED247_ERROR("Stream [" << configuration->_name << "] already exist !");
  return result.first->second;
}

ed247::stream_ptr_t ed247::StreamSet::get(std::string name)
{
  auto iterator = _streams.find(name);
  if (iterator != _streams.end()) return iterator->second;
  return nullptr;
}

ed247::stream_list_t ed247::StreamSet::find(std::string str_regex)
{
  std::regex reg(str_regex);
  stream_list_t founds;
  for(auto& stream_pair : _streams) {
    if(std::regex_match(stream_pair.first, reg)) {
      founds.push_back(stream_pair.second);
    }
  }
  return founds;
}
