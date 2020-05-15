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

#ifndef ED247_CHANNEL_H_
#define ED247_CHANNEL_H_

#include "ed247_internals.h"
#include "ed247_xml.h"
#include "ed247_cominterface.h"
#include "ed247_stream.h"

#include <memory>

namespace ed247
{

class SmartListChannels;

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

        FrameHeader() {}
        FrameHeader(const xml::Header & configuration):
            _send_header({0, 0, {0, 0}, 0}),
            _configuration(configuration)
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

        void encode(char * frame, size_t frame_capacity, size_t & frame_index, ed247_uid_t component_identifier);
        void decode(const char * frame, size_t frame_size, size_t & frame_index);

        size_t length();

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

        void fill_transport_timestamp();

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
};

class Channel : public ed247_internal_channel_t, public std::enable_shared_from_this<Channel>
{
    public:
        typedef struct {
            std::shared_ptr<BaseStream> stream;
            ed247_direction_t direction;
        } stream_dir_t;
        using map_streams_t = std::map<ed247_uid_t,stream_dir_t>;
        Channel() {}
        Channel(std::shared_ptr<xml::Channel> & configuration):
            _configuration(configuration),
            _sstreams(std::make_shared<SmartListStreams>()),
            _header(configuration->header)
        {
            _sstreams->set_managed(true);
        }
        virtual ~Channel();

        const xml::Channel * get_configuration() const { return _configuration.get(); }

        const FrameHeader & get_header() const { return _header; }

        std::string get_name() const { return _configuration ? std::string(_configuration->info.name) : std::string(); }

        void add_emitter(ComInterface & com_interface)
        {
            LOG_DEBUG() << "# Channel [" << get_name() << "] append emitter [" << com_interface.get_name() << "]" << LOG_END;
            _emitters.push_back(com_interface.shared_from_this());
        }

        bool has_emitter(const std::shared_ptr<ComInterface> & com_interface)
        {
            return std::find_if(_emitters.begin(), _emitters.end(),
                [&com_interface](const std::weak_ptr<ComInterface> & p){ return p.lock() == com_interface ; }) != _emitters.end();
        }

        void add_receiver(ComInterface & com_interface)
        {
            LOG_DEBUG() << "# Channel [" << get_name() << "] append receiver [" << com_interface.get_name() << "]" << LOG_END;
            _receivers.push_back(com_interface.shared_from_this());
        }

        bool has_receiver(const std::shared_ptr<ComInterface> & com_interface)
        {
            return std::find_if(_receivers.begin(), _receivers.end(),
                [&com_interface](const std::weak_ptr<ComInterface> & p){ return p.lock() == com_interface ; }) != _receivers.end();
        }

        void add_stream(BaseStream & stream, ed247_direction_t direction)
        {
            LOG_DEBUG() << "# Channel [" << get_name() << "] append stream [" << stream.get_name() << "]" << LOG_END;
            if(_streams.find(stream.get_configuration()->info.uid) != _streams.end())
                THROW_ED247_ERROR(ED247_STATUS_FAILURE, "Stream [" << stream.get_name() << "] uses an UID already registered in Channel [" << get_name() << "]");
            stream_dir_t stream_dir= {stream.shared_from_this(), direction};
            _streams.insert(std::make_pair(stream.get_configuration()->info.uid, stream_dir));
            LOG_DEBUG() << "# Size [" << _streams.size() << "]" << LOG_END;
        }

        void send();

        // Return 0 if not accepted, return 1 if decoded (can be dumped)
        void encode(const ed247_uid_t & component_identifier);
        bool decode(const char * frame, size_t frame_size);

        std::vector<std::weak_ptr<ComInterface>> & get_emitters() { return _emitters; }
        std::vector<std::weak_ptr<ComInterface>> & get_receivers() { return _receivers; }

        BaseSample & buffer() { return _buffer; }

        map_streams_t & streams() { return _streams; }

        std::vector<std::shared_ptr<BaseStream>> find_streams(std::string strregex);

        std::shared_ptr<SmartListStreams> sstreams() { return _sstreams; }

        uint32_t missed_frames();

    protected:
        std::shared_ptr<xml::Channel> _configuration;
        std::vector<std::weak_ptr<ComInterface>> _emitters;
        std::vector<std::weak_ptr<ComInterface>> _receivers;
        map_streams_t _streams;
        std::shared_ptr<SmartListStreams> _sstreams;
        FrameHeader _header;
        bool _updated{false};
        BaseSample _buffer;

        void allocate_buffer()
        {
            size_t capacity = 0;
            capacity += _header.length();
            for(auto & p : _streams){
                capacity += sizeof(ed247_uid_t) + sizeof(uint16_t);
                capacity += p.second.stream->buffer().capacity();
            }
            LOG_DEBUG() << "# Allocate Channel internal buffer with [" << capacity << "] bytes" << LOG_END;
            _buffer.allocate(capacity);
        }

        void populate_sstreams()
        {
            for(auto & p : _streams){
                _sstreams->push_back(p.second.stream);
            }
            std::unique(_sstreams->begin(), _sstreams->end());
            _sstreams->reset();
        }

    public:

        class Pool {
            public:
                explicit Pool(std::shared_ptr<ComInterface::Pool> & pool_interfaces,
                    std::shared_ptr<BaseStream::Pool> & pool_streams);
                ~Pool();

                std::shared_ptr<Channel> get(std::shared_ptr<xml::Channel> & configuration);

                std::vector<std::shared_ptr<Channel>> find(std::string str_regex);

                std::shared_ptr<SmartListChannels> channels();

                void send();
                void encode(const ed247_uid_t & component_identifier);
                void encode_and_send(const ed247_uid_t & component_identifier);

                size_t size() const;

            private:
                std::shared_ptr<SmartListChannels>      _channels;
                std::shared_ptr<ComInterface::Pool>     _pool_interfaces;
                std::shared_ptr<BaseStream::Pool>       _pool_streams;
        };

        class Builder{
            std::shared_ptr<Channel> create(std::shared_ptr<xml::Channel> & configuration,
                std::shared_ptr<ComInterface::Pool> & pool_interfaces,
                std::shared_ptr<BaseStream::Pool> & pool_streams) const;
            friend class Pool;
        };
};

class SmartListChannels : public SmartList<std::shared_ptr<Channel>>, public ed247_internal_channel_list_t
{
    public:
        using SmartList<std::shared_ptr<Channel>>::SmartList;
        virtual ~SmartListChannels() {}
};

}

#endif