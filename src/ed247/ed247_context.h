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

#include "ed247_internals.h"
#include "ed247_xml.h"
#include "ed247_channel.h"

#include <vector>

namespace ed247
{

class Context : public ed247_internal_context_t
{
    public:

        explicit Context();
        ~Context();
        Context(const Context &)                = delete;
        Context(Context &&)                     = delete;
        Context & operator = (const Context &)  = delete;
        Context & operator = (Context &&)       = delete;
        template<typename T>
        Context(std::initializer_list<T>)       = delete;

        void initialize();

        const xml::Component* getConfiguration() { return _configuration.get(); }

  ed247::SignalSet* get_signal_set() { return &_signal_set; }  // TODO: ref

        ed247::StreamSet* get_stream_set() { return &_stream_set; }  // TODO: ref

        ed247::ChannelSet* get_channel_set() { return &_channel_set; }  // TODO: ref

        void send_pushed_samples()
        {
          for(auto & c : _channel_set.channels()) {
            c.second->encode_and_send();
          }
        }

        ed247_status_t wait_frame(int32_t timeout_us)
        {
            return _receiver_set.wait_frame(timeout_us);
        }
        ed247_status_t wait_during(int32_t duration_us)
        {
            return _receiver_set.wait_during(duration_us);
        }

        void set_user_data(void *user_data){
            _user_data = user_data;
        }

        void *get_user_data(){
            return _user_data;
        }

        class Builder
        {
            public:
                static Context * create_filepath(std::string ecic_filepath);
                static Context * create_content(std::string ecic_content);
                static void initialize(Context & context);

                Builder(const Builder &) = delete;
                Builder(Builder &&) = delete;
                Builder & operator = (const Builder &) = delete;
                Builder & operator = (Builder &&) = delete;

            private:
                Builder(){}

        };

    private:

        std::unique_ptr<xml::Component>  _configuration;
        udp::ReceiverSet                 _receiver_set;
        ed247::SignalSet                 _signal_set;
        ed247::StreamSet                 _stream_set;
        ed247::ChannelSet                _channel_set;
        void*                            _user_data;
};

}

#endif
