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
  _stream_set(_signal_set),
  _channel_set(_receiver_set, _stream_set)
{
  for(const xml::Channel& channel_configuration: _configuration->_channel_list) {
    _channel_set.create(&channel_configuration, _configuration->_identifier);
  }
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
