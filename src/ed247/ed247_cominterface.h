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

#ifndef _ED247_COMINTERFACE_H_
#define _ED247_COMINTERFACE_H_

#include "ed247_internals.h"
#include "ed247_xml.h"

#include <unordered_map>
#include <map>
#include <unordered_set>
#include <functional>
#include <memory>

namespace ed247
{

struct SocketInfos : public sockaddr_in
{

    static const uint16_t DEFAULT_PORT{1901};

    bool valid = false;
    std::vector<SocketInfos> mtc_groups;
    uint16_t ttl;

    SocketInfos()
    {
        sin_family = PF_INET;
        sin_addr.s_addr = htonl(INADDR_ANY);
        sin_port = DEFAULT_PORT;
        ttl = 1;
    }

    explicit SocketInfos(std::string ip_address, uint16_t ip_port, uint16_t new_ttl = 1)
    {
        sin_family = PF_INET;
#ifdef _MSC_VER
        if(ip_address.empty()){
            sin_addr.s_addr = htonl(INADDR_ANY);
        }else{
            inet_pton(AF_INET, ip_address.c_str(), &sin_addr.s_addr);
        }
#else
        sin_addr.s_addr = ip_address.empty() ? htonl(INADDR_ANY) : inet_addr(ip_address.c_str());
#endif
        sin_port = htons(ip_port == 0 ? DEFAULT_PORT : ip_port);
        ttl = new_ttl;
        valid = true;
    }

    SocketInfos(const SocketInfos & s)
    {
        sin_family = s.sin_family;
        sin_addr.s_addr = s.sin_addr.s_addr;
        sin_port = s.sin_port;
        ttl = s.ttl;
        valid = s.valid;
        mtc_groups = s.mtc_groups;
    }

    SocketInfos & operator = (const SocketInfos & s)
    {
        sin_family = s.sin_family;
        sin_addr.s_addr = s.sin_addr.s_addr;
        sin_port = s.sin_port;
        ttl = s.ttl;
        valid = s.valid;
        mtc_groups = s.mtc_groups;
        return *this;
    }

    bool is_default() const
    {
        return sin_port == htons(DEFAULT_PORT);
    }

    operator std::string() const
    {
        std::ostringstream oss;
        std::string ipaddr;
#ifdef _MSC_VER
        char straddr[INET_ADDRSTRLEN];
        static struct in_addr sin_addr_temp = sin_addr;
        InetNtop(AF_INET, &sin_addr_temp, straddr, INET_ADDRSTRLEN);
        ipaddr = std::string(straddr);
#elif __unix__
        char straddr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &sin_addr, straddr, INET_ADDRSTRLEN);
        ipaddr = std::string(straddr);
#else
        ipaddr = std::string(inet_ntoa(sin_addr));
#endif
        oss << ipaddr << ":" << ntohs(sin_port) << (!valid ? std::string("[NOK]") : std::string());
        return oss.str();
    }

    bool operator == (const SocketInfos & other) const
    {
        return sin_addr.s_addr == other.sin_addr.s_addr && 
            sin_port == other.sin_port &&
            valid == other.valid &&
            ttl == other.ttl;
    }
};

}

namespace std
{

template<> struct hash<ed247::SocketInfos>
{
    std::size_t operator () (const ed247::SocketInfos & infos) const
    {
        std::size_t ret = 0;
        hash_combine(ret,(uint64_t)infos.sin_addr.s_addr,(uint16_t)infos.sin_port);
        return ret;
    }
};

}

std::ostream & operator << (std::ostream & os, const ed247::SocketInfos & e);

bool operator == (const struct in_addr & x, const struct in_addr & y);

namespace ed247
{

class Channel;

class ComInterface : public std::enable_shared_from_this<ComInterface>
{
    public:
        virtual ~ComInterface() {}

        virtual void unregister_channel(Channel & channel) {
            _UNUSED(channel);
        }

        virtual std::string get_name() {
            return "";
        }
        
        virtual void send_frame(Channel & from, const void * frame, const size_t frame_size) = 0;

        ed247_status_t register_send_callback(ed247_com_callback_t callback, ed247_context_t context)
        {
            auto it = std::find_if(_send_callbacks.begin(), _send_callbacks.end(),
                [&context, &callback](const std::pair<ed247_context_t,ed247_com_callback_t> & element){
                    return element.first == context && element.second == callback;
                });
            if(it != _send_callbacks.end()){
                return ED247_STATUS_FAILURE;
            }
            _send_callbacks.push_back(std::make_pair(context,callback));
            return ED247_STATUS_SUCCESS;
        }

        ed247_status_t unregister_send_callback(ed247_com_callback_t callback, ed247_context_t context)
        {
            auto it = std::find_if(_send_callbacks.begin(), _send_callbacks.end(),
                [&context, &callback](const std::pair<ed247_context_t,ed247_com_callback_t> & element){
                    return element.first == context && element.second == callback;
                });
            if(it == _send_callbacks.end()){
                return ED247_STATUS_FAILURE;
            }
            _send_callbacks.erase(it);
            return ED247_STATUS_SUCCESS;
        }

        ed247_status_t register_recv_callback(ed247_com_callback_t callback, ed247_context_t context)
        {
            auto it = std::find_if(_recv_callbacks.begin(), _recv_callbacks.end(),
                [&context, &callback](const std::pair<ed247_context_t,ed247_com_callback_t> & element){
                    return element.first == context && element.second == callback;
                });
            if(it != _recv_callbacks.end()){
                return ED247_STATUS_FAILURE;
            }
            _recv_callbacks.push_back(std::make_pair(context, callback));
            return ED247_STATUS_SUCCESS;
        }

        ed247_status_t unregister_recv_callback(ed247_com_callback_t callback, ed247_context_t context)
        {
            auto it = std::find_if(_recv_callbacks.begin(), _recv_callbacks.end(),
                [&context, &callback](const std::pair<ed247_context_t,ed247_com_callback_t> & element){
                    return element.first == context && element.second == callback;
                });
            if(it == _recv_callbacks.end()){
                return ED247_STATUS_FAILURE;
            }
            _recv_callbacks.erase(it);
            return ED247_STATUS_SUCCESS;
        }

    protected:
        std::vector<std::weak_ptr<Channel>> _channels;
        std::vector<std::pair<ed247_context_t,ed247_com_callback_t>> _send_callbacks;
        std::vector<std::pair<ed247_context_t,ed247_com_callback_t>> _recv_callbacks;

        void run_send_callbacks()
        {
            for(auto & pair : _send_callbacks)
            {
                if(pair.second){
                    (*pair.second)(pair.first);
                }else
                    PRINT_WARNING("Callback [" << pair.second << "] is not callable.");
            }
        }

        void run_recv_callbacks()
        {
            for(auto & pair : _recv_callbacks)
            {
                if(pair.second){
                    (*pair.second)(pair.first);
                }else
                    PRINT_WARNING("Callback [" << pair.second << "] is not callable.");
            }
        }

    public:
        struct Pool : public std::enable_shared_from_this<Pool>
        {
            virtual ~Pool() {};
            
            virtual ed247_status_t wait_frame(int32_t timeout_us) {
                _UNUSED(timeout_us);
                return ED247_STATUS_FAILURE;
            };

            virtual ed247_status_t wait_during(int32_t duration_us) {
                _UNUSED(duration_us);
                return ED247_STATUS_FAILURE;
            };
        };
        class Builder
        {
            public:
                void build(std::shared_ptr<Pool> pool, const xml::ComInterface & configuration, Channel & channel);
        };
};

class UdpSocket : public ComInterface
{
    public:

        static const uint32_t MAX_FRAME_SIZE{65508};

        class Pair : public std::pair<std::shared_ptr<UdpSocket>,std::shared_ptr<UdpSocket>>
        {
            public:
                std::shared_ptr<UdpSocket> & emitter() { return first;  }
                std::shared_ptr<UdpSocket> & receiver() { return second; }
        };

        struct Frame
        {
            char    frame[MAX_FRAME_SIZE];
            size_t  size{MAX_FRAME_SIZE};
        };

        static std::string get_last_error();

#ifdef _MSC_VER
        using ip_address_t = struct ::in_addr;
#else
        using ip_address_t = struct in_addr;
#endif
        using socket_t = ED247_SOCKET;
        using map_destinations_t = std::unordered_map<Channel*,std::shared_ptr<std::vector<SocketInfos>>>;
        using sources_t = std::vector<Channel*>;
        using frame_t = Frame;

    public:
        UdpSocket(const SocketInfos & socket_infos):
            _socket_infos(socket_infos) {}
        ~UdpSocket() override
        {
            PRINT_DEBUG("# Delete Socket [" << _socket_infos << "]");
            close();
        };

        bool is_valid(){ return _socket_infos.valid; }
        virtual std::string get_name() final;
        const socket_t & get_socket() const { return _socket; }
        const SocketInfos & get_socket_infos() const { return _socket_infos; }

        virtual void send_frame(Channel & from, const void * frame, const size_t frame_size) final;
        bool recv();
        
        // Associate to a channel as an emitter
        void register_channel_emitter(Channel & channel, const xml::UdpSocket & configuration);
        // Associate to a channel as a receiver
        void register_channel_receiver(Channel & channel, const xml::UdpSocket & configuration);

        virtual void unregister_channel(Channel & channel) final;

        void get_recv_frame(const char * & frame, size_t & frame_size)
        {
            frame = _recv.frame;
            frame_size = _recv.size;
        }

    protected:
        SocketInfos                 _socket_infos; // Contains ip address & port to bind the socket on
        socket_t                    _socket{INVALID_SOCKET};
        map_destinations_t          _destinations;
        std::vector<ip_address_t>   _joined_multicast_groups; // Relevant only for mc inputs
        sources_t                   _sources;
        frame_t                     _recv;

        void initialize();

        void close();

    public:

        class Pool : public ComInterface::Pool
        {
            public:
                Pool();
                ~Pool() override;

                UdpSocket::Pair get(const xml::UdpSocket & configuration);

                virtual ed247_status_t wait_frame(int32_t timeout_us) final;
                virtual ed247_status_t wait_during(int32_t duration_us) final;

                ed247_status_t register_send_callback(ed247_com_callback_t callback, ed247_context_t context)
                {
                    ed247_status_t tmp_status = ED247_STATUS_FAILURE;
                    ed247_status_t status = ED247_STATUS_SUCCESS;
                    for(auto & i : _outputs){
                        tmp_status = i->register_send_callback(callback, context);
                        status = tmp_status == ED247_STATUS_FAILURE ? tmp_status : status;
                    }
                    status = tmp_status == ED247_STATUS_FAILURE ? tmp_status : status;
                    return status;
                }

                ed247_status_t unregister_send_callback(ed247_com_callback_t callback, ed247_context_t context)
                {
                    ed247_status_t tmp_status = ED247_STATUS_FAILURE;
                    ed247_status_t status = ED247_STATUS_SUCCESS;
                    for(auto & i : _outputs){
                        tmp_status = i->unregister_send_callback(callback, context);
                        status = tmp_status == ED247_STATUS_FAILURE ? tmp_status : status;
                    }
                    status = tmp_status == ED247_STATUS_FAILURE ? tmp_status : status;
                    return status;
                }

                ed247_status_t register_recv_callback(ed247_com_callback_t callback, ed247_context_t context)
                {
                    ed247_status_t tmp_status = ED247_STATUS_FAILURE;
                    ed247_status_t status = ED247_STATUS_SUCCESS;
                    for(auto & i : _inputs){
                        tmp_status = i->register_recv_callback(callback, context);
                        status = tmp_status == ED247_STATUS_FAILURE ? tmp_status : status;
                    }
                    status = tmp_status == ED247_STATUS_FAILURE ? tmp_status : status;
                    return status;
                }

                ed247_status_t unregister_recv_callback(ed247_com_callback_t callback, ed247_context_t context)
                {
                    ed247_status_t tmp_status = ED247_STATUS_FAILURE;
                    ed247_status_t status = ED247_STATUS_SUCCESS;
                    for(auto & i : _inputs){
                        tmp_status = i->unregister_recv_callback(callback, context);
                        status = tmp_status == ED247_STATUS_FAILURE ? tmp_status : status;
                    }
                    status = tmp_status == ED247_STATUS_FAILURE ? tmp_status : status;
                    return status;
                }

            private:
                std::vector<std::shared_ptr<UdpSocket>> _outputs;
                std::vector<std::shared_ptr<UdpSocket>> _inputs;
                struct select_options_s {
                    fd_set fd;
                    int nfds;
                } _select_options;
        };

        class Builder
        {
            void build(std::shared_ptr<UdpSocket::Pool> pool, const xml::UdpSocket & configuration, Channel & channel);
            friend class ComInterface::Builder;
        };

        class Factory
        {
            protected:

                static Factory & getInstance()
                {
                    static Factory factory;
                    if(factory._host_ip_addresses.empty())
                        factory.setup();
                    return factory;
                }

                Pair create(const xml::UdpSocket & configuration);

                void unregister(const SocketInfos & socket_infos);

                static std::vector<ip_address_t> host_ip_address();

            private:
                std::unordered_map<SocketInfos,std::shared_ptr<UdpSocket>> _sockets;
                std::vector<ip_address_t> _host_ip_addresses;

                Factory(){}

                void setup();

                std::shared_ptr<UdpSocket> find_or_create(SocketInfos & socket_infos);
                bool is_host_ip_address(const ip_address_t & ip_address);

            friend class UdpSocket::Pool;
        };
};

}

#endif