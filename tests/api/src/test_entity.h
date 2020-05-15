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

#ifndef _TEST_ENTITY_H_
#define _TEST_ENTITY_H_

#include <inttypes.h>
#include <string>

#ifdef __linux
#define SOCKET int
#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif
#endif
#ifdef _WIN32
#include <winsock2.h>
#define SOCKET SOCKET
#endif

#define TESTER_ID_MASTER 1
#define TESTER_ID_SLAVE 2

#define TIMEOUT_US 45000000

struct test_entity_internal_t {};

namespace test
{

struct Entity;

const uint8_t       MAX_CLIENT_NUMBER{10};
const uint16_t      DEFAULT_PORT{3000};
const std::string   DEFAULT_IP{"127.0.0.1"};

std::string get_last_socket_error();
uint64_t get_time_us();
void sleep_us(uint32_t duration_us);
std::string get_env_variable(const std::string & variable);

/**
 * @brief The function counts the number of line where regex trace_to_find is found in the file pointed by filename.
 * @param[in] filename designates the file to parse.
 * @param[in] trace_to_find designates the regex to look for in each line of the file.
 * @return NULL if the file could not be openned, or a pointer that store the number of hit lines.
 * @note If an ill-formated regex is provided, the corresponding error will be thrown.
 * @note Regex not correctly implemented on gcc4.8.x and earlier (used for linux build)
 * @note Providing ".*" allows to count the number of lines in the provided file
**/
const uint32_t* count_matching_lines_in_file(const char* filename, const char* trace_to_find);


struct SocketEntity
{
    // virtual ~SocketEntity() = 0;
    static void close(SOCKET & socket);
};

struct Server : public SocketEntity
{
    Server(Entity * entity);
    ~Server();

    bool wait(uint32_t eid, uint32_t timeout_us = TIMEOUT_US);

private:
    void accept(uint32_t eid, uint32_t timeout_us = TIMEOUT_US);

    Entity *_entity;
    SOCKET _socket{INVALID_SOCKET};
    SOCKET _clients[MAX_CLIENT_NUMBER]{INVALID_SOCKET};

};

struct Client  : public SocketEntity
{
    Client(Entity * entity);
    ~Client();

    void send(uint32_t eid, uint32_t timeout_us = TIMEOUT_US);

private:
    void connect(uint32_t eid, uint32_t timeout_us = TIMEOUT_US);
    void done(uint32_t eid, uint32_t timeout_us = TIMEOUT_US);
    void kill();

    Entity *_entity;
    SOCKET _servers[MAX_CLIENT_NUMBER]{INVALID_SOCKET};
};

struct Entity : public test_entity_internal_t
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

    void send(uint32_t eid, uint32_t timeout_us = TIMEOUT_US)
    {
        client.send(eid, timeout_us);
    }

    bool wait(uint32_t eid, uint32_t timeout_us = TIMEOUT_US)
    {
        return server.wait(eid, timeout_us);
    }
};

}

void test_init(uint32_t src_id);
void test_wait(uint32_t remote_id);
void test_send(uint32_t remote_id);
void test_sync(uint32_t remote_id);
void test_stop();

#endif