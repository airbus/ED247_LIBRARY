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
#ifndef ED247_CHANNEL_H_
#define ED247_CHANNEL_H_
#include "ed247.h"
#include "ed247_xml.h"
#include "ed247_cominterface.h"
#include "ed247_stream.h"
#include "ed247_frame_header.h"

// base structures for C API
struct ed247_internal_channel_t {};


namespace ed247
{
  class Channel : public ed247_internal_channel_t
  {
  public:
    using map_uid_stream_t = std::unordered_map<ed247_uid_t, stream_ptr_t>;

    Channel(const xml::Channel* configuration,
            ed247_uid_t ec_id,
            udp::ReceiverSet& context_receiver_set,
            ed247::StreamSet& context_stream_set);
    ~Channel();

    // Configuration accessors
    const std::string& get_name() const                  { return _configuration->_name;                    }
    const std::string& get_comment() const               { return _configuration->_comment;                 }
    ed247_standard_t get_frame_standard_revision() const { return _configuration->_frame_standard_revision; }

    // Handle user-data
    void set_user_data(void *user_data)  { _user_data = user_data;  }
    void get_user_data(void **user_data) { *user_data = _user_data; }

    // Stream access
    stream_list_t find_streams(std::string strregex);
    stream_ptr_t get_stream(std::string str_name);
    map_uid_stream_t& streams() { return _streams; }

    // Encode the channel and send it.
    // Nothing is send if there are no data in streams.
    // In some cases, this function may send severals packets.
    void encode_and_send();

    // Decode frame and fill streams data
    // Return false if the frame cannot be decoded
    bool decode(const char* frame, uint32_t frame_size);

  private:
    const xml::Channel* _configuration;
    udp::ComInterface   _com_interface;
    map_uid_stream_t    _streams;
    FrameHeader         _header;
    Sample              _buffer;
    void*               _user_data;

    ED247_FRIEND_TEST();
  };

  typedef std::shared_ptr<Channel>                       channel_ptr_t;
  typedef std::vector<channel_ptr_t>                     channel_list_t;
  typedef std::unordered_map<std::string, channel_ptr_t> channel_map_t;

  class ChannelSet {
  public:
    ChannelSet(udp::ReceiverSet& context_receiver_set, ed247::StreamSet& pool_streams);
    channel_ptr_t create(const xml::Channel* configuration, ed247_uid_t ec_id);

    channel_ptr_t get(std::string str_name);
    channel_list_t find(std::string str_regex);

    channel_map_t& channels()  { return _channels;        }
    uint32_t size() const      { return _channels.size(); }

  private:
    channel_map_t      _channels;
    udp::ReceiverSet&  _context_receiver_set;
    ed247::StreamSet&  _pool_streams;
  };

}

#endif
