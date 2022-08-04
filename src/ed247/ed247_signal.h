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

#include "ed247_internals.h"
#include "ed247_xml.h"

namespace ed247
{

class BaseSample;
class BaseSignal;
class BaseStream;

typedef std::shared_ptr<BaseSignal> signal_ptr_t;
typedef std::vector<signal_ptr_t>   signal_list_t;

template<ed247_stream_type_t E>
class Stream;

template<ed247_signal_type_t ... E>
struct SignalBuilder {
    SignalBuilder() {}
    signal_ptr_t create(const ed247_signal_type_t & type, std::shared_ptr<xml::Signal> & configuration, BaseStream & stream);
};

template<ed247_signal_type_t T, ed247_signal_type_t ... E>
struct SignalBuilder<T, E...> : public SignalBuilder<E...>, private SignalTypeChecker<T> {
    SignalBuilder() : SignalBuilder<E...>() {}
    signal_ptr_t create(const ed247_signal_type_t & type, std::shared_ptr<xml::Signal> & configuration, BaseStream & stream);
};

template<ed247_signal_type_t T>
struct SignalBuilder<T> : private SignalTypeChecker<T> {
    SignalBuilder() {}
    signal_ptr_t create(const ed247_signal_type_t & type, std::shared_ptr<xml::Signal> & configuration, BaseStream & stream);
};

class BaseSignal : public ed247_internal_signal_t, public std::enable_shared_from_this<BaseSignal>
{
    public:
        BaseSignal(){}
        BaseSignal(std::shared_ptr<xml::Signal> & configuration, std::shared_ptr<BaseStream> & stream):
            _configuration(configuration),
            _stream(stream),
            _user_data(nullptr)
        {}

        virtual ~BaseSignal(){}

        void set_user_data(void *user_data)
        {
            _user_data = user_data;
        }

        void get_user_data(void **user_data)
        {
            *user_data = _user_data;
        }

        static uint32_t nad_type_size(const ed247_nad_type_t & nad_type)
        {
            switch(nad_type){
                case ED247_NAD_TYPE_INT8:       return sizeof(int8_t);
                case ED247_NAD_TYPE_INT16:      return sizeof(int16_t);
                case ED247_NAD_TYPE_INT32:      return sizeof(int32_t);
                case ED247_NAD_TYPE_INT64:      return sizeof(int64_t);
                case ED247_NAD_TYPE_UINT8:      return sizeof(uint8_t);
                case ED247_NAD_TYPE_UINT16:     return sizeof(uint16_t);
                case ED247_NAD_TYPE_UINT32:     return sizeof(uint32_t);
                case ED247_NAD_TYPE_UINT64:     return sizeof(uint64_t);
                case ED247_NAD_TYPE_FLOAT32:    return sizeof(float);
                case ED247_NAD_TYPE_FLOAT64:    return sizeof(double);
                default:                        return 0;
            }
        }

        uint32_t get_sample_max_size_bytes() const
        {
          return _configuration->get_sample_max_size_bytes();
        }

        const xml::Signal * get_configuration() const
        {
            return _configuration.get();
        }

        std::string get_name() const
        {
            return _configuration ? std::string(_configuration->_name) : std::string();
        }

        std::shared_ptr<BaseStream> get_stream()
        {
            return _stream.lock();
        }

        std::unique_ptr<BaseSample> allocate_sample() const;

        uint32_t position() const { return _configuration->_position; }

    protected:
        std::shared_ptr<xml::Signal> _configuration;
        std::weak_ptr<BaseStream> _stream;

    private:
        void *_user_data;

    public:
        class Pool : public std::enable_shared_from_this<Pool>
        {
            public:
                Pool(){}
                ~Pool(){};

                signal_ptr_t get(std::shared_ptr<xml::Signal> & configuration, BaseStream & stream);

                signal_list_t find(std::string str_regex);

                signal_ptr_t get(std::string str_name);

                signal_list_t & signals() { return _signals; }

                uint32_t size() const;

            private:
                signal_list_t _signals;
                SignalBuilder<
                    ED247_SIGNAL_TYPE_DISCRETE,
                    ED247_SIGNAL_TYPE_ANALOG,
                    ED247_SIGNAL_TYPE_NAD,
                    ED247_SIGNAL_TYPE_VNAD> _builder;
                
        };
        class Builder
        {
            public:
                signal_ptr_t build(std::shared_ptr<Pool> & pool, std::shared_ptr<xml::Signal> & configuration, BaseStream & stream) const;
        };
};

template<ed247_signal_type_t E>
class Signal : public BaseSignal, private SignalTypeChecker<E>
{
    public:
        const ed247_signal_type_t type {E};

        using BaseSignal::BaseSignal;

    public:
        class Builder
        {
            public:
                std::shared_ptr<Signal<E>> create(std::shared_ptr<xml::Signal> & configuration, BaseStream & stream) const;
        };
};

}

#endif
