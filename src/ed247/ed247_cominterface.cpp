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
#include "ed247_cominterface.h"
#include "ed247_time.h"
#include <unistd.h>
#include <fcntl.h>
#include <unordered_map>
#include <unordered_set>

// Networking
#ifdef __unix__
# include <arpa/inet.h>
const auto& ed247_close_socket = close;
#elif _WIN32
# include <Ws2tcpip.h>
const auto& ed247_close_socket = closesocket;
#endif

namespace
{

//
// Constant for setsockopt
//
#ifndef _MSC_VER
  static const __attribute__((__unused__)) int zero = 0;
  static const __attribute__((__unused__)) int one = 1;
#else
  int zero = 0;
  int one = 1;
#endif

  //
  // system error message
  //
  std::string ed247_get_system_error()
  {
#ifdef _WIN32
    LPSTR messageBuffer = nullptr;
    DWORD dwError = WSAGetLastError();
    uint32_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                   NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    char errmsg[1024];
    strerror_s(errmsg, 1024, errno);
    std::string err = std::string(messageBuffer, size);
    LocalFree(messageBuffer);
    return err + " | " + std::string(errmsg);
#else
    return std::string(strerror(errno));
#endif
  }

#define THROW_SOCKET_ERROR(address, m)                                  \
  THROW_ED247_ERROR("[" << address << "] " << m << " (" << ed247_get_system_error() << ")")

}


//
// Handle system sockets
// Store sockets to prevent bind several times the same address (output)
// Same addresses may be used in several ECIC. So this class is global (not context scope)
//
namespace ed247 {
  namespace udp {

#define SYSTEM_SOCKET_ASSERT(test, address, socket, m)                  \
    if ((test) == false) {                                              \
      ed247_close_socket(socket);                                       \
      THROW_ED247_ERROR("[" << address << "] " << m << " (" << ed247_get_system_error() << ")"); \
    }

    struct socket_address_hash {
      size_t operator()(const socket_address_t& socket_address) const {
        return std::hash<uint32_t>()(socket_address.sin_addr.s_addr) ^ std::hash<uint16_t>()(socket_address.sin_port);
      }
    };

    struct socket_address_equal_to {
      bool operator()(const socket_address_t& a, const socket_address_t& b) const {
        return a.sin_addr.s_addr == b.sin_addr.s_addr && a.sin_port == b.sin_port;
      }
    };

    struct system_socket_map_t {

      // Return an existing socket or create a new one
      ed247_socket_t create(const socket_address_t& address) {
        socket_map_t::iterator isock = _map.find(address);
        if (isock != _map.end()) {
          isock->second.count++;
          PRINT_DEBUG("[SOCKET] Reuse socket " << address << ". Use count: " << isock->second.count);
          return isock->second.socket;
        } else {
          int sockerr = 0;
          PRINT_DEBUG("[SOCKET] Create new socket " << address << ".");
          ed247_socket_t socket = ::socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
          SYSTEM_SOCKET_ASSERT(socket != INVALID_SOCKET, address, INVALID_SOCKET, "Failed to create the socket!");

          // Allow to reuse address already used by self or another sofware, might have stange behaviors...
          sockerr = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&one, sizeof(one));
          SYSTEM_SOCKET_ASSERT(sockerr == 0, address, socket, "Failed to set socket options SO_REUSEADDR.");

          // Set to non blocking
#ifdef _WIN32
          unsigned long ul_one = 1;
          sockerr = ioctlsocket(socket, FIONBIO, (unsigned long *)&ul_one);
#else
          int flags = fcntl(socket, F_GETFL, 0);
          sockerr = fcntl(socket, F_SETFL, flags | O_NONBLOCK);
#endif
          SYSTEM_SOCKET_ASSERT(sockerr == 0, address, socket, "Failed to set socket to non-blocking.");

          sockerr = bind(socket, (struct sockaddr *)&address, sizeof(struct sockaddr_in));
          SYSTEM_SOCKET_ASSERT(sockerr == 0, address, socket, "Failed to bind socket");

          _map.insert(socket_map_t::value_type(address, socket_container_t(socket)));
          return socket;
        }
      }

      // Free socket
      void release(const socket_address_t& address) {
        socket_map_t::iterator isock = _map.find(address);
        if (isock != _map.end()) {
          if (--isock->second.count == 0) {
            PRINT_DEBUG("[SOCKET] Close socket " << address << ".");
            ed247_close_socket(isock->second.socket);
            _map.erase(isock);
          } else {
            PRINT_DEBUG("[SOCKET] Socket still used " << address << ". Use count: " << isock->second.count);
          }
        } else {
          PRINT_DEBUG("[SOCKET] Try to close an non-existing socket " << address << ".");
        }
      }

      // Check all sockets are freed (for debug purpose)
      void assert_sockets_freed() {
        if (_map.empty()) {
          SAY("[SOCKET] All are freed");
        } else {
          for(auto& socket : _map) {
            SAY("[SOCKET] - Still in use: " << socket.first);
          }
          THROW_ED247_ERROR("[SOCKET] Some socket still in use");
        }
      }


    private:
      struct socket_container_t {
        socket_container_t(ed247_socket_t sock) : socket(sock), count(1) {}
        ed247_socket_t socket;
        uint32_t       count;
      };

      using socket_map_t = std::unordered_map<socket_address_t, socket_container_t,
                                              socket_address_hash, socket_address_equal_to>;
      socket_map_t _map;

    };

    system_socket_map_t system_socket_map;

    void assert_sockets_closed() { system_socket_map.assert_sockets_freed(); }

    // Might be used to check destination unicity. Not yet implemented.
    using address_set_t = std::unordered_set<socket_address_t,
                                             socket_address_hash, socket_address_equal_to>;
  }
}

//
// ComInterface
//
void ed247::udp::ComInterface::load(const xml::ComInterface& configuration,
                                    receiver_set_t& context_receiver_set,
                                    receiver::receive_callback_t receive_callback)
{
#ifdef _WIN32
  static bool winsocks_initialized = false;
  if (winsocks_initialized == false) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    winsocks_initialized = true;
  }
#endif

  for(const xml::UdpSocket& socket_configuration : configuration._udp_sockets)
  {
    socket_address_t destination_address(socket_configuration._dst_ip_address, socket_configuration._dst_ip_port);

    switch (socket_configuration._direction)
    {
    case ED247_DIRECTION_IN:
    {
      socket_address_t from_address(destination_address);
      // In multicast, we bind to the provided mc_ip_address to specify interface or to INADDR_ANY.
      // The destination is the ulticast group group to join.
      if (destination_address.is_multicast()) from_address.set_ip_address(socket_configuration._mc_ip_address);
      context_receiver_set.emplace(new receiver(from_address, destination_address /*mcast group*/, receive_callback));
      break;
    }

    case ED247_DIRECTION_OUT:
    {
      socket_address_t from_address(socket_configuration._src_ip_address, socket_configuration._src_ip_port);
      // Favor src address, even in multicast (reviewer: why ?)
      if (from_address.is_any_addr() && destination_address.is_multicast())
        from_address.set_ip_address(socket_configuration._mc_ip_address);
      _emitters.emplace_back(new emitter(from_address, destination_address, socket_configuration._mc_ttl));
      break;
    }

    default:
      THROW_ED247_ERROR("Bidirectional UdpSocket is not supported.");
    }
  }
}

void ed247::udp::ComInterface::send_frame(const void* payload, const uint32_t payload_size)
{
  for(auto emitter = _emitters.begin() ; emitter != _emitters.end(); emitter++) {
    (*emitter)->send_frame(payload, payload_size);
  }
}


//
// transceiver
//
ed247::udp::transceiver::transceiver(const socket_address_t& socket_address) :
  _socket_address(socket_address)
{
  MEMCHECK_NEW(this, "transceiver " << _socket_address);
  _socket = system_socket_map.create(_socket_address);
}

ed247::udp::transceiver::~transceiver() {
  MEMCHECK_DEL(this, "transceiver " << _socket_address);
  system_socket_map.release(_socket_address);
}

//
// emitter
//
ed247::udp::emitter::emitter(socket_address_t from_address, socket_address_t destination_address, uint16_t multicast_ttl) :
  transceiver(from_address),
  _destination_address(destination_address)
{
  if(_destination_address.is_multicast()) {
    int sockerr = 0;

    // Set outgoing interface
    if (from_address.is_any_addr() == false) {
      sockerr = setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_IF, (const char*) &from_address.sin_addr, sizeof(struct in_addr));
      if (sockerr) THROW_SOCKET_ERROR(_socket_address, "Failed to set the multicast outgoing interface.");
    }

    // Set multicast packet TTL
    sockerr = setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_TTL, (const char*) &multicast_ttl, sizeof(multicast_ttl));
    if (sockerr) THROW_SOCKET_ERROR(_socket_address, "Failed to set the multicast TTL.");

    // Send multicast traffic to myself too
    sockerr = setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_LOOP, (const char*) &one, sizeof(one));
    if (sockerr) THROW_SOCKET_ERROR(_socket_address, "Failed to enable the multicast loop.");
  }
}


void ed247::udp::emitter::send_frame(const void* payload, const uint32_t payload_size)
{
  PRINT_CRAZY("sendto() from (" << _socket_address << ") to (" << _destination_address << "), size " << payload_size << "b [" << hex_stream(payload, payload_size) << "]");
  int32_t sent_size = sendto(_socket, (const char *)payload, payload_size, 0, (struct sockaddr *)&_destination_address, sizeof(struct sockaddr_in));
  if(sent_size < 0 || (uint32_t)sent_size != payload_size) {
    PRINT_ERROR("Failed to send frame from socket socket [" << _socket_address << "] to [" << _destination_address << "] (" << ed247_get_system_error() << ")");
  }
}

//
// receiver
//
ed247::udp::receiver::frame_t ed247::udp::receiver::_receive_frame;

ed247::udp::receiver::receiver(socket_address_t from_address, socket_address_t multicast_group_address, receive_callback_t callback) :
  transceiver(from_address),
  _receive_callback(callback)
{
  if (multicast_group_address.is_multicast()) {
    int sockerr = 0;

    struct ip_mreq imreq;
    memset(&imreq, 0, sizeof(struct ip_mreq));
    imreq.imr_interface.s_addr = from_address.sin_addr.s_addr;
    imreq.imr_multiaddr.s_addr = multicast_group_address.sin_addr.s_addr;
    sockerr = setsockopt(_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&imreq, sizeof(struct ip_mreq));
    if (sockerr) THROW_SOCKET_ERROR(_socket_address, "Failed to join the multicast group " << multicast_group_address << ".");
  }
}

void ed247::udp::receiver::receive()
{
  int recv_result = 0;
  bool frame_received = false;
  do {
    recv_result = ::recvfrom(_socket, _receive_frame.payload, MAX_FRAME_SIZE, 0, nullptr, 0);
    if(recv_result <= 0) break;
    frame_received = true;

    _receive_frame.size = recv_result;
    PRINT_CRAZY("Received frame of " << _receive_frame.size << " bytes: [" << hex_stream(_receive_frame.payload, _receive_frame.size) << "]");
    _receive_callback(_receive_frame.payload, _receive_frame.size);
  } while(recv_result > 0);

  if(frame_received == false && recv_result <= 0) {
    PRINT_ERROR("recvfrom() failed  on socket " << _socket_address << ". " << ed247_get_system_error());
  }
}

//
// receiver_set_t
//
ed247::udp::receiver_set_t::receiver_set_t()
{
  MEMCHECK_NEW(this, "udp::receiver_set_t");
  FD_ZERO(&_select_options.fd);
  _select_options.nfds = 0;
}

ed247::udp::receiver_set_t::~receiver_set_t()
{
  MEMCHECK_DEL(this, "udp::receiver_set_t");
}

void ed247::udp::receiver_set_t::emplace(receiver* receiver)
{
  ed247_socket_t socket = receiver->get_socket();
  _select_options.nfds = (std::max)((int)(socket+1), _select_options.nfds);
  FD_SET(socket, &_select_options.fd);

  _receivers.emplace_back(receiver);
}


ed247_status_t ed247::udp::receiver_set_t::wait_frame(int32_t timeout_us)
{
  PRINT_CRAZY("UDP: Waiting for first frame to be received");

  if (_select_options.nfds <= 0) {
    PRINT_DEBUG("wait_frame: No socket opened in reading (no input messages in ECIC ?)");
    // Select will fail on Windows without errors. Simulate the wait for nothing.
    sleep_us(timeout_us);
    return ED247_STATUS_TIMEOUT;
  };

  ed247_status_t  status = ED247_STATUS_TIMEOUT;
  struct ::timeval timeout;
  int select_status = 1;
  fd_set select_fd;

  if(timeout_us >= 0) {
    timeout.tv_sec = (uint32_t)timeout_us / 1000000;
    timeout.tv_usec = (uint32_t)timeout_us % 1000000;
  }

  do {
    memcpy(&select_fd, &_select_options.fd, sizeof(fd_set));
    select_status = select(_select_options.nfds, &select_fd, NULL, NULL, timeout_us >= 0 ? &timeout : NULL);
    PRINT_CRAZY("wait_frame select result: " << select_status << ", errno: " << strerror(errno));
    if(select_status > 0){
      // Something received, call the associated receiver
      PRINT_CRAZY("Data received !");
      for(auto & receiver : _receivers) {
        const auto & socket = receiver->get_socket();
        if(socket != INVALID_SOCKET && FD_ISSET(socket, &select_fd)){
          receiver->receive();
          status = ED247_STATUS_SUCCESS;
        }
      }
      // Discard timeout since something has been received
      timeout.tv_sec = 0;
      timeout.tv_usec = 0;
    }
  } while(select_status > 0 || (select_status < 0 && errno == EINTR));

  if(select_status < 0 && _select_options.nfds > 0){
    PRINT_ERROR("Failed to perform select() (" << select_status << ":" << ed247_get_system_error() << ")");
    status = ED247_STATUS_FAILURE;
  }

  return status;
}

ed247_status_t ed247::udp::receiver_set_t::wait_during(int32_t duration_us)
{
  PRINT_CRAZY("UDP: Waiting during [" << duration_us << "] us");

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
  } while(remaining_us > 0 && status != ED247_STATUS_FAILURE);

  return status;
}


//
// socket_address_t
//
ed247::udp::socket_address_t::socket_address_t(std::string ip_address, uint16_t ip_port)
{
  sin_family = PF_INET;
  set_ip_port(ip_port);
  set_ip_address(ip_address);
}

void ed247::udp::socket_address_t::set_ip_address(std::string ip_address)
{
  if(ip_address.empty()) {
    sin_addr.s_addr = htonl(INADDR_ANY);
  } else {
    sin_addr.s_addr = inet_addr(ip_address.c_str());
  }
}

void ed247::udp::socket_address_t::set_ip_port(uint16_t ip_port)
{
  sin_port = htons(ip_port);
}

bool ed247::udp::socket_address_t::is_multicast() const
{
  return IN_MULTICAST(ntohl(sin_addr.s_addr));
}

bool ed247::udp::socket_address_t::is_unicast() const
{
  return is_multicast() == false;
}

bool ed247::udp::socket_address_t::is_any_addr() const {
  return (sin_addr.s_addr == htonl(INADDR_ANY));
}

std::ostream & operator << (std::ostream & os, const ed247::udp::socket_address_t& socket_address)
{
  std::string ipaddr;
#ifdef _MSC_VER
  char straddr[INET_ADDRSTRLEN];
  static struct in_addr sin_addr_temp = socket_address.sin_addr;
  InetNtop(AF_INET, &sin_addr_temp, straddr, INET_ADDRSTRLEN);
  ipaddr = std::string(straddr);
#elif __unix__
  char straddr[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &socket_address.sin_addr, straddr, INET_ADDRSTRLEN);
  ipaddr = std::string(straddr);
#else
  ipaddr = std::string(inet_ntoa(socket_address.sin_addr));
#endif
  return os << ipaddr << ":" << ntohs(socket_address.sin_port);
}
