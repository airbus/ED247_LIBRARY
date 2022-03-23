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

#ifndef _SYNC_ENTITY_H_
#define _SYNC_ENTITY_H_

#include <inttypes.h>
#include <string>

#ifdef __unix__
#define SOCKET int
#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif
#endif
#ifdef _WIN32
#include <winsock2.h>
#define SOCKET SOCKET
#endif

#define SYNCER_ID_MASTER 1
#define SYNCER_ID_SLAVE 2

#define SYNC_TIMEOUT_US (10 * 1000 * 1000)

struct sync_entity_internal_t {};

namespace synchro
{

struct Entity;

const uint8_t       MAX_CLIENT_NUMBER{10};
const uint16_t      DEFAULT_PORT{3000};
const std::string   DEFAULT_IP{"127.0.0.1"};

uint64_t get_time_us();
void sleep_us(uint32_t duration_us);

struct SocketEntity
{
    // virtual ~SocketEntity() = 0;
    static void close(SOCKET & socket);
};

struct Server : public SocketEntity
{
    Server(Entity * entity);
    ~Server();

    bool wait(uint32_t eid, uint32_t timeout_us = SYNC_TIMEOUT_US);

private:
    void accept(uint32_t eid, uint32_t timeout_us = SYNC_TIMEOUT_US);

    Entity *_entity;
    SOCKET _socket{INVALID_SOCKET};
    SOCKET _clients[MAX_CLIENT_NUMBER]{INVALID_SOCKET};

};

struct Client  : public SocketEntity
{
    Client(Entity * entity);
    ~Client();

    void send(uint32_t eid, uint32_t timeout_us = SYNC_TIMEOUT_US);

private:
    void connect(uint32_t eid, uint32_t timeout_us = SYNC_TIMEOUT_US);

    Entity *_entity;
    SOCKET _servers[MAX_CLIENT_NUMBER]{INVALID_SOCKET};
};

struct Entity : public sync_entity_internal_t
{
    uint32_t id;
    Server server;
    Client client;

    static void init()
    {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    }

    explicit Entity(uint32_t eid);

    void send(uint32_t eid, uint32_t timeout_us = SYNC_TIMEOUT_US)
    {
        client.send(eid, timeout_us);
    }

    bool wait(uint32_t eid, uint32_t timeout_us = SYNC_TIMEOUT_US)
    {
        return server.wait(eid, timeout_us);
    }
};

}

void sync_init(uint32_t src_id);
void sync_wait(uint32_t remote_id);
void sync_send(uint32_t remote_id);
void sync_sync(uint32_t remote_id);
void sync_stop();

#endif
