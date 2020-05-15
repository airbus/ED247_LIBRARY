/******************************************************************************
 * The MIT Licence
 *
 * Copyright (c) 2020 Airbus Operations S.A.S
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
#include "ed247_memhooks.h"
#include "ed247_stream.h"

#include <regex>

#ifdef __linux__
#include <time.h>
#endif

namespace ed247
{

// Header

size_t FrameHeader::length()
{
    return _configuration.enable == ED247_YESNO_YES ? (sizeof(uint16_t)*2+sizeof(uint32_t)*2) : 0;
}

void FrameHeader::fill_transport_timestamp()
{
    if(_configuration.transport_timestamp == ED247_YESNO_NO)
        return;

#ifdef __linux__
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    _send_header.transport_timestamp.epoch_s = tp.tv_sec;
    _send_header.transport_timestamp.offset_ns = tp.tv_nsec;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    _send_header.transport_timestamp.epoch_s = tv.tv_sec;
    _send_header.transport_timestamp.offset_ns = tv.tv_usec * 1000LL;
#endif
}

void FrameHeader::encode(char * frame, size_t frame_capacity, size_t & frame_index, ed247_uid_t component_identifier)
{
    if(_configuration.enable == ED247_YESNO_YES){
        fill_transport_timestamp();
        memset(frame+frame_index, 0, sizeof(uint16_t)+sizeof(uint16_t)+sizeof(uint32_t)+sizeof(uint32_t));
        if(frame_index + sizeof(uint16_t) > frame_capacity)
            THROW_ED247_ERROR(ED247_STATUS_FAILURE, "Failed to write producer identifier in header (not enough space)");
        *(uint16_t*)(frame+frame_index) = htons(component_identifier);
        frame_index += sizeof(uint16_t);
        if(frame_index + sizeof(uint16_t) > frame_capacity)
            THROW_ED247_ERROR(ED247_STATUS_FAILURE, "Failed to write sequence number in header (not enough space)");
        *(uint16_t*)(frame+frame_index) = htons(_send_header.sequence_number);
        _send_header.sequence_number++;
        frame_index += sizeof(uint16_t);
        if(_configuration.transport_timestamp == ED247_YESNO_YES){
            if(frame_index + sizeof(ed247_timestamp_t) > frame_capacity)
                THROW_ED247_ERROR(ED247_STATUS_FAILURE, "Failed to write Transport timestamp in header (not enough space)");
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

void FrameHeader::decode(const char * frame, size_t frame_size, size_t & frame_index)
{
    frame_index = 0;
    // Increment the regular number of the sequence number
    // This number shall be rewriten if the sequence number is present in the header
    // In case of frames without the sequence number, it allows to count the number of frames that are discarded (missformated/corrupted)
    if(_configuration.enable == ED247_YESNO_YES){
        static header_element_t recv_header;
        if(frame_index + sizeof(uint16_t) > frame_size)
            THROW_ED247_ERROR(ED247_STATUS_FAILURE, "Failed to write producer identifier in header (not enough space)");
        recv_header.component_identifier = ntohs(*(uint16_t*)(frame+frame_index));
        frame_index += sizeof(uint16_t);
        if(frame_index + sizeof(uint16_t) > frame_size)
            THROW_ED247_ERROR(ED247_STATUS_FAILURE, "Failed to write sequence number in header (not enough space)");
        recv_header.sequence_number = ntohs(*(uint16_t*)(frame+frame_index));
        frame_index += sizeof(uint16_t);
        if(_configuration.transport_timestamp == ED247_YESNO_YES){
            if(frame_index + sizeof(ed247_timestamp_t) > frame_size)
                THROW_ED247_ERROR(ED247_STATUS_FAILURE, "Failed to write QoS timestamp in header (not enough space)");
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
            if(_recv_headers.size() == _recv_headers.capacity())
                THROW_ED247_ERROR(ED247_STATUS_FAILURE, "No more producer allowed for reception");
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
        }
    }
}

// Channel

Channel::~Channel()
{
    //_emitters.clear();
    for(auto iter = _emitters.begin() ; iter != _emitters.end() ; iter = _emitters.erase(iter)){
        auto sp_interface = iter->lock();
        if(sp_interface)
            sp_interface->unregister_channel(*this);
    }
    //_receivers.clear();
    for(auto iter = _receivers.begin() ; iter != _receivers.end() ; iter = _receivers.erase(iter)){
        auto sp_interface = iter->lock();
        if(sp_interface)
            sp_interface->unregister_channel(*this);
    }
}

void Channel::send()
{
    if(_buffer.empty()) return;
    for(auto & we : _emitters){
        we.lock()->send_frame(*this, _buffer.data(), _buffer.size());
    }
}

void Channel::encode(const ed247_uid_t & component_identifier)
{
    size_t buffer_index = 0;
    size_t stream_data_size = 0;
    // Encode header
    _header.encode((char*)_buffer.data(), _buffer.capacity(), buffer_index, component_identifier);
    // Encode channel payload
    if(!_configuration->simple){
        for(auto & p : _streams){
            auto & s = p.second.stream;
            if(!(p.second.direction & ED247_DIRECTION_OUT))
                continue;
            if(s->send_stack().size()){
                if(buffer_index + sizeof(ed247_uid_t) + sizeof(uint16_t) > _buffer.capacity())
                    THROW_ED247_ERROR(ED247_STATUS_FAILURE, "Channel buffer is too small !");
                // Write Stream UID
                ed247_uid_t sid = htons(s->get_configuration()->info.uid);
                memcpy((char *)_buffer.data() + buffer_index, &sid, sizeof(ed247_uid_t));
                buffer_index += sizeof(ed247_uid_t);
                // Write Stream sample data
                stream_data_size = s->encode((char*)_buffer.data()+buffer_index+sizeof(uint16_t),_buffer.capacity());
                // Write Stream sample size
                if(stream_data_size != (uint16_t)stream_data_size)
                    THROW_ED247_ERROR(ED247_STATUS_FAILURE, "Stream samples are too high !");
                uint16_t size = htons((uint16_t)stream_data_size);
                memcpy((char *)_buffer.data() + buffer_index, &size, sizeof(uint16_t));
                buffer_index += sizeof(uint16_t) + stream_data_size;
            }
        }
    }else{
        auto & s = _streams.begin()->second.stream;
        // Write Stream sample data
        stream_data_size = s->encode((char*)_buffer.data()+buffer_index,_buffer.capacity());
        buffer_index += stream_data_size;
    }
    _buffer.set_size(buffer_index);
}

bool Channel::decode(const char * frame, size_t frame_size)
{
    bool stop = false;
    // if(!MemoryHooksManager::getInstance().isEnabled())
    //     LOG_DEBUG() << "Channel [" << get_name() << "] decode a frame of size [" << frame_size << "]" << LOG_END;
    size_t frame_index = 0;
    _header.decode(frame, frame_size, frame_index);
    if(!_configuration->simple){
        while(frame_index < frame_size){
            if((frame_size - frame_index) < sizeof(ed247_uid_t))
                THROW_ED247_ERROR(ED247_STATUS_FAILURE, "Received frame is corrupted (wrong stream id size)");
            ed247_uid_t sid = ntohs(*(ed247_uid_t*)(frame+frame_index));
            frame_index += sizeof(ed247_uid_t);
            if((frame_size - frame_index) < sizeof(uint16_t))
                THROW_ED247_ERROR(ED247_STATUS_FAILURE, "Received frame is corrupted (wrong stream sample size size)");
            uint16_t stream_sample_size = ntohs(*(uint16_t*)(frame+frame_index));
            frame_index += sizeof(uint16_t);
            if(_streams[sid].direction & ED247_DIRECTION_IN){
                // If stream is not declared as input, do not decode stream data
                if(!_streams[sid].stream->decode(frame+frame_index, stream_sample_size, &_header)) stop = true;
            }
            frame_index += stream_sample_size;
        }
    }else{
        if(!_streams.begin()->second.stream->decode(frame+frame_index, frame_size-frame_index, &_header)) stop = true;
    }
    return !stop;
}

std::vector<std::shared_ptr<BaseStream>> Channel::find_streams(std::string strregex)
{
    std::regex reg(strregex);
    std::vector<std::shared_ptr<BaseStream>> founds;
    map_streams_t::iterator iter = _streams.begin();
    for(iter = _streams.begin() ; iter != _streams.end() ; iter++){
        if(!iter->second.stream)
            THROW_ED247_ERROR(ED247_STATUS_FAILURE, "Channel contains an invalid Stream at [" << iter->first << "]");
        if(std::regex_match(iter->second.stream->get_name(), reg)){
            founds.push_back(iter->second.stream);
        }
    }
    return std::move(founds);
}

uint32_t Channel::missed_frames()
{
    return _header.missed_frames();
}

// Channel::Pool

Channel::Pool::Pool(std::shared_ptr<ComInterface::Pool> & pool_interfaces,
                    std::shared_ptr<BaseStream::Pool> & pool_streams):
                    _channels(std::make_shared<SmartListChannels>()),
                    _pool_interfaces(pool_interfaces),
                    _pool_streams(pool_streams)
{
    _channels->set_managed(true);
}

Channel::Pool::~Pool()
{
    _channels->clear();
    _pool_interfaces.reset();
}

std::shared_ptr<Channel> Channel::Pool::get(std::shared_ptr<xml::Channel> & configuration)
{
    static Channel::Builder builder;
    std::shared_ptr<Channel> sp_channel;
    std::string name{configuration->info.name};

    auto iter = std::find_if(_channels->begin(),_channels->end(),
        [&name](const std::shared_ptr<Channel> & c){ return c->get_name() == name; });
    if(iter == _channels->end()){
        sp_channel = builder.create(configuration, _pool_interfaces, _pool_streams);
        _channels->push_back(sp_channel);
    }else{
        // sp_channel = *iter;
        THROW_ED247_ERROR(ED247_STATUS_FAILURE, "Channel [" << name << "] already exists");
    }
    
    return sp_channel;
}

std::vector<std::shared_ptr<Channel>> Channel::Pool::find(std::string strregex)
{
    std::regex reg(strregex);
    std::vector<std::shared_ptr<Channel>> founds;
    for(auto channel : *_channels){
        if(std::regex_match(channel->get_name(), reg)){
            founds.push_back(channel);
        }
    }
    return founds;
}

std::shared_ptr<SmartListChannels> Channel::Pool::channels()
{
    return _channels;
}

void Channel::Pool::encode(const ed247_uid_t & component_identifier)
{
    for(auto & c : *_channels){
        c->encode(component_identifier);
    }
}

size_t Channel::Pool::size() const
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
    for(auto & c : *_channels){
        c->encode(component_identifier);
        c->send();
    }
}

// Channel::Builder
std::shared_ptr<Channel> Channel::Builder::create(std::shared_ptr<xml::Channel> & configuration,
    std::shared_ptr<ComInterface::Pool> & pool_interfaces,
    std::shared_ptr<BaseStream::Pool> & pool_streams) const
{
    static ComInterface::Builder builder_interface;
    static BaseStream::Builder builder_streams;

    auto sp_channel = std::make_shared<Channel>(configuration);
    builder_interface.build(pool_interfaces, configuration->com_interface, *sp_channel);

    for(auto stream_configuration : configuration->streams){
        builder_streams.build(pool_streams, stream_configuration, *sp_channel);
    }

    sp_channel->allocate_buffer();
    sp_channel->populate_sstreams();

    return sp_channel;
}

}