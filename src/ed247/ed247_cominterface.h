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
#ifndef _ED247_COMINTERFACE_H_
#define _ED247_COMINTERFACE_H_
#include "ed247_xml.h"
#include "ed247_friend_test.h"
#include <functional>

// Networking
#ifdef __unix__
# include <sys/socket.h>
# include <netinet/in.h>
# include <sys/select.h>
using ed247_socket_t = int;
# define INVALID_SOCKET (-1)
#elif _WIN32
# include <winsock2.h>
using ed247_socket_t = SOCKET;
#endif


//
// socket_address_t : store a network address (ip/port)
//
namespace ed247 {
  namespace udp {
    struct socket_address_t: public sockaddr_in
    {
      socket_address_t(std::string ip_address, uint16_t ip_port);
      void set_ip_address(std::string ip_address);
      void set_ip_port(uint16_t ip_port);
      bool is_multicast() const;
      bool is_unicast() const;
      bool is_any_addr() const;
    };
  }
}
std::ostream& operator<<(std::ostream & os, const ed247::udp::socket_address_t& socket_address);



namespace ed247 {
  namespace udp {

    //
    // Transceiver (aka ECIC UdpSocket)
    // Hold a system socket and prepare it for transceiving.
    // base class for emitter and receiver.
    //
    class Transceiver {
    public:
      Transceiver(const socket_address_t& socket_address);
      ~Transceiver();

      Transceiver(const Transceiver&) = delete;
      Transceiver& operator=(const Transceiver&) = delete;

      const ed247_socket_t& get_socket() const { return _socket; }

    protected:
      socket_address_t _socket_address;           // Where the packets come from (regardless direction)
      ed247_socket_t   _socket{INVALID_SOCKET};   // Where the packets come from (regardless direction)
    };

    class Emitter : public Transceiver
    {
    public:
      Emitter(socket_address_t from_address, socket_address_t destination_address, uint16_t multicast_ttl = 1);
      void send_frame(const void* payload, const uint32_t payload_size);

    private:
      socket_address_t _destination_address;
    };

    class Receiver : public Transceiver
    {
    public:
      using receive_callback_t = std::function<void(const char* payload, uint32_t size)>;

      Receiver(socket_address_t from_address,
               socket_address_t multicast_interface,
               socket_address_t multicast_group_address,
               receive_callback_t callback);
      void receive();

    private:
      static const uint32_t MAX_FRAME_SIZE{65508};
      struct frame_t
      {
        char     payload[MAX_FRAME_SIZE];
        uint32_t size{MAX_FRAME_SIZE};
      };

      receive_callback_t  _receive_callback;
      static frame_t      _receive_frame;    // static: all receive will share the same memory to prevent 65k alloc per receiver

      ED247_FRIEND_TEST();
    };

    //
    // ReceiverSet
    // Store receiver and allows to receive data on all of them.
    // There is only one of this class per context.
    //
    class ReceiverSet
    {
    public:
      ReceiverSet();
      ~ReceiverSet();

      ReceiverSet& operator=(const ReceiverSet &)  = delete;
      ReceiverSet& operator=(ReceiverSet &&)       = delete;

      // Add receiver and take onership
      void emplace(Receiver* receiver);

      // Receive frames from all registered receivers.
      // Call receive_callback(s) set by ComInterface::load()
      ed247_status_t wait_frame(int32_t timeout_us);
      ed247_status_t wait_during(int32_t duration_us);

    private:
      std::vector<std::unique_ptr<Receiver>> _receivers;

      struct select_options_s {
        fd_set fd;
        int nfds;
      } _select_options;

      ED247_FRIEND_TEST();
    };


    //
    // ComInterface
    // The ComInterface will load all its transceivers (aka ECIC UdpSokets) and :
    // - Store its emitters to allow to send_frame to all of them,
    // - Store its receivers in context_receiver_set to allow to receive on all context receivers.
    //
    class ComInterface
    {
    public:
      // Load configuration and
      // - store emmiters
      // - store receivers in context_receiver_set,
      // - set receive_callback on each of them.
      void load(const xml::ComInterface& configuration,
                ReceiverSet& context_receiver_set,
                Receiver::receive_callback_t receive_callback);

      // Send a frame to all ComInterface emitters
      void send_frame(const void* payload, const uint32_t payload_size);

      ComInterface();
      ~ComInterface();

    private:
      std::vector<std::unique_ptr<Emitter>> _emitters;
    };


    // throw an exception if not all socket are closed (debug purpose)
    void assert_sockets_closed();
  }
}

#endif
