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

#ifndef _ED247_INTERNALS_H_
#define _ED247_INTERNALS_H_

/************
 * Includes *
 ************/

#include "ed247.h"
#include "ed247_types.h"
#include "ed247_logs.h"

#include <string>
#include <memory>
#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <iterator>
#include <vector>

#include <assert.h>
#include <chrono>

#ifdef __linux__
    #include <byteswap.h>
#elif __QNXNTO__
    #include <gulliver.h>
    #define bswap_16(x) ENDIAN_SWAP16(x)
    #define bswap_32(x) ENDIAN_SWAP32(x)
    #define bswap_64(x) ENDIAN_SWAP64(x)
#endif

#ifdef __unix__
    #include <fcntl.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <string.h>
    #include <netdb.h>
    #include <ifaddrs.h>
    #ifndef INVALID_SOCKET
        #define INVALID_SOCKET (-1)
    #endif
    #ifndef ED247_SOCKET
        #define ED247_SOCKET int
    #endif
#elif _WIN32
    #include <winsock2.h>
    #include <Ws2tcpip.h>
    #include <mswsock.h>
    #include <windows.h>
    #ifndef ED247_SOCKET
        #define ED247_SOCKET SOCKET
    #endif
    #ifndef _MSC_VER
        #define bswap_16(x) __builtin_bswap16(x)
        #define bswap_32(x) __builtin_bswap32(x)
        #define bswap_64(x) __builtin_bswap64(x)
    #else
        #define bswap_16(x) _byteswap_ushort(x)
        #define bswap_32(x) _byteswap_ulong(x)
        #define bswap_64(x) _byteswap_uint64(x)
    #endif
#endif

#ifndef _MSC_VER
    #include <unistd.h>
    #include <sys/time.h>
#endif

#ifndef NDEBUG
    #define DEBUG_ONLY if(true)
#else
    #define DEBUG_ONLY if(false)
#endif

#ifndef _UNUSED
#define _UNUSED(x) (void)x
#endif

/***********
 * Defines *
 ***********/

#if __cplusplus < 201402L

namespace std
{
    // std::make_unique only available since C++14
    template<class T> struct _Unique_if {
        typedef unique_ptr<T> _Single_object;
    };
    template<class T> struct _Unique_if<T[]> {
        typedef unique_ptr<T[]> _Unknown_bound;
    };
    template<class T, size_t N> struct _Unique_if<T[N]> {
        typedef void _Known_bound;
    };
#ifndef _MSC_VER
    template<class T, class... Args>
        typename _Unique_if<T>::_Single_object
        make_unique(Args&&... args) {
            return unique_ptr<T>(new T(std::forward<Args>(args)...));
        }
#endif
    template<class T>
        typename _Unique_if<T>::_Unknown_bound
        make_unique(size_t n) {
            typedef typename remove_extent<T>::type U;
            return unique_ptr<T>(new U[n]());
        }
    template<class T, class... Args>
        typename _Unique_if<T>::_Known_bound
        make_unique(Args&&...) = delete;
}

#endif

inline void hash_combine(std::size_t& seed) {_UNUSED(seed);}

template <typename T, typename... Rest>
inline void hash_combine(std::size_t& seed, const T& v, Rest... rest) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    hash_combine(seed, rest...);
}

template<typename T>
struct weak_hash : public std::unary_function<std::weak_ptr<T>, size_t>
{
    size_t operator()(const std::weak_ptr<T> & wp) const
    {
        auto sp = wp.lock();
        return std::hash<decltype(sp)>()(sp);
    }
};

template<typename T>
struct weak_equal_to : public std::unary_function<std::weak_ptr<T>, bool>
{
    bool operator()(const std::weak_ptr<T> & wx, const std::weak_ptr<T> & wy) const
    {
        return wx.lock() == wy.lock();
    }
};

#ifdef _MSC_VER
static int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970 
    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime( &system_time );
    SystemTimeToFileTime( &system_time, &file_time );
    time =  ((uint64_t)file_time.dwLowDateTime )      ;
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
    return 0;
}
#endif

uint64_t get_time_us();

template<ed247_stream_type_t E>
struct StreamTypeChecker
{
    static_assert(E != ED247_STREAM_TYPE__INVALID && E != ED247_STREAM_TYPE__COUNT, "Invalid stream type");
};

template<ed247_signal_type_t E>
struct SignalTypeChecker
{
    static_assert(E != ED247_SIGNAL_TYPE__INVALID, "Invalid signal type");
};

template<ed247_stream_type_t E>
struct StreamSignalTypeChecker
{
    static const bool value = 
        E == ED247_STREAM_TYPE_ANALOG ||
        E == ED247_STREAM_TYPE_DISCRETE ||
        E == ED247_STREAM_TYPE_NAD ||
        E == ED247_STREAM_TYPE_VNAD
        ;
};

/*********
 * Types *
 *********/

struct ed247_internal_context_t {};

struct ed247_internal_channel_t {};

struct ed247_internal_stream_t {};

struct ed247_internal_signal_t {};

struct ed247_internal_frame_list_t {};

struct ed247_internal_channel_list_t {};

struct ed247_internal_stream_list_t {};

struct ed247_internal_signal_list_t {};

struct ed247_internal_stream_assistant_t {};

struct ed247_internal_time_sample_t : public ed247_timestamp_t {};

namespace ed247
{

class Configuration
{
    public:
        static Configuration & getInstance() {
            static Configuration instance;
            return instance;
        }
        const libed247_configuration_t & get() const {
            return _configuration;
        }
        void set(const libed247_configuration_t & configuration) {
            _configuration = configuration;
        }

    private:
        libed247_configuration_t _configuration;
};

class SimulationTimeHandler
{
    public:
        static SimulationTimeHandler & get()
        {
            static SimulationTimeHandler s;
            return s;
        }

        void set_handler(libed247_set_simulation_time_ns_t handler, void *user_data)
        {
            _handler = handler;
            _user_data = user_data;
        }

        bool update_timestamp(ed247_timestamp_t & timestamp)
        {
            if(is_valid())
                return (*_handler)((ed247_internal_time_sample_t*)(&timestamp), _user_data) != ED247_STATUS_SUCCESS;
            else
                return true;
        }

        bool is_valid() const
        {
            return _handler != nullptr;
        }

    protected:
        libed247_set_simulation_time_ns_t _handler;
        void *_user_data;

        SimulationTimeHandler():
            _handler(nullptr),
            _user_data(nullptr)
        {}
};

template<typename T>
class SmartList : public std::vector<T>
{
    public:
        SmartList():
            _managed(false)
        {
            reset();
        }
        SmartList(std::vector<T> && other): std::vector<T>(other),
            _managed(false)
        {
            reset();
        }
        virtual ~SmartList() {};

        T * next()
        {
            next_iter();
            return current_value();
        }

        virtual T * next_ok()
        {
            return next();
        }

        void reset()
        {
            _iter = std::vector<T>::end();
        }

        void set_managed(bool managed) { _managed = managed; }
        bool managed() const { return _managed; }

        typename std::vector<T>::iterator _iter;
    protected:
        bool _managed;

        void next_iter()
        {
            if(_iter == std::vector<T>::end()){
                _iter = std::vector<T>::begin();
            }else{
                ++_iter;
            }
        }

        T * current_value()
        {
            if(_iter == std::vector<T>::end()){
                return nullptr;
            }else{
                return &(*_iter);
            }
        }
};

}

#endif