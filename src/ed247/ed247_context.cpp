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
#include "ed247_context.h"
#include "ed247_client_list.h"
#include "ed247_logs.h"
#include "ed247_stream_assistant.h"

//
// Client lists (ed247.h interface)
//
namespace ed247 {
  using ClientStreamList = ed247::client_list_container<ed247_internal_stream_list_t,
                                                        ed247::Stream,
                                                        ed247::stream_map_t,
                                                        ed247::ContextOwned::True>;

  using ClientChannelList = ed247::client_list_container<ed247_internal_channel_list_t,
                                                         ed247::Channel,
                                                         ed247::channel_map_t,
                                                         ed247::ContextOwned::True>;


  // A stream list where get_next() return the next stream which has received data
  struct ClientStreamListWithData : public ClientStreamList
  {
    ClientStreamListWithData(ed247::stream_map_t& container) :
      client_list_container(&container, false)
    {
    }

    virtual ed247::Stream* get_next() override
    {
      ClientStreamList::get_next();
      _iterator = std::find_if(_iterator,
                               _container->end(),
                               [](const ed247::stream_map_t::value_type& sp) {
                                 return sp.second->get_incoming_sample_number() > 0;
                               });
      return get_current();
    }
  };
}

//
// Context
//

ed247::Context* ed247::Context::create_from_filepath(std::string ecic_filepath)
{
  PRINT_DEBUG("ECIC filepath [" << ecic_filepath << "]");
  Context* context = new Context(xml::load_filepath(ecic_filepath));
  return context;
}

ed247::Context* ed247::Context::create_from_content(std::string ecic_content)
{
  PRINT_DEBUG("ECIC content [" << ecic_content << "]");
  Context* context = new Context(xml::load_content(ecic_content));
  return context;
}

ed247::Context::Context(std::unique_ptr<ed247::xml::Component>&& configuration):
  _configuration(std::move(configuration)),
  _stream_set(this),
  _channel_set(this),
  _client_streams(ed247::ClientStreamList::wrap(_stream_set.streams())),
  _client_streams_with_data(new ed247::ClientStreamListWithData(_stream_set.streams())),
  _client_channels(ed247::ClientChannelList::wrap(_channel_set.channels()))
{
  for(const xml::Channel& channel_configuration: _configuration->_channel_list) {
    _channel_set.create(&channel_configuration);
  }
}

bool ed247::Context::stream_assistants_written_push_samples(const ed247_timestamp_t* data_timestamp)
{
  for(auto& stream : _stream_set.get_streams_signals_output()) {
    if (stream->get_assistant()->push_if_was_written(data_timestamp, nullptr) == false) {
      return false;
    }
  }
  return true;
}

bool ed247::Context::stream_assistants_pop_samples()
{
  for(auto& stream : _stream_set.get_streams_signals_input()) {
    bool empty = false;
    while (empty == false) {
      if (stream->get_assistant()->pop(nullptr, nullptr, nullptr, &empty) == ED247_STATUS_FAILURE) {
        return false;
      }
    }
  }
  return true;
}


void ed247::Context::send_pushed_samples()
{
  for(auto& channel : _channel_set.channels()) {
    channel.second->encode_and_send();
  }
}

ed247_status_t ed247::Context::wait_frame(int32_t timeout_us)
{
  return _receiver_set.wait_frame(timeout_us);
}

ed247_status_t ed247::Context::wait_during(int32_t duration_us)
{
  return _receiver_set.wait_during(duration_us);
}
