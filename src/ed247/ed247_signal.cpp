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
#include "ed247_signal.h"
#include "ed247_sample.h"
#include <regex>

std::unique_ptr<ed247::Sample> ed247::signal::allocate_sample() const
{
  return std::unique_ptr<ed247::Sample>(new ed247::Sample(get_sample_max_size_bytes()));
}

ed247::signal_ptr_t ed247::signal_set_t::create(const xml::Signal* configuration, ed247_internal_stream_t* ed247_api_stream)
{
  auto result = _signals.emplace(std::make_pair(configuration->_name,
                                                signal_ptr_t(new signal(configuration, ed247_api_stream))));

  if (result.second == false) THROW_ED247_ERROR("Signal [" << configuration->_name << "] already exist !");

  return result.first->second;
}

ed247::signal_ptr_t ed247::signal_set_t::get(const std::string& name)
{
  auto iterator = _signals.find(name);
  if (iterator != _signals.end()) return iterator->second;
  return nullptr;
}

ed247::signal_list_t ed247::signal_set_t::find(const std::string& str_regex)
{
  std::regex reg(str_regex);
  signal_list_t founds;
  for(auto& signal_pair : _signals) {
    if(std::regex_match(signal_pair.first, reg)) {
      founds.push_back(signal_pair.second);
    }
  }
  return founds;
}

