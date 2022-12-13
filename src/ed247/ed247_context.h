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
#ifndef _ED247_CONTEXT_H_
#define _ED247_CONTEXT_H_
#include "ed247_xml.h"
#include "ed247_channel.h"

// base structures for C API
struct ed247_internal_context_t {};

namespace ed247
{

  class Context : public ed247_internal_context_t
  {
  public:
    static Context* create_from_filepath(std::string ecic_filepath);
    static Context* create_from_content(std::string ecic_content);

    Context(const Context &)             = delete;
    Context(Context &&)                  = delete;
    Context& operator=(const Context &)  = delete;
    Context& operator=(Context &&)       = delete;

    // configuration accessors
    const std::string& get_file_producer_identifier() { return _configuration->_file_producer_identifier; }
    const std::string& get_file_producer_comment()    { return _configuration->_file_producer_comment;    }
    const std::string& get_version()                  { return _configuration->_version;                  }
    ed247_component_type_t get_component_type()       { return _configuration->_component_type;           }
    const std::string& get_name()                     { return _configuration->_name;                     }
    const std::string& get_comment()                  { return _configuration->_comment;                  }
    ed247_uid_t get_identifier()                      { return _configuration->_identifier;               }
    ed247_standard_t get_standard_revision()          { return _configuration->_standard_revision;        }


    // Handle user-data
    void set_user_data(void *user_data)  { _user_data = user_data;  }
    void get_user_data(void **user_data) { *user_data = _user_data; }


    // Content access
    udp::ReceiverSet& get_receiver_set() { return _receiver_set; }
    SignalSet& get_signal_set()          { return _signal_set;   }
    StreamSet& get_stream_set()          { return _stream_set;   }
    ChannelSet& get_channel_set()        { return _channel_set;  }


    // Send all pushed streams in their respective channels/CommInterface
    void send_pushed_samples();

    // Receive frames and fill associated streams
    ed247_status_t wait_frame(int32_t timeout_us);
    ed247_status_t wait_during(int32_t duration_us);

  private:
    Context(std::unique_ptr<xml::Component>&& configuration);

    std::unique_ptr<xml::Component>  _configuration;
    udp::ReceiverSet                 _receiver_set;
    SignalSet                        _signal_set;
    StreamSet                        _stream_set;
    ChannelSet                       _channel_set;
    void*                            _user_data;
  };

}

#endif
