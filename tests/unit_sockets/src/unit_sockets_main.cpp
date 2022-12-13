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

// Set ED247_FRIEND_TEST to have access to class private members
#define TEST_CLASS_NAME(test_case_name, test_name) test_case_name##_##test_name##_Test
class TEST_CLASS_NAME(SocketContext, TEST_EMITTER_1_1_1_RECEPTION_1_1_1);
class TEST_CLASS_NAME(SocketContext, TEST_EMITTER_2_1_1_RECEPTION_2_1_1);
#define ED247_FRIEND_TEST()                                             \
  friend TEST_CLASS_NAME(SocketContext, TEST_EMITTER_1_1_1_RECEPTION_1_1_1); \
  friend TEST_CLASS_NAME(SocketContext, TEST_EMITTER_2_1_1_RECEPTION_2_1_1)

#include "unitary_test.h"
#include "ed247_channel.h"


class SocketContext : public ::testing::TestWithParam<ed247::xml::UdpSocket> {};

// Naming convention
// TEST_EMITTER_<num_frame_per_channel>_<num_channel>_<num_socket>
// _RECEPTION_<num_frame_per_channel>_<num_channel>_<num_socket>

TEST_P(SocketContext, TEST_EMITTER_1_1_1_RECEPTION_1_1_1)
{
  try{
    ed247::udp::ReceiverSet receiver_set;
    ed247::SignalSet signal_set;
    ed247::StreamSet stream_set(signal_set);
    ed247::ChannelSet channel_set(receiver_set, stream_set);

    // Socket
    ed247::xml::UdpSocket sp_socket_conf = GetParam();

    RecordProperty("description", strize() << "Send a single frame from a single channel with one socket to another channel with another socket. [" << sp_socket_conf << "]");

    // Stream (Emitter)
    ed247::xml::A429Stream* sp_stream_emitter_conf = new ed247::xml::A429Stream();
    sp_stream_emitter_conf->_name = "StreamOut";
    sp_stream_emitter_conf->_direction = ED247_DIRECTION_OUT;
    sp_stream_emitter_conf->_uid = 0;

    // Channel (Emitter)
    ed247::xml::Channel sp_channel_emitter_conf;
    sp_channel_emitter_conf._name = "ChannelOutput";
    sp_socket_conf._direction = ED247_DIRECTION_OUT;
    sp_channel_emitter_conf._com_interface._udp_sockets.push_back(sp_socket_conf);
    sp_channel_emitter_conf._stream_list.emplace_back(sp_stream_emitter_conf);
    auto channel_emitter = channel_set.create(&sp_channel_emitter_conf, 0);

    // Stream (Receiver)
    ed247::xml::A429Stream* sp_stream_receiver_conf = new ed247::xml::A429Stream();
    sp_stream_receiver_conf->_name = "StreamIn";
    sp_stream_receiver_conf->_direction = ED247_DIRECTION_IN;
    sp_stream_receiver_conf->_uid = 0;

    // Channel (Receiver)
    ed247::xml::Channel sp_channel_receiver_conf;
    sp_channel_receiver_conf._name = "ChannelInput";
    sp_socket_conf._direction = ED247_DIRECTION_IN;
    sp_channel_receiver_conf._com_interface._udp_sockets.push_back(sp_socket_conf);
    sp_channel_receiver_conf._stream_list.emplace_back(sp_stream_receiver_conf);
    auto channel_receiver = channel_set.create(&sp_channel_receiver_conf, 0);

    // Prepare frame to send
    std::string send_msg{"Hell"};
    uint32_t msg_size = sizeof(ed247_uid_t)+sizeof(uint16_t)+4;
    char *msg = (char *)malloc(msg_size);
    *(ed247_uid_t*)(msg) = htons(sp_stream_emitter_conf->_uid);
    *(uint16_t*)(msg+sizeof(ed247_uid_t)) = htons(4);
    memcpy(msg+sizeof(ed247_uid_t)+sizeof(uint16_t),send_msg.c_str(),send_msg.length());

    // Prepare for reception
    const char *recv_frame;
    uint32_t recv_frame_size;

    malloc_count_start();

    // Send frame
    channel_emitter->_com_interface.send_frame((const void *)msg, msg_size);
    // Recv frame
    ASSERT_EQ(receiver_set.wait_frame(10000), ED247_STATUS_SUCCESS);

    ASSERT_EQ(malloc_count_stop(), 0);

    // Retrieve message
    recv_frame = receiver_set._receivers.front()->_receive_frame.payload;
    recv_frame_size = receiver_set._receivers.front()->_receive_frame.size;
    std::string recv_msg{recv_frame+sizeof(ed247_uid_t)+sizeof(uint16_t), recv_frame_size-sizeof(ed247_uid_t)-sizeof(uint16_t)};
    ASSERT_EQ(send_msg,recv_msg);

    malloc_count_start();

    // Recv frame : timeout expected
    ASSERT_EQ(receiver_set.wait_frame(10000), ED247_STATUS_TIMEOUT);

    // Send frame
    channel_emitter->_com_interface.send_frame((const void *)msg, msg_size);
    // Recv frame
    ASSERT_EQ(receiver_set.wait_during(10000), ED247_STATUS_SUCCESS);

    ASSERT_EQ(malloc_count_stop(), 0);

    // Retrieve message
    recv_frame = receiver_set._receivers.front()->_receive_frame.payload;
    recv_frame_size = receiver_set._receivers.front()->_receive_frame.size;
    recv_msg = std::string{recv_frame+sizeof(ed247_uid_t)+sizeof(uint16_t), recv_frame_size-sizeof(ed247_uid_t)-sizeof(uint16_t)};
    ASSERT_EQ(send_msg,recv_msg);

    malloc_count_start();

    // Recv frame : timeout expected
    ASSERT_EQ(receiver_set.wait_frame(1000), ED247_STATUS_TIMEOUT);

    ASSERT_EQ(malloc_count_stop(), 0);

    // END
    free(msg);
  }
  catch(std::exception & e)
  {
    SAY("Failure: " << e.what());
    ASSERT_TRUE(false);
  }
  catch(...)
  {
    SAY("Failure");
    ASSERT_TRUE(false);
  }

  SAY("END");
}

TEST_P(SocketContext, TEST_EMITTER_2_1_1_RECEPTION_2_1_1)
{
  try{
    ed247::udp::ReceiverSet receiver_set;
    ed247::SignalSet signal_set;
    ed247::StreamSet stream_set(signal_set);
    ed247::ChannelSet channel_set(receiver_set, stream_set);

    // Socket
    ed247::xml::UdpSocket sp_socket_conf = GetParam();

    RecordProperty("description", strize() << "Send two frames from a single channel with one socket to another channel with another socket. [" << sp_socket_conf << "]");

    // Stream (Emitter)
    ed247::xml::A429Stream* sp_stream_emitter_conf = new ed247::xml::A429Stream();
    sp_stream_emitter_conf->_name = "StreamOut";
    sp_stream_emitter_conf->_direction = ED247_DIRECTION_OUT;
    sp_stream_emitter_conf->_uid = 0;

    // Channel (Emitter)
    ed247::xml::Channel sp_channel_emitter_conf;
    sp_channel_emitter_conf._name = "ChannelOutput";
    sp_socket_conf._direction = ED247_DIRECTION_OUT;
    sp_channel_emitter_conf._com_interface._udp_sockets.push_back(sp_socket_conf);
    sp_channel_emitter_conf._stream_list.emplace_back(sp_stream_emitter_conf);
    auto channel_emitter = channel_set.create(&sp_channel_emitter_conf, 0);

    // Stream (Receiver)
    auto sp_stream_receiver_conf = new ed247::xml::A429Stream();
    sp_stream_receiver_conf->_name = "StreamIn";
    sp_stream_receiver_conf->_direction = ED247_DIRECTION_IN;
    sp_stream_receiver_conf->_uid = 0;

    // Channel (Receiver)
    ed247::xml::Channel sp_channel_receiver_conf;
    sp_channel_receiver_conf._name = "ChannelInput";
    sp_socket_conf._direction = ED247_DIRECTION_IN;
    sp_channel_receiver_conf._com_interface._udp_sockets.push_back(sp_socket_conf);
    sp_channel_receiver_conf._stream_list.emplace_back(sp_stream_receiver_conf);
    auto channel_receiver = channel_set.create(&sp_channel_receiver_conf, 0);

      // Prepare frame to send
      uint32_t msg_size = sizeof(ed247_uid_t)+sizeof(uint16_t)+4;
      std::string send_msg_a{"HelA"};
      char *msg_a = (char *)malloc(msg_size);
      *(ed247_uid_t*)(msg_a) = htons(sp_stream_emitter_conf->_uid);
      *(uint16_t*)(msg_a+sizeof(ed247_uid_t)) = htons(4);
      memcpy(msg_a+sizeof(ed247_uid_t)+sizeof(uint16_t),send_msg_a.c_str(),send_msg_a.length());
      std::string send_msg_b{"HelB"};
      char *msg_b = (char *)malloc(msg_size);
      *(ed247_uid_t*)(msg_b) = htons(sp_stream_emitter_conf->_uid);
      *(uint16_t*)(msg_b+sizeof(ed247_uid_t)) = htons(4);
      memcpy(msg_b+sizeof(ed247_uid_t)+sizeof(uint16_t),send_msg_b.c_str(),send_msg_b.length());

      // Prepare for reception
      const char *recv_frame;
      uint32_t recv_frame_size;

      malloc_count_start();

      // Send frame
      channel_emitter->_com_interface.send_frame((const void *)msg_a, msg_size);
      channel_emitter->_com_interface.send_frame((const void *)msg_b, msg_size);
      // Recv frame
      ASSERT_EQ(receiver_set.wait_frame(1000), ED247_STATUS_SUCCESS);

      ASSERT_EQ(malloc_count_stop(), 0);

      // Retrieve message
      recv_frame = receiver_set._receivers.front()->_receive_frame.payload;
      recv_frame_size = receiver_set._receivers.front()->_receive_frame.size;
      std::string recv_msg = std::string{recv_frame+sizeof(ed247_uid_t)+sizeof(uint16_t), recv_frame_size-sizeof(ed247_uid_t)-sizeof(uint16_t)};
      ASSERT_EQ(send_msg_b,recv_msg);

      malloc_count_start();

      // Recv frame : timeout expected
      ASSERT_EQ(receiver_set.wait_frame(1000), ED247_STATUS_TIMEOUT);

      ASSERT_EQ(malloc_count_stop(), 0);

      // END
      free(msg_a);
      free(msg_b);
    }
    catch(std::exception & e)
    {
      SAY("Failure: " << e.what());
      ASSERT_TRUE(false);
    }
    catch(...)
    {
      SAY("Failure");
      ASSERT_TRUE(false);
    }

    SAY("END");
  }


  ed247::xml::UdpSocket create_udp_socket(std::string dst_ip_address, uint16_t dst_ip_port,
                                          std::string src_ip_address = std::string(), uint16_t src_ip_port = 0,
                                          std::string mc_ip_address = std::string(), uint16_t mc_ttl = 0,
                                          ed247_direction_t direction = ED247_DIRECTION_INOUT)
  {
    ed247::xml::UdpSocket socket;
    socket._dst_ip_address = dst_ip_address;
    socket._dst_ip_port = dst_ip_port;
    socket._src_ip_address = src_ip_address;
    socket._src_ip_port = src_ip_port;
    socket._mc_ip_address = mc_ip_address;
    socket._mc_ttl = mc_ttl;
    socket._direction = direction;

    return socket;
  }


  std::vector<ed247::xml::UdpSocket> sockets_unicast;
  std::vector<ed247::xml::UdpSocket> sockets_multicast;

  INSTANTIATE_TEST_CASE_P(UnicastTests, SocketContext,
                          ::testing::ValuesIn(sockets_unicast));

  INSTANTIATE_TEST_CASE_P(MulticastTests,SocketContext,
                          ::testing::ValuesIn(sockets_multicast));



int main(int argc, char **argv)
{
  std::string multicast_interface_ip;
  if(argc >= 3)
    multicast_interface_ip = argv[2];

  SAY("Multicast interface: " << multicast_interface_ip);

  sockets_unicast.push_back(create_udp_socket("127.0.0.1",2600,"127.0.0.1",2500));
  sockets_unicast.push_back(create_udp_socket("127.0.0.1",2600));
  sockets_multicast.push_back(create_udp_socket("224.1.1.1",6000,"",5000,multicast_interface_ip));

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
