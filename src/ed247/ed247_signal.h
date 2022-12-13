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
#ifndef _ED247_SIGNAL_H_
#define _ED247_SIGNAL_H_
#include "ed247.h"
#include "ed247_xml.h"
#include "ed247_friend_test.h"
#include <memory>
#include <vector>
#include <unordered_map>


// base structures for C API
struct ed247_internal_signal_t {};
struct ed247_internal_stream_t;

namespace ed247
{
  class Signal : public ed247_internal_signal_t
  {
  public:
    Signal(const xml::Signal* configuration, ed247_internal_stream_t* ed247_api_stream);
    ~Signal();

    // No implicit copy
    Signal(const Signal & other) = delete;
    Signal& operator = (const Signal & other) = delete;

    // configuration accessors
    const std::string& get_name() const                      { return _configuration->_name;                        }
    const std::string& get_comment() const                   { return _configuration->_comment;                     }
    const std::string& get_icd() const                       { return _configuration->_icd;                         }
    ed247_signal_type_t get_type() const                     { return _configuration->_type;                        }
    uint32_t get_byte_offset() const                         { return _configuration->_byte_offset;                 }
    const std::string& get_analogue_electrical_unit() const  { return _configuration->_analogue_electrical_unit;    }

    ed247_nad_type_t get_nad_type() const                    { return _configuration->_nad_type;                    }
    uint32_t get_nad_type_size() const                       { return _configuration->get_nad_type_size();          }
    const std::string& get_nad_unit() const                  { return _configuration->_nad_unit;                    }
    const std::vector<uint32_t> get_nad_dimensions()         { return _configuration->_nad_dimensions;              }

    uint32_t get_vnad_position() const                       { return _configuration->_vnad_position;               }
    uint32_t get_vnad_max_number() const                     { return _configuration->_vnad_max_number;             }
    uint32_t get_sample_max_size_bytes() const               { return _configuration->get_sample_max_size_bytes();  }


    // implementation of ed247_signal_get_stream()
    ed247_internal_stream_t* get_api_stream() { return _ed247_api_stream; }

    // Handle user-data
    void set_user_data(void *user_data)  { _user_data = user_data;  }
    void get_user_data(void **user_data) { *user_data = _user_data; }


  private:
    const xml::Signal*       _configuration;
    ed247_internal_stream_t* _ed247_api_stream;  // Needed for API method ed247_signal_get_stream()
    void*                    _user_data;
  };


  typedef std::shared_ptr<Signal>   signal_ptr_t;
  typedef std::vector<signal_ptr_t> signal_list_t;


  class SignalSet
  {
  public:
    signal_ptr_t create(const xml::Signal* configuration, ed247_internal_stream_t* ed247_api_stream);
    signal_ptr_t get(const std::string& name);
    signal_list_t find(const std::string& regex);

    SignalSet();
    ~SignalSet();

    SignalSet& operator=(const SignalSet &)  = delete;
    SignalSet& operator=(SignalSet &&)       = delete;

  protected:
    ED247_FRIEND_TEST();
    std::unordered_map<std::string, signal_ptr_t> _signals;
  };
}

#endif
