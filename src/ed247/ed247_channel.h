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

#ifndef ED247_CHANNEL_H_
#define ED247_CHANNEL_H_

#include "ed247_internals.h"
#include "ed247_xml.h"
#include "ed247_cominterface.h"
#include "ed247_stream.h"
#include "ed247_frame_header.h"

#include <memory>
#include <map>

namespace ed247
{
  class Channel;
  typedef std::shared_ptr<Channel>   channel_ptr_t;
  typedef std::vector<channel_ptr_t> channel_list_t;

class Channel : public ed247_internal_channel_t, public std::enable_shared_from_this<Channel>
{
    public:
        typedef struct {
            stream_ptr_t stream;
            ed247_direction_t direction;
        } stream_dir_t;
        using map_streams_t = std::map<ed247_uid_t,stream_dir_t>;

        Channel(const xml::Channel* configuration, ed247_uid_t component_identifier):
            _configuration(configuration),
            _sstreams(std::make_shared<stream_list_t>()),
            _header(configuration->_header, component_identifier, get_name()),
            _user_data(NULL)
        {
          MEMCHECK_NEW(this, "Channel " << _configuration->_name);
        }
        virtual ~Channel();

        void set_user_data(void *user_data)
        {
            _user_data = user_data;
        }

        void get_user_data(void **user_data)
        {
            *user_data = _user_data;
        }

        const xml::Channel * get_configuration() const { return _configuration; }

        const FrameHeader & get_header() const { return _header; }

        std::string get_name() const { return _configuration ? _configuration->_name : std::string(); }

        void add_stream(stream_ptr_t stream, ed247_direction_t direction)
        {
            PRINT_DEBUG("Channel [" << get_name() << "] append stream [" << stream->get_name() << "]");
            if(_streams.find(stream->get_uid()) != _streams.end())
                THROW_ED247_ERROR("Stream [" << stream->get_name() << "] uses an UID already registered in Channel [" << get_name() << "]");
            stream_dir_t stream_dir= {stream, direction};
            _streams.insert(std::make_pair(stream->get_uid(), stream_dir));
            PRINT_DEBUG("Size [" << _streams.size() << "]");
        }

        // Return true if at least one of the stream has samples in its send fifo
        bool has_samples_to_send();

        void send();

        // Return false on error
        void encode(const ed247_uid_t & component_identifier);
        bool decode(const char * frame, uint32_t frame_size);

        Sample & buffer() { return _buffer; }

        map_streams_t & streams() { return _streams; }

        stream_list_t find_streams(std::string strregex);

        stream_ptr_t get_stream(std::string str_name);

        std::shared_ptr<stream_list_t> sstreams() { return _sstreams; }

    protected:
        const xml::Channel* _configuration;
        udp::ComInterface _com_interface;
        map_streams_t _streams;
        std::shared_ptr<stream_list_t> _sstreams;
        FrameHeader _header;
        bool _updated{false};
        Sample _buffer;

        void allocate_buffer()
        {
            uint32_t capacity = 0;
            capacity += _header.get_size();
            for(auto & p : _streams){
                capacity += sizeof(ed247_uid_t) + sizeof(uint16_t);
                capacity += p.second.stream->get_max_size();
            }
            PRINT_DEBUG("Allocate Channel internal buffer with [" << capacity << "] bytes");
            _buffer.allocate(capacity);
        }

        void populate_sstreams()
        {
            for(auto & p : _streams){
                _sstreams->push_back(p.second.stream);
            }
            std::unique(_sstreams->begin(), _sstreams->end());
        }

        ED247_FRIEND_TEST();

    private:
        void *_user_data;

    public:

        class Pool {
            public:
                explicit Pool(udp::receiver_set_t& context_receiver_set,
                    std::shared_ptr<ed247::StreamSet> & pool_streams);
                ~Pool();

          channel_ptr_t get(const xml::Channel* configuration, ed247_uid_t component_identifier);

                std::vector<channel_ptr_t> find(std::string str_regex);

                channel_ptr_t get(std::string str_name);

                std::shared_ptr<channel_list_t> channels();

                void send();
                void encode(const ed247_uid_t & component_identifier);
                void encode_and_send(const ed247_uid_t & component_identifier);

                uint32_t size() const;

            private:
                std::shared_ptr<channel_list_t>    _channels;
                udp::receiver_set_t&               _context_receiver_set;
                std::shared_ptr<ed247::StreamSet>  _pool_streams;
        };

        class Builder{
            channel_ptr_t create(const xml::Channel* configuration,
                                 ed247_uid_t component_identifier,
                                 udp::receiver_set_t& context_receiver_set,
                                 std::shared_ptr<ed247::StreamSet> & pool_streams) const;
            friend class Pool;
        };
};

}

#endif
