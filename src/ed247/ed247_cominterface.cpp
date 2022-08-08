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


#include "ed247_cominterface.h"
#include "ed247_channel.h"
#include "ed247_internals.h"
#include "ed247_time.h"

#include <utility>
#include <memory>
#include <string>

#ifdef __unix__
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#endif
namespace
{

#ifndef _MSC_VER
  static const __attribute__((__unused__)) int zero = 0;
  static const __attribute__((__unused__)) int one = 1;
#else
  int zero = 0;
  int one = 1;
#endif


  static void sleep_us(uint32_t duration_us)
  {
#ifdef _WIN32
    Sleep(duration_us / 1000);
#else
    struct timespec ts;
    ts.tv_sec = duration_us / (1000 * 1000);
    ts.tv_nsec = (duration_us % (1000 * 1000)) * 1000;
    nanosleep(&ts, NULL);
#endif
  }
}

std::ostream & operator << (std::ostream & os, const ed247::SocketInfos & e)
{
    return os << std::string(e);
}

bool operator == (const struct in_addr & x, const struct in_addr & y)
{
    return x.s_addr == y.s_addr;
}

namespace ed247
{

// ComInterface

void ComInterface::Builder::build(std::shared_ptr<Pool> pool, const xml::ComInterface & configuration, Channel & channel)
{
    UdpSocket::Builder udp_socket_builder;
    auto udp_socket_pool = std::dynamic_pointer_cast<UdpSocket::Pool>(pool);
    if(!udp_socket_pool)
        THROW_ED247_ERROR("Failed to retrieve the UdpSocket pool from base ComInterface pool");
    for(auto & sp_udp_socket_configuration : configuration.udp_sockets){
        udp_socket_builder.build(udp_socket_pool, *sp_udp_socket_configuration, channel);
    }
}

// UdpSocket

std::string UdpSocket::get_last_error()
{
#ifdef _WIN32
    LPSTR messageBuffer = nullptr;
    DWORD dwError = WSAGetLastError();
    uint32_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    char errmsg[1024];
    strerror_s(errmsg, 1024, errno);
    std::string err = std::string(messageBuffer, size);
    LocalFree(messageBuffer);
    return err + " | " + std::string(errmsg);
#else
    return std::string(strerror(errno));
#endif
}

void UdpSocket::send_frame(Channel & channel, const void * frame, const uint32_t frame_size)
{
    PRINT_CRAZY("Socket [" << _socket_infos << "] sends a frame of channel [" << channel.get_name() << "]");
    auto & destinations = *(_destinations[&channel]);
    for(auto & destination : destinations){
        PRINT_CRAZY("Socket [" << _socket_infos << "] send to [" << destination << "] a frame of " << frame_size << " bytes: [" << hex_stream(frame, frame_size) << "]");
        int32_t sent_size = sendto(_socket, static_cast<const char *>(frame), (int)frame_size, 0, (struct sockaddr *)&destination, sizeof(struct sockaddr_in));
        if(sent_size < 0 || (uint32_t)sent_size != frame_size) {
          PRINT_ERROR("Failed to send frame from socket socket [" << _socket_infos << "] to [" << destination << "] (" << UdpSocket::get_last_error() << ")");
        }
    }
}

void UdpSocket::recv()
{
    int sockerr = 1;
    static SocketInfos client_infos;
    static socklen_t client_infos_size = sizeof(struct sockaddr);
    bool first_loop = true;

    while(sockerr > 0) {
        sockerr = ::recvfrom(_socket, _recv.frame, MAX_FRAME_SIZE, 0, (struct sockaddr *)&client_infos, &client_infos_size);
        if(sockerr <= 0)
            continue;

        first_loop = false;

        _recv.size = sockerr;
        PRINT_CRAZY("Received frame of " << _recv.size << " bytes: [" << hex_stream(_recv.frame, _recv.size) << "]");

        // Decode frames in channels
        for(auto & p_channel : _sources){
            if(p_channel) {
              // Ignore decode error: process next frame
              p_channel->decode(_recv.frame, _recv.size);
            }
        }
    }
    // Check if an error occured during the first loop
    if(first_loop && sockerr <= 0) {
      PRINT_ERROR("Failed to receive data on socket [" << _socket_infos << "], at recvfrom() level (" << sockerr << ":" << UdpSocket::get_last_error() << ")");
    }
}

std::string UdpSocket::get_name()
{
  return strize() << "UdpSocket[" << _socket_infos << "]";
}

void UdpSocket::initialize()
{
    PRINT_DEBUG("Initialize socket [" << _socket_infos << "]");
    int sockerr;

    if(_socket != INVALID_SOCKET)
        THROW_ED247_ERROR("Failed to initialize socket [" << _socket_infos << "], already initialized !");

    PRINT_DEBUG("Create socket [" << _socket_infos << "]");
    _socket = ::socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if(_socket == INVALID_SOCKET)
        THROW_ED247_ERROR("Failed to create the socket [" << _socket_infos << "]");

    // Allow to reuse port when socket is dying (restart)
    // On Windows, SO_REUSEADDR mean SO_REUSEPORT + SO_REUSEADDR.
    // With the SO_REUSEPORT option, 2 softwares can bind to the same ip/port, which
    // might have stange behaviors...
    sockerr = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&one, sizeof(one));
    if(sockerr){
        close();
        THROW_ED247_ERROR("Failed to setup socket options [SO_REUSEADDR] on socket [" << _socket_infos << "] (" << sockerr << ":" << UdpSocket::get_last_error() << ")");
    }

    // Set to non blocking
#ifdef _WIN32
    unsigned long ul_one = 1;
    sockerr = ioctlsocket(_socket, FIONBIO, (unsigned long *)&ul_one);
    if(sockerr != NO_ERROR){
        close();
        THROW_ED247_ERROR("Failed to setup socket to non-blocking [" << _socket_infos << "] (" << sockerr << ":" << UdpSocket::get_last_error() << ")");
    }
#else
    int flags = fcntl(_socket, F_GETFL, 0);
    if(fcntl(_socket, F_SETFL, flags | O_NONBLOCK) == -1){
        close();
        THROW_ED247_ERROR("Failed to setup socket to non-blocking [" << _socket_infos << "] (" << sockerr << ":" << UdpSocket::get_last_error() << ")");
    }
#endif

    sockerr = bind(_socket, (struct sockaddr *)&_socket_infos, sizeof(struct sockaddr_in));
    if(sockerr){
        close();
        THROW_ED247_ERROR("Failed to bind socket [" << _socket_infos << "] (" << sockerr << ":" << UdpSocket::get_last_error() << ")");
    }
}

void UdpSocket::register_channel_emitter(Channel & channel, const xml::UdpSocket & configuration)
{
    SocketInfos socket_infos_dst{configuration.dst_ip_address,configuration.dst_ip_port,configuration.mc_ttl};

    auto dest_iter = _destinations.find(&channel);
    std::weak_ptr<Channel> weak_channel = channel.shared_from_this();
    if(dest_iter == _destinations.end())
        dest_iter = (_destinations.insert(std::make_pair(&channel,std::make_shared<std::vector<SocketInfos>>()))).first;
    PRINT_DEBUG("Socket [" << _socket_infos << "] appends the destination [" << socket_infos_dst << "] for channel [" << channel.get_configuration()->_name << "]");
    dest_iter->second->push_back(socket_infos_dst);
    channel.add_emitter(*this);

    // Setup multicast options (output)
    if(IN_MULTICAST(ntohl(socket_infos_dst.sin_addr.s_addr))){
        // If the outgoing interface is specified, set-it
        struct ::in_addr iaddr;
        int sockerr;
        memset(&iaddr, 0, sizeof(struct ::in_addr));
        iaddr.s_addr = _socket_infos.sin_addr.s_addr;
        sockerr = setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_IF, (const char*) &iaddr, sizeof(struct ::in_addr));
        if(sockerr){
            close();
            THROW_ED247_ERROR("Failed to setup the outgoing interface of socket [" << _socket_infos << "] (" << sockerr << ":" << UdpSocket::get_last_error() << ")");
        }

        // Set multicast packet TTL
        uint32_t ttl = socket_infos_dst.ttl;
        sockerr = setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_TTL, (const char*) &ttl, sizeof(ttl));
        if(sockerr){
            close();
            THROW_ED247_ERROR("Failed to setup the outgoing interface of socket [" << _socket_infos << "] (" << sockerr << ":" << UdpSocket::get_last_error() << ")");
        }

        // Send multicast traffic to myself too
        sockerr = setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_LOOP, (const char*) &one, sizeof(one));
        if(sockerr){
            close();
            THROW_ED247_ERROR("Failed to setup the outgoing interface of socket [" << _socket_infos << "] (" << sockerr << ":" << UdpSocket::get_last_error() << ")");
        }
    }
}

void UdpSocket::register_channel_receiver(Channel & channel, const xml::UdpSocket & configuration)
{
    SocketInfos socket_infos_dst{configuration.dst_ip_address, configuration.dst_ip_port};

    if(std::find(_sources.begin(), _sources.end(), &channel) != _sources.end())
        THROW_ED247_ERROR("Channel [" << channel.get_name() << "] has already been registered as incoming in socket [" << _socket_infos << "]");

    _sources.push_back(&channel);

    if(_socket_infos.is_default()) // Do not receive on default emitter port
        return;

    channel.add_receiver(*this);

    // Setup multicast options (input)
    if(IN_MULTICAST(ntohl(socket_infos_dst.sin_addr.s_addr)) &&
        std::find(_socket_infos.mtc_groups.begin(), _socket_infos.mtc_groups.end(), socket_infos_dst) == _socket_infos.mtc_groups.end()){
        // Join the multicast group
        struct ip_mreq imreq;
        int sockerr;
        memset(&imreq, 0, sizeof(struct ip_mreq));
        imreq.imr_multiaddr.s_addr = socket_infos_dst.sin_addr.s_addr;
        imreq.imr_interface.s_addr = _socket_infos.sin_addr.s_addr;
        sockerr = setsockopt(_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&imreq, sizeof(struct ip_mreq));
        if(sockerr){
            close();
            THROW_ED247_ERROR("The socket [" << _socket_infos << "] failed to join the multicast group [" << configuration.dst_ip_address << "] (" << sockerr << ":" << UdpSocket::get_last_error() << ")");
        }
        _socket_infos.mtc_groups.push_back(socket_infos_dst);
    }
}

void UdpSocket::unregister_channel(Channel & channel)
{
    auto sp_interface = std::dynamic_pointer_cast<ComInterface>(shared_from_this());
    if(channel.has_emitter(sp_interface)){
        auto iter = _destinations.find(&channel);
        if(iter != _destinations.end())
            _destinations.erase(iter);
    }
    if(channel.has_receiver(sp_interface)){
        auto iter = std::find(_sources.begin(), _sources.end(), &channel);
        if(iter != _sources.end())
            _sources.erase(iter);
    }
}

void UdpSocket::close()
{
    PRINT_DEBUG("Close socket [" << _socket_infos << "]");

    if(_socket != INVALID_SOCKET){
        shutdown(_socket, 2);
#ifdef __unix__
        ::close(_socket);
#elif _WIN32
        closesocket(_socket);
#endif
        _socket = INVALID_SOCKET;
    }
}

// UdpSocket::Builder

void UdpSocket::Builder::build(std::shared_ptr<UdpSocket::Pool> pool, const xml::UdpSocket & configuration, Channel & channel)
{
    auto socket_pair = pool->get(configuration);
    if(socket_pair.emitter())
        socket_pair.emitter()->register_channel_emitter(channel, configuration);
    if(socket_pair.receiver())
        socket_pair.receiver()->register_channel_receiver(channel, configuration);
}

// UdpSocket::Factory

UdpSocket::Pair UdpSocket::Factory::create(const xml::UdpSocket & configuration)
{
    Pair socket_pair;

    SocketInfos socket_infos_dst{configuration.dst_ip_address, configuration.dst_ip_port};
    SocketInfos socket_infos_src{configuration.src_ip_address, configuration.src_ip_port};
    SocketInfos socket_infos_dst_mtc{configuration.mc_ip_address, configuration.dst_ip_port, configuration.mc_ttl};
    SocketInfos socket_infos_src_mtc{configuration.mc_ip_address, configuration.src_ip_port};

    bool is_multicast = IN_MULTICAST(ntohl(socket_infos_dst.sin_addr.s_addr));

    if(!is_multicast){
        // UNICAST
        if(is_host_ip_address(socket_infos_dst.sin_addr) && (configuration.direction & ED247_DIRECTION_IN)){
            // receiver (dst)
            socket_pair.second = find_or_create(socket_infos_dst);
        }
        if(configuration.direction & ED247_DIRECTION_OUT){
            // emitter (src)
            socket_pair.first = find_or_create(socket_infos_src);
        }
    }else{
        // MULTICAST ()
        if(configuration.direction & ED247_DIRECTION_IN){
            // receiver (mtc)
            socket_pair.second = find_or_create(socket_infos_dst_mtc);
        }
        if(configuration.direction & ED247_DIRECTION_OUT){
            // emitter (if src specified, overrides mtc)
            if(!configuration.src_ip_address.empty() &&
                is_host_ip_address(socket_infos_src.sin_addr)){
                // emitter (src)
                socket_pair.first = find_or_create(socket_infos_src);
            }else{
                // emitter (mtc)
                socket_pair.first = find_or_create(socket_infos_src_mtc);
            }
        }
    }

    PRINT_INFO("Socket[" << configuration.toString() << "] " <<
    "[" << (is_multicast ? std::string("MULTICAST") : std::string("UNICAST")) << "] " <<
    "Emitter[" << (socket_pair.emitter() ? std::string(socket_pair.emitter()->_socket_infos) : std::string("NONE")) << "] " <<
    "Receiver[" << (socket_pair.receiver() ? std::string(socket_pair.receiver()->_socket_infos) : std::string("NONE")) << "]");

    return socket_pair;
}

void UdpSocket::Factory::unregister(const SocketInfos & socket_infos)
{
    auto iter = _sockets.find(socket_infos);
    if(iter != _sockets.end()){
        if(iter->second->_destinations.size() == 0 && iter->second->_sources.size() == 0){
            PRINT_DEBUG("Socket [" << socket_infos << "] is not needed anymore, destroy it");
            _sockets.erase(iter);
        }else{
            PRINT_DEBUG("Socket [" << socket_infos << "] is used elsewhere, do not destroy it");
        }
    }
}

void UdpSocket::Factory::setup()
{
#ifdef _WIN32
    char host_buffer[256];
    WSADATA wsaData;
    struct addrinfo *info;
    struct addrinfo *iter;
    std::string ipaddr;

    WSAStartup(MAKEWORD(2, 2), &wsaData);

    if(gethostname(host_buffer, sizeof(host_buffer)) == -1)
        THROW_ED247_ERROR("Failed to retrieve hostname.");

    PRINT_DEBUG("Host [" << std::string(host_buffer) << "]");

    if(getaddrinfo(host_buffer, nullptr, nullptr, &info) != 0)
        THROW_ED247_ERROR("Failed to retrieve host addr info");

    for(iter = info ; iter != nullptr ; iter = iter->ai_next){
        if(iter->ai_family == AF_INET){
            _host_ip_addresses.emplace_back(((sockaddr_in*)iter->ai_addr)->sin_addr);
#ifdef _MSC_VER
            char straddr[INET_ADDRSTRLEN];
            InetNtop(AF_INET, &_host_ip_addresses.back(), straddr, INET_ADDRSTRLEN);
            ipaddr = std::string(straddr);
#elif __unix__
            char straddr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &_host_ip_addresses.back(), straddr, INET_ADDRSTRLEN);
            ipaddr = std::string(straddr);
#else
            ipaddr = std::string(inet_ntoa(_host_ip_addresses.back()));
#endif
            PRINT_DEBUG("Available ip address [" << ipaddr << "]");
        }
    }
#else
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;

    getifaddrs (&ifap);
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr != nullptr && ifa->ifa_addr->sa_family==AF_INET) {
            sa = (struct sockaddr_in *) ifa->ifa_addr;
            _host_ip_addresses.emplace_back(sa->sin_addr);
            PRINT_DEBUG("Available ip address [" << std::string(inet_ntoa(_host_ip_addresses.back())) << "]");
        }
    }

    freeifaddrs(ifap);
#endif
}

std::shared_ptr<UdpSocket> UdpSocket::Factory::find_or_create(SocketInfos & socket_infos)
{
    auto socket_iter = _sockets.find(socket_infos);
    if(socket_iter == _sockets.end()){
        std::shared_ptr<UdpSocket> socket = std::make_shared<UdpSocket>(socket_infos);
        _sockets.insert(std::make_pair(socket_infos,socket));
        socket->initialize();
        PRINT_DEBUG("Socket [" << socket_infos << "] created");
        return socket;
    }else{
        PRINT_DEBUG("Socket [" << socket_infos << "] found (Destinations [" << socket_iter->second->_destinations.size() << "] / Receivers [" << socket_iter->second->_sources.size() << "])");
        return socket_iter->second;
    }
}

bool UdpSocket::Factory::is_host_ip_address(const ip_address_t & ip_address)
{
    return ip_address.s_addr == htonl(INADDR_LOOPBACK) || std::find(_host_ip_addresses.begin(),_host_ip_addresses.end(),ip_address) != _host_ip_addresses.end();
}

// UdpSocket::Pool

UdpSocket::Pool::Pool()
{
    FD_ZERO(&_select_options.fd);
    _select_options.nfds = 0;
}

UdpSocket::Pool::~Pool()
{
    for(auto iter = _outputs.begin() ; iter != _outputs.end(); ){
        SocketInfos & socket_infos = (*iter)->_socket_infos;
        iter = _outputs.erase(iter);
        UdpSocket::Factory::getInstance().unregister(socket_infos);
    }
    for(auto iter = _inputs.begin() ; iter != _inputs.end(); ){
        SocketInfos & socket_infos = (*iter)->_socket_infos;
        iter = _inputs.erase(iter);
        UdpSocket::Factory::getInstance().unregister(socket_infos);
    }
}

UdpSocket::Pair UdpSocket::Pool::get(const xml::UdpSocket & configuration)
{
    auto socket_pair = UdpSocket::Factory::getInstance().create(configuration);

    if(socket_pair.emitter() &&
        std::find(_outputs.begin(),_outputs.end(),socket_pair.emitter()) == _outputs.end())
        _outputs.push_back(socket_pair.emitter());

    if(socket_pair.receiver() &&
        std::find(_inputs.begin(),_inputs.end(),socket_pair.receiver()) == _inputs.end()){
        _inputs.push_back(socket_pair.receiver());
        // Update select options
        ED247_SOCKET socket = socket_pair.receiver()->get_socket();
        _select_options.nfds = (std::max)((int)(socket+1), _select_options.nfds);
        FD_SET(socket, &_select_options.fd);
    }

    return socket_pair;
}

ed247_status_t UdpSocket::Pool::wait_frame(int32_t timeout_us)
{
    PRINT_CRAZY("Socket pool waiting for first frame to be received");

    if (_select_options.nfds <= 0) {
      PRINT_DEBUG("wait_frame: No socket opened in reading (no input messages in ECIC ?)");
      // Select will fail on Windows without errors. Simulate the wait for nothing.
      sleep_us(timeout_us);
      return ED247_STATUS_TIMEOUT;
    };

    ed247_status_t  status = ED247_STATUS_TIMEOUT;
    struct ::timeval timeout;
    int sockerr = 1;
    fd_set select_fd;

    if(timeout_us >= 0){
        timeout.tv_sec = (uint32_t)timeout_us / 1000000;
        timeout.tv_usec = (uint32_t)timeout_us % 1000000;
    }

    do {
        memcpy(&select_fd, &_select_options.fd, sizeof(fd_set));
        sockerr = select(_select_options.nfds, &select_fd, NULL, NULL, timeout_us >= 0 ? &timeout : NULL);
        PRINT_CRAZY("wait_frame select result: " << sockerr << ", errno: " << strerror(errno));
        if(sockerr > 0){
            // Something received
            PRINT_CRAZY("Data received !");
            for(auto & input : _inputs){
                const auto & socket = input->get_socket();
                if(socket != INVALID_SOCKET && FD_ISSET(socket, &select_fd)){
                    PRINT_CRAZY("Data received on [" << input->get_socket_infos() << "]");
                    input->recv();
                    status = ED247_STATUS_SUCCESS;
                }
            }
            // Discard timeout as something has been received
            timeout.tv_sec = 0;
            timeout.tv_usec = 0;
        }
    }while(sockerr > 0 || (sockerr < 0 && errno == EINTR));

    if(sockerr < 0 && _select_options.nfds > 0){
        PRINT_ERROR("Failed to perform select() (" << sockerr << ":" << get_last_error() << ")");
        status = ED247_STATUS_FAILURE;
    }

    return status;
}

ed247_status_t UdpSocket::Pool::wait_during(int32_t duration_us)
{
    PRINT_CRAZY("Socket pool waiting during [" << duration_us << "] us");

    ed247_status_t status = ED247_STATUS_NODATA;
    ed247_status_t tmp_status = status;
    int64_t        begin_us = get_monotonic_time_us();
    int64_t        remaining_us = duration_us;

    do {
        tmp_status = wait_frame(remaining_us);
        if(tmp_status == ED247_STATUS_SUCCESS){
            status = ED247_STATUS_SUCCESS;
        }else if(tmp_status == ED247_STATUS_FAILURE){
            status = ED247_STATUS_FAILURE;
        }
        remaining_us = duration_us - (get_monotonic_time_us() - begin_us);
    }while(remaining_us > 0 && status != ED247_STATUS_FAILURE);

    return status;
}

}
