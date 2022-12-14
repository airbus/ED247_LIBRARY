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
#include "ed247_channel.h"
#include "ed247_context.h"
#include "ed247_logs.h"
#include <regex>

typedef uint16_t stream_size_t;

//
// Channel
//

ed247::Channel::Channel(Context* context, const xml::Channel* configuration) :
  _context(context),
  _configuration(configuration),
  _com_interface(context),
  _header(configuration->_header, context->get_identifier(), get_name()),
  _user_data(NULL)
{
  uint32_t capacity = 0;
  capacity += _header.get_size();

  for(auto& stream_configuration : configuration->_stream_list)
  {
    // Create and insert the new stream
    stream_ptr_t stream = _context->get_stream_set().create(stream_configuration.get(), this);
    auto result = _streams.insert(std::make_pair(stream->get_uid(), stream));
    if (result.second == false) {
      THROW_ED247_ERROR("Stream [" << stream->get_name() << "] uses an UID already registered in Channel [" << get_name() << "]");
    }

    // Compute buffer capacity
    if(_configuration->_is_simple_channel == false) {
      capacity += sizeof(ed247_uid_t) + sizeof(stream_size_t);
    }
    capacity += stream->get_max_size();
  }

  // Load the ComInterface and connect decode()
  _com_interface.load(configuration->_com_interface,
                      std::bind(&Channel::decode, this, std::placeholders::_1, std::placeholders::_2));

  _buffer.allocate(capacity);

  MEMCHECK_NEW(this, "Channel " << _configuration->_name);
}

ed247::Channel::~Channel()
{
  MEMCHECK_DEL(this, "Channel " << _configuration->_name);
}



ed247::stream_list_t ed247::Channel::find_streams(std::string strregex)
{
  std::regex reg(strregex);
  stream_list_t founds;
  map_uid_stream_t::iterator iter = _streams.begin();
  for(iter = _streams.begin() ; iter != _streams.end() ; iter++){
    if(!iter->second) {
      THROW_ED247_ERROR("Channel '" << get_name() << "': contains an invalid Stream at [" << iter->first << "]");
    }
    if(std::regex_match(iter->second->get_name(), reg)){
      founds.push_back(iter->second);
    }
  }
  return founds;
}

ed247::stream_ptr_t ed247::Channel::get_stream(std::string str_name)
{
  map_uid_stream_t::iterator iter = _streams.begin();
  for(iter = _streams.begin() ; iter != _streams.end() ; iter++){
    if(!iter->second) {
      THROW_ED247_ERROR("Channel '" << get_name() << "': contains an invalid Stream at [" << iter->first << "]");
    }
    if(iter->second->get_name() == str_name) return iter->second;
  }
  return nullptr;
}

void ed247::Channel::encode_and_send()
{
  // Will be set to true if it remain data to send
  bool need_new_packet = false;

  do {
    need_new_packet = false;

    // Note: we don't perform many size check: the buffer is big enougth for
    // one stream and Stream::encode perform a check

    _buffer.reset();

    if(_configuration->_is_simple_channel)
    {
      // Simple channel
      uint32_t frame_index = 0;
      stream_ptr_t& stream = _streams.begin()->second;
      if (stream->get_outgoing_sample_number() != 0) {
        _header.encode(_buffer.data_rw(), _buffer.capacity(), frame_index);
        frame_index += stream->encode(_buffer.data_rw() + frame_index, _buffer.capacity());
        _buffer.set_size(frame_index);

        need_new_packet |= (stream->get_outgoing_sample_number() != 0);
      }
    }
    else
    {
      // MultiChannel
      uint32_t frame_index = 0;
      bool header_wrote = false;

      for(auto& pair : _streams) {
        stream_ptr_t& stream = pair.second;

        if((stream->get_direction() & ED247_DIRECTION_OUT) == false ||
           stream->get_outgoing_sample_number() == 0) {
          continue;
        }

        if (header_wrote == false) {
          _header.encode(_buffer.data_rw(), _buffer.capacity(), frame_index);
          header_wrote = true;
        }

        if(frame_index + sizeof(ed247_uid_t) + sizeof(stream_size_t) > _buffer.capacity()) {
          // TODO: instead of throwing, we can loop to send another packet (aka fragmentation)
          THROW_ED247_ERROR("Channel '" << get_name() << "': buffer is too small ! (" << _buffer.capacity() << " bytes)");
        }

        // Write the stream
        uint32_t stream_index = frame_index + sizeof(ed247_uid_t) + sizeof(stream_size_t);
        uint32_t stream_size = stream->encode(_buffer.data_rw() + stream_index, _buffer.capacity() - stream_index);

        // Write the header
        if (stream_size > 0) {
          *(ed247_uid_t*)(_buffer.data_rw() + frame_index) = htons(stream->get_uid());
          *(stream_size_t*)(_buffer.data_rw() + frame_index + sizeof(ed247_uid_t)) = htons((stream_size_t)stream_size);
          frame_index = stream_index + stream_size;
        }
        need_new_packet |= (stream->get_outgoing_sample_number() != 0);
      }
      _buffer.set_size(frame_index);
    }

    if(_buffer.empty() == false) {
      _com_interface.send_frame(_buffer.data(), _buffer.size());
    }

  } while (need_new_packet);
}



bool ed247::Channel::decode(const char* frame, uint32_t frame_size)
{
  uint32_t frame_index = 0;

  if (_header.decode(frame, frame_size, frame_index) == false) return false;

  if(_configuration->_is_simple_channel)
  {
    // Simple channel
    stream_ptr_t& stream = _streams.begin()->second;
    if (stream->decode(frame + frame_index, frame_size - frame_index, _header.get_recv_frame_details()) == false) {
      return false;
    }
  }
  else
  {
    // MultiChannel
    while(frame_index < frame_size) {

      // Decode header
      if (frame_size - frame_index < sizeof(ed247_uid_t) + sizeof(stream_size_t)) {
        PRINT_ERROR("Channel '" << get_name() << ": frame of size " << frame_size << " too short to contain another stream at byte " << frame_index << ".");
        return false;
      }
      ed247_uid_t stream_uid = ntohs(*(ed247_uid_t*)(frame + frame_index));
      frame_index += sizeof(ed247_uid_t);
      stream_size_t stream_sample_size = ntohs(*(stream_size_t*)(frame + frame_index));
      frame_index += sizeof(stream_size_t);

      // Decode the stream
      if (frame_size - frame_index < stream_sample_size) {
        PRINT_ERROR("Channel '" << get_name() << ": frame of size " << frame_size << " too short at byte " <<
                    frame_index << ". A stream of size " << stream_sample_size << " is expected.");
        return false;
      }
      map_uid_stream_t::iterator istream = _streams.find(stream_uid);
      if (istream != _streams.end()) {
        if (istream->second->decode(frame + frame_index, stream_sample_size, _header.get_recv_frame_details()) == false) {
          // Decode goes wrong. We cannot decode remaining data
          PRINT_ERROR("Channel '" << get_name() << ": Cannot decode stream " << stream_uid);
          return false;
        }
      } else {
        // We don't known this stream.
        // This is not an error: it might be for another receiver. We process the next stream.
      }
      frame_index += stream_sample_size;
    }
  }

  return true;
}


//
// ChannelSet
//

ed247::ChannelSet::ChannelSet(Context* context) :
  _context(context)
{
  MEMCHECK_NEW(this, "ChannelSet");
}

ed247::ChannelSet::~ChannelSet()
{
  MEMCHECK_DEL(this, "ChannelSet");
}

ed247::channel_ptr_t ed247::ChannelSet::create(const ed247::xml::Channel* configuration)
{
  channel_ptr_t channel = std::make_shared<Channel>(_context, configuration);
  auto result = _channels.emplace(std::make_pair(configuration->_name, channel));
  if (result.second == false) THROW_ED247_ERROR("Channel [" << configuration->_name << "] already exist !");
  return result.first->second;
}

ed247::channel_ptr_t ed247::ChannelSet::get(std::string name)
{
  auto iterator = _channels.find(name);
  if (iterator != _channels.end()) return iterator->second;
  return nullptr;
}

ed247::channel_list_t ed247::ChannelSet::find(std::string str_regex)
{
  std::regex reg(str_regex);
  channel_list_t founds;
  for(auto& channel_pair : _channels) {
    if(std::regex_match(channel_pair.first, reg)) {
      founds.push_back(channel_pair.second);
    }
  }
  return founds;
}
