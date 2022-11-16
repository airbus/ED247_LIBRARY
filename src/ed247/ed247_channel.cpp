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
#include "ed247_stream.h"

#include <regex>

#ifdef __unix__
#include <time.h>
#endif

namespace ed247
{
// Channel

Channel::~Channel()
{
    MEMCHECK_DEL(this, "Channel " << _configuration->_name);
}

void Channel::send()
{
    if(_buffer.empty()) return;
    _com_interface.send_frame(_buffer.data(), _buffer.size());
}

bool Channel::has_samples_to_send()
{
    for(auto & stream_iterator : _streams) {
        if (stream_iterator.second.stream->send_stack().size() != 0) return true;
    }
    return false;
}

void Channel::encode(const ed247_uid_t & component_identifier)
{
    uint32_t buffer_index = 0;
    uint32_t stream_data_size = 0;
    // Encode header
    _header.encode(_buffer.data_rw(), _buffer.capacity(), buffer_index, component_identifier);
    uint32_t header_index = buffer_index;
    // Encode channel payload
    if(!_configuration->_simple){
        for(auto & p : _streams){
            auto & s = p.second.stream;
            if(!(p.second.direction & ED247_DIRECTION_OUT))
                continue;
            if(s->send_stack().size()){
                if(buffer_index + sizeof(ed247_uid_t) + sizeof(uint16_t) > _buffer.capacity()) {
                    THROW_ED247_ERROR("Channel '" << get_name() << "': buffer is too small ! (" << _buffer.capacity() << " bytes)");
                }
                // Write Stream UID
                ed247_uid_t sid = htons(s->get_configuration()->_uid);
                memcpy(_buffer.data_rw() + buffer_index, &sid, sizeof(ed247_uid_t));
                buffer_index += sizeof(ed247_uid_t);
                // Write Stream sample data
                stream_data_size = s->encode(_buffer.data_rw()+buffer_index+sizeof(uint16_t),_buffer.capacity());
                // Write Stream sample size
                if(stream_data_size != (uint16_t)stream_data_size) {
                    THROW_ED247_ERROR("Channel '" << get_name() << "': Stream data size it too big for a 16 bits number ! (" << stream_data_size << ")");
                }
                uint16_t size = htons((uint16_t)stream_data_size);
                memcpy(_buffer.data_rw() + buffer_index, &size, sizeof(uint16_t));
                buffer_index += sizeof(uint16_t) + stream_data_size;
            }
        }
    }else{
        auto & s = _streams.begin()->second.stream;
        // Write Stream sample data
        stream_data_size = s->encode(_buffer.data_rw()+buffer_index,_buffer.capacity());
        buffer_index += stream_data_size;
    }
    if(buffer_index == header_index){ // Check if something had been written in the buffer after the header
        _buffer.reset();
    }else{
        _buffer.set_size(buffer_index);
    }
}

bool Channel::decode(const char * frame, uint32_t frame_size)
{
  uint32_t frame_index = 0;
  if (_header.decode(frame, frame_size, frame_index) == false) return false;

  if(_configuration->_simple == false) {
    while(frame_index < frame_size) {
      // A multichannel stream starts by its UID and its size
      if (frame_size - frame_index < sizeof(ed247_uid_t) + sizeof(uint16_t)) {
        PRINT_ERROR("Channel '" << get_name() << ": frame of size " << frame_size << " too short to contain another stream at byte " << frame_index << ".");
        return false;
      }
      ed247_uid_t stream_uid = ntohs(*(ed247_uid_t*)(frame + frame_index));
      frame_index += sizeof(ed247_uid_t);
      uint16_t stream_sample_size = ntohs(*(uint16_t*)(frame + frame_index));
      frame_index += sizeof(uint16_t);

      if (frame_size - frame_index < stream_sample_size) {
        PRINT_ERROR("Channel '" << get_name() << ": frame of size " << frame_size << " too short at byte " << frame_index << ". A stream of size " << stream_sample_size << " is expected.");
        return false;
      }

      map_streams_t::iterator istream = _streams.find(stream_uid);
      if (istream != _streams.end()) {
        istream->second.stream->decode(frame + frame_index, stream_sample_size, &_header); // Ignore the decode error to process the next stream
      } else {
        // We don't known this stream. This is not an error: it might be for another receiver. We process the next stream.
      }

      frame_index += stream_sample_size;
    }
  }
  else {
    // Simple channel
    if (_streams.begin()->second.stream->decode(frame + frame_index, frame_size - frame_index, &_header) == false) {
      return false;
    }
  }
  return true;
}

stream_list_t Channel::find_streams(std::string strregex)
{
    std::regex reg(strregex);
    stream_list_t founds;
    map_streams_t::iterator iter = _streams.begin();
    for(iter = _streams.begin() ; iter != _streams.end() ; iter++){
        if(!iter->second.stream) {
            THROW_ED247_ERROR("Channel '" << get_name() << "': contains an invalid Stream at [" << iter->first << "]");
        }
        if(std::regex_match(iter->second.stream->get_name(), reg)){
            founds.push_back(iter->second.stream);
        }
    }
    return founds;
}

stream_ptr_t Channel::get_stream(std::string str_name)
{
    map_streams_t::iterator iter = _streams.begin();
    for(iter = _streams.begin() ; iter != _streams.end() ; iter++){
        if(!iter->second.stream) {
            THROW_ED247_ERROR("Channel '" << get_name() << "': contains an invalid Stream at [" << iter->first << "]");
        }
        if(iter->second.stream->get_name() == str_name) return iter->second.stream;
    }
    return nullptr;
}

uint32_t Channel::missed_frames()
{
    return _header.missed_frames();
}

// Channel::Pool

Channel::Pool::Pool(udp::receiver_set_t& context_receiver_set,
                    std::shared_ptr<ed247::StreamSet> & pool_streams):
                    _channels(std::make_shared<channel_list_t>()),
                    _context_receiver_set(context_receiver_set),
                    _pool_streams(pool_streams)
{
}

Channel::Pool::~Pool()
{
    _channels->clear();
}

channel_ptr_t Channel::Pool::get(const xml::Channel* configuration)
{
    static Channel::Builder builder;
    channel_ptr_t sp_channel;
    std::string name{configuration->_name};

    auto iter = std::find_if(_channels->begin(),_channels->end(),
        [&name](const channel_ptr_t & c){ return c->get_name() == name; });
    if(iter == _channels->end()){
        sp_channel = builder.create(configuration, _context_receiver_set, _pool_streams);
        _channels->push_back(sp_channel);
    }else{
        // sp_channel = *iter;
        THROW_ED247_ERROR("Channel [" << name << "] already exists");
    }

    return sp_channel;
}

std::vector<channel_ptr_t> Channel::Pool::find(std::string strregex)
{
    std::regex reg(strregex);
    std::vector<channel_ptr_t> founds;
    for(auto channel : *_channels){
        if(std::regex_match(channel->get_name(), reg)){
            founds.push_back(channel);
        }
    }
    return founds;
}

channel_ptr_t Channel::Pool::get(std::string str_name)
{
    for(auto channel : *_channels){
        if(channel->get_name() == str_name) return channel;
    }
    return nullptr;

}

std::shared_ptr<channel_list_t> Channel::Pool::channels()
{
    return _channels;
}

void Channel::Pool::encode(const ed247_uid_t & component_identifier)
{
    for(auto & c : *_channels){
        c->encode(component_identifier);
    }
}

uint32_t Channel::Pool::size() const
{
    return _channels->size();
}

void Channel::Pool::send()
{
    for(auto & c : *_channels){
        c->send();
    }
}

void Channel::Pool::encode_and_send(const ed247_uid_t & component_identifier)
{
    for(auto & c : *_channels) {
        // Hack ED247LIB-27
        // if send queue is greather than the number of samples allowed in one packet
        // we have to send several packets. So we loop until the queu is empty.
        while (c->has_samples_to_send()) {
            c->encode(component_identifier);
            c->send();
        }
    }
}

// Channel::Builder
channel_ptr_t Channel::Builder::create(const xml::Channel* configuration,
    udp::receiver_set_t& context_receiver_set,
    std::shared_ptr<ed247::StreamSet> & pool_streams) const
{
    auto sp_channel = std::make_shared<Channel>(configuration);
    sp_channel->_com_interface.load(configuration->_com_interface, context_receiver_set, std::bind(&Channel::decode, sp_channel.get(), std::placeholders::_1, std::placeholders::_2));

    for(auto& stream_configuration : configuration->_stream_list) {
      stream_ptr_t sp_stream = pool_streams->get(stream_configuration.get(), sp_channel.get());
      sp_channel->add_stream(*sp_stream, stream_configuration.get()->_direction);
    }

    sp_channel->allocate_buffer();
    sp_channel->populate_sstreams();

    return sp_channel;
}

}
