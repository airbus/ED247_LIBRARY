/******************************************************************************
 * The MIT Licence
 *
 * Copyright (c) 2019 Airbus Operations S.A.S
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

/************
 * Includes *
 ************/

#include <ed247_test.h>
#include <ed247_logs.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ed247_memhooks.h>
#include <ed247_channel.h>

#include <memory>

/***********
 * Defines *
 ***********/

using namespace ed247;

/*************
 * Functions *
 *************/

class SocketContext : public ::testing::TestWithParam<xml::UdpSocket> {};

// Naming convention
// TEST_EMITTER_<num_frame_per_channel>_<num_channel>_<num_socket>
// _RECEPTION_<num_frame_per_channel>_<num_channel>_<num_socket>

TEST_P(SocketContext, TEST_EMITTER_1_1_1_RECEPTION_1_1_1)
{
    try{
        int idebug = 0;

        auto pool_sockets = std::make_shared<UdpSocket::Pool>();
        auto pool_streams = std::make_shared<BaseStream::Pool>();
        auto pool_interfaces = std::dynamic_pointer_cast<ComInterface::Pool>(pool_sockets);
        auto pool_channels = std::make_shared<Channel::Pool>(
                pool_interfaces,
                pool_streams);

        // Socket
        auto sp_socket_conf = std::make_shared<xml::UdpSocket>(GetParam());

        std::ostringstream oss;
        oss << "Send a single frame from a single channel with one socket to another channel with another socket. [" << sp_socket_conf->toString() << "]";
        RecordProperty("description",oss.str());

        // Stream (Emitter)
        auto sp_stream_emitter_conf = std::make_shared<xml::A429Stream>();
        sp_stream_emitter_conf->reset();
        sp_stream_emitter_conf->info.name = "Stream";
        sp_stream_emitter_conf->info.direction = ED247_DIRECTION_OUT;
        sp_stream_emitter_conf->info.uid = 0;

        // Channel (Emitter)
        auto sp_channel_emitter_conf = std::make_shared<xml::Channel>();
        sp_channel_emitter_conf->info.name = "ChannelOutput";
        sp_channel_emitter_conf->com_interface.udp_sockets.push_back(sp_socket_conf);
        sp_channel_emitter_conf->streams.push_back(sp_stream_emitter_conf);
        auto channel_emitter = pool_channels->get(sp_channel_emitter_conf);

        // Stream (Receiver)
        auto sp_stream_receiver_conf = std::make_shared<xml::A429Stream>();
        sp_stream_receiver_conf->reset();
        sp_stream_receiver_conf->info.name = "Stream";
        sp_stream_receiver_conf->info.direction = ED247_DIRECTION_IN;
        sp_stream_receiver_conf->info.uid = 0;

        // Channel (Receiver)
        auto sp_channel_receiver_conf = std::make_shared<xml::Channel>();
        sp_channel_receiver_conf->info.name = "ChannelInput";
        sp_channel_receiver_conf->com_interface.udp_sockets.push_back(sp_socket_conf);
        sp_channel_receiver_conf->streams.push_back(sp_stream_receiver_conf);
        auto channel_receiver = pool_channels->get(sp_channel_receiver_conf);

        // Prepare frame to send
        std::string send_msg{"Hell"};
        size_t msg_size = sizeof(ed247_uid_t)+sizeof(uint16_t)+4;
        char *msg = (char *)malloc(msg_size);
        *(ed247_uid_t*)(msg) = htons(sp_stream_emitter_conf->info.uid);
        *(uint16_t*)(msg+sizeof(ed247_uid_t)) = htons(4);
        memcpy(msg+sizeof(ed247_uid_t)+sizeof(uint16_t),send_msg.c_str(),send_msg.length());

        // Prepare for reception
        const char *recv_frame;
        size_t recv_frame_size;

        LOG_INFO() << "### " << idebug++ << LOG_END;

        memhooks_section_start();

        // Send frame
        channel_emitter->get_emitters().front().lock()->send_frame(*channel_emitter, (const void *)msg, msg_size);
        // Recv frame
        ASSERT_EQ(pool_sockets->wait_frame(10000), ED247_STATUS_SUCCESS);

        ASSERT_TRUE(memhooks_section_stop());

        LOG_INFO() << "### " << idebug++ << LOG_END;

        // Retrieve message
        std::dynamic_pointer_cast<UdpSocket>(channel_receiver->get_receivers().front().lock())->get_recv_frame(recv_frame, recv_frame_size);
        std::string recv_msg{recv_frame+sizeof(ed247_uid_t)+sizeof(uint16_t), recv_frame_size-sizeof(ed247_uid_t)-sizeof(uint16_t)};
        ASSERT_EQ(send_msg,recv_msg);

        LOG_INFO() << "### " << idebug++ << LOG_END;

        memhooks_section_start();

        // Recv frame : timeout expected
        ASSERT_EQ(pool_sockets->wait_frame(10000), ED247_STATUS_TIMEOUT);

        // Send frame
        channel_emitter->get_emitters().front().lock()->send_frame(*channel_emitter, (const void *)msg, msg_size);
        // Recv frame
        ASSERT_EQ(pool_sockets->wait_during(10000), ED247_STATUS_SUCCESS);
        
        ASSERT_TRUE(memhooks_section_stop());

        LOG_INFO() << "### " << idebug++ << LOG_END;

        // Retrieve message
        std::dynamic_pointer_cast<UdpSocket>(channel_receiver->get_receivers().front().lock())->get_recv_frame(recv_frame, recv_frame_size);
        recv_msg = std::string{recv_frame+sizeof(ed247_uid_t)+sizeof(uint16_t), recv_frame_size-sizeof(ed247_uid_t)-sizeof(uint16_t)};
        ASSERT_EQ(send_msg,recv_msg);

        LOG_INFO() << "### " << idebug++ << LOG_END;

        memhooks_section_start();

        // Recv frame : timeout expected
        ASSERT_EQ(pool_sockets->wait_frame(1000), ED247_STATUS_TIMEOUT);

        ASSERT_TRUE(memhooks_section_stop());

        LOG_INFO() << "### " << idebug++ << LOG_END;

        // END
        free(msg);
    }
    catch(std::exception & e)
    {
        LOG_INFO() << "Failure: " << e.what() << LOG_END;
        ASSERT_TRUE(false);
    }
    catch(...)
    {
        LOG_INFO() << "Failure" << LOG_END;
        ASSERT_TRUE(false);
    }

    LOG_INFO() << "END" << LOG_END;
}

TEST_P(SocketContext, TEST_EMITTER_2_1_1_RECEPTION_2_1_1)
{
    try{
        int idebug = 0;

        auto pool_sockets = std::make_shared<UdpSocket::Pool>();
        auto pool_streams = std::make_shared<BaseStream::Pool>();
        auto pool_interfaces = std::dynamic_pointer_cast<ComInterface::Pool>(pool_sockets);
        auto pool_channels = std::make_shared<Channel::Pool>(
                pool_interfaces,
                pool_streams);

        // Socket
        auto sp_socket_conf = std::make_shared<xml::UdpSocket>(GetParam());

        std::ostringstream oss;
        oss << "Send two frames from a single channel with one socket to another channel with another socket. [" << sp_socket_conf->toString() << "]";
        RecordProperty("description",oss.str());

        // Stream (Emitter)
        auto sp_stream_emitter_conf = std::make_shared<xml::A429Stream>();
        sp_stream_emitter_conf->reset();
        sp_stream_emitter_conf->info.name = "Stream";
        sp_stream_emitter_conf->info.direction = ED247_DIRECTION_OUT;
        sp_stream_emitter_conf->info.uid = 0;

        // Channel (Emitter)
        auto sp_channel_emitter_conf = std::make_shared<xml::Channel>();
        sp_channel_emitter_conf->info.name = "ChannelOutput";
        sp_channel_emitter_conf->com_interface.udp_sockets.push_back(sp_socket_conf);
        sp_channel_emitter_conf->streams.push_back(sp_stream_emitter_conf);
        auto channel_emitter = pool_channels->get(sp_channel_emitter_conf);

        // Stream (Receiver)
        auto sp_stream_receiver_conf = std::make_shared<xml::A429Stream>();
        sp_stream_receiver_conf->reset();
        sp_stream_receiver_conf->info.name = "Stream";
        sp_stream_receiver_conf->info.direction = ED247_DIRECTION_IN;
        sp_stream_receiver_conf->info.uid = 0;

        // Channel (Receiver)
        auto sp_channel_receiver_conf = std::make_shared<xml::Channel>();
        sp_channel_receiver_conf->info.name = "ChannelInput";
        sp_channel_receiver_conf->com_interface.udp_sockets.push_back(sp_socket_conf);
        sp_channel_receiver_conf->streams.push_back(sp_stream_receiver_conf);
        auto channel_receiver = pool_channels->get(sp_channel_receiver_conf);

        // Prepare frame to send
        size_t msg_size = sizeof(ed247_uid_t)+sizeof(uint16_t)+4;
        std::string send_msg_a{"HelA"};
        char *msg_a = (char *)malloc(msg_size);
        *(ed247_uid_t*)(msg_a) = htons(sp_stream_emitter_conf->info.uid);
        *(uint16_t*)(msg_a+sizeof(ed247_uid_t)) = htons(4);
        memcpy(msg_a+sizeof(ed247_uid_t)+sizeof(uint16_t),send_msg_a.c_str(),send_msg_a.length());
        std::string send_msg_b{"HelB"};
        char *msg_b = (char *)malloc(msg_size);
        *(ed247_uid_t*)(msg_b) = htons(sp_stream_emitter_conf->info.uid);
        *(uint16_t*)(msg_b+sizeof(ed247_uid_t)) = htons(4);
        memcpy(msg_b+sizeof(ed247_uid_t)+sizeof(uint16_t),send_msg_b.c_str(),send_msg_b.length());

        // Prepare for reception
        const char *recv_frame;
        size_t recv_frame_size;

        LOG_INFO() << "### " << idebug++ << LOG_END;

        memhooks_section_start();

        // Send frame
        channel_emitter->get_emitters().front().lock()->send_frame(*channel_emitter, (const void *)msg_a, msg_size);
        channel_emitter->get_emitters().front().lock()->send_frame(*channel_emitter, (const void *)msg_b, msg_size);
        // Recv frame
        ASSERT_EQ(pool_sockets->wait_frame(1000), ED247_STATUS_SUCCESS);
        
        ASSERT_TRUE(memhooks_section_stop());

        // LOG_INFO() << "### " << idebug++ << LOG_END;

        // // Retrieve message
        // std::dynamic_pointer_cast<UdpSocket>(channel_receiver->get_receivers().front().lock())->get_recv_frame(recv_frame, recv_frame_size);
        // std::string recv_msg{recv_frame+sizeof(ed247_uid_t)+sizeof(uint16_t), recv_frame_size-sizeof(ed247_uid_t)-sizeof(uint16_t)};
        // ASSERT_EQ(send_msg_a,recv_msg);

        // LOG_INFO() << "### " << idebug++ << LOG_END;

        // memhooks_section_start();

        // // Recv frame
        // ASSERT_EQ(pool_sockets->wait_frame(1000), ED247_STATUS_SUCCESS);
        
        // ASSERT_TRUE(memhooks_section_stop());

        LOG_INFO() << "### " << idebug++ << LOG_END;

        // Retrieve message
        std::dynamic_pointer_cast<UdpSocket>(channel_receiver->get_receivers().front().lock())->get_recv_frame(recv_frame, recv_frame_size);
        std::string recv_msg = std::string{recv_frame+sizeof(ed247_uid_t)+sizeof(uint16_t), recv_frame_size-sizeof(ed247_uid_t)-sizeof(uint16_t)};
        ASSERT_EQ(send_msg_b,recv_msg);

        LOG_INFO() << "### " << idebug++ << LOG_END;

        memhooks_section_start();

        // Recv frame : timeout expected
        ASSERT_EQ(pool_sockets->wait_frame(1000), ED247_STATUS_TIMEOUT);

        ASSERT_TRUE(memhooks_section_stop());

        LOG_INFO() << "### " << idebug++ << LOG_END;

        // END
        free(msg_a);
        free(msg_b);
    }
    catch(std::exception & e)
    {
        LOG_INFO() << "Failure: " << e.what() << LOG_END;
        ASSERT_TRUE(false);
    }
    catch(...)
    {
        LOG_INFO() << "Failure" << LOG_END;
        ASSERT_TRUE(false);
    }

    LOG_INFO() << "END" << LOG_END;
}

std::vector<xml::UdpSocket> sockets_unicast = {
    xml::UdpSocket("127.0.0.1",2600,"127.0.0.1",2500),
    xml::UdpSocket("127.0.0.1",2600)
};

std::vector<xml::UdpSocket> sockets_multicast = {
    xml::UdpSocket("226.23.12.4",6000,"",5000)
};

INSTANTIATE_TEST_CASE_P(UnicastTests, SocketContext,
    ::testing::ValuesIn(sockets_unicast));

INSTANTIATE_TEST_CASE_P(MulticastTests,SocketContext,
    ::testing::ValuesIn(sockets_multicast));

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}