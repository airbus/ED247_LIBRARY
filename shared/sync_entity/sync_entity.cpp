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

#include "sync_entity.h"

#include <utility>
#include <memory>
#include <cstring>
#include <string>

#ifdef __unix__
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
// If system do not support CLOCK_MONOTONIC_RAW, fallback to CLOCK_MONOTONIC
# ifndef CLOCK_MONOTONIC_RAW
#  define CLOCK_MONOTONIC_RAW CLOCK_MONOTONIC
# endif
#endif
#ifndef _MSC_VER
#include <unistd.h>
#include <sys/time.h>
#endif
#ifdef _WIN32
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <mswsock.h>
#endif

#include <fstream>
#include <stdexcept>
#include <regex>
#include "ed247_logs.h"

#ifndef _MSC_VER
static const __attribute__((__unused__)) int zero = 0;
static const __attribute__((__unused__)) int one = 1;
#else
int zero = 0;
int one = 1;
#endif


namespace synchro
{
std::string get_last_socket_error()
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

#ifdef _MSC_VER
int gettimeofday(struct timeval * tp, struct timezone * tzp)
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

uint64_t get_time_us()
{
#ifdef __unix__
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
    return ((uint64_t)tp.tv_sec) * 1000000LL + ((uint64_t)tp.tv_nsec) / 1000LL;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000LL + (uint64_t)tv.tv_usec;
#endif
}

void sleep_us(uint32_t duration_us)
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

std::string get_env_variable(const std::string & variable)
{
    char * value;
#ifdef _MSC_VER
    uint32_t len;
    _dupenv_s(&value, &len, variable.c_str());
#else
    value = getenv(variable.c_str());
#endif
    return value != NULL ? std::string(value) : std::string();
}

// SocketEntity

void SocketEntity::close(SOCKET & socket)
{
    if(socket != INVALID_SOCKET){
        shutdown(socket, 2);
#ifdef __unix__
        ::close(socket);
#elif _WIN32
        closesocket(socket);
#endif
        socket = INVALID_SOCKET;
    }
}

// Server

Server::Server(Entity * entity):
    _entity(entity)
{
    int sockerr;

    for(int i = 0 ; i < MAX_CLIENT_NUMBER ; i++)
        _clients[i] = INVALID_SOCKET;

    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if(_socket == INVALID_SOCKET)
        THROW_ED247_ERROR("Failed to create server socket of sync entity [" << _entity->id << "]");
    
    sockerr = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&one, sizeof(one));
    if(sockerr)
        THROW_ED247_ERROR("Failed to set server socket option SOL_SOCKET of sync entity [" << _entity->id << "] (" << sockerr << ":" << get_last_socket_error() << ")");

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(DEFAULT_PORT + _entity->id);
    
    SAY("SYNC ENTITY " << _entity->id << ": Bind server on port " << (uint32_t)(DEFAULT_PORT + _entity->id));

    sockerr = bind(_socket, (struct sockaddr *) &addr, sizeof(addr));
    if(sockerr)
        THROW_ED247_ERROR("Failed to bind server socket of sync entity [" << _entity->id << "] (" << sockerr << ":" << get_last_socket_error() << ")");

    sockerr = listen(_socket, MAX_CLIENT_NUMBER);
    if(!_socket)
        THROW_ED247_ERROR("Failed to listen by sync entity [" << _entity->id << "] (" << sockerr << ":" << get_last_socket_error() << ")");
}

Server::~Server()
{
    for(int i = 0 ; i < MAX_CLIENT_NUMBER ; i++){
        close(_clients[i]);
    }
    close(_socket);

}

bool Server::wait(uint32_t eid, uint32_t timeout_us)
{
    PRINT_DEBUG("SYNC ENTITY " << _entity->id << ": Waiting for " << eid << " ...");

    int sockerr;
    uint64_t begin_us = get_time_us();

    // If client is not connected, accept it
    if(_clients[eid] == INVALID_SOCKET)
        accept(eid,timeout_us);

    // Wait for data
    uint64_t elapsed_us = get_time_us() - begin_us;
    if (elapsed_us > timeout_us)
        THROW_ED247_ERROR("Test entity [" << _entity->id << "] server socket timeout waiting sync entity [" << eid << "]");

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = (long)(timeout_us - elapsed_us);

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(_clients[eid], &rfds);

    sockerr = ::select((int)_clients[eid] + 1, &rfds, NULL, NULL, &tv);
    if(sockerr == 0)
        THROW_ED247_ERROR("Test entity [" << _entity->id << "] timout waiting for sync entity [" << eid << "] (" << sockerr << ")");
    else if(sockerr <= 0)
        THROW_ED247_ERROR("Test entity [" << _entity->id << "] server socket select timeout waiting sync entity [" << eid << "] (" << sockerr << ")");

    // Receive data from remote sync entity
    uint32_t rid = 0;
    if((sockerr = ::recv(_clients[eid], (char*)&rid, 4, 0)) != 4)
        THROW_ED247_ERROR("Server receive error [" << sockerr << "]");

    if(rid == 0)
        THROW_ED247_ERROR("Server received id is null");

    if(rid != eid)
        THROW_ED247_ERROR("Test entity [" << _entity->id << "] server socket expect sync entity [" << eid << "] but has received [" << rid << "]");

    return true;
}

void Server::accept(uint32_t eid, uint32_t timeout_us)
{
    int sockerr;
    
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = timeout_us;

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(_socket, &rfds);

    // Wait for client
    sockerr = ::select((int)_socket + 1, &rfds, NULL, NULL, &tv);
    if(sockerr == 0)
        THROW_ED247_ERROR("Test entity [" << _entity->id << "] timout waiting for sync entity [" << eid << "] (" << sockerr << ")");
    else if(sockerr <= 0)
        THROW_ED247_ERROR("Failed to perform select on server socket of sync entity [" << _entity->id << "] (" << sockerr << ")");

    // Accept connection
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(sockaddr_in);
    _clients[eid] = ::accept(_socket, (struct sockaddr *) & addr, &addr_len);
    if(_clients[eid] == 0)
        THROW_ED247_ERROR("Test entity [" << _entity->id << "] timout accepting sync entity [" << eid << "] (" << sockerr << ")");
    else if(_clients[eid] <= 0)
        THROW_ED247_ERROR("Failed to accept connection on the server socket of sync entity [" << _entity->id << "] (" << _clients[eid] << ")");
}

// Client

Client::Client(Entity * entity):
    _entity(entity)
{
    for(int i = 0 ; i < MAX_CLIENT_NUMBER ; i++)
        _servers[i] = INVALID_SOCKET;
}

Client::~Client()
{
    for(int i = 0 ; i < MAX_CLIENT_NUMBER ; i++)
        close(_servers[i]);
}

void Client::send(uint32_t eid, uint32_t timeout_us)
{
    PRINT_DEBUG("SYNC ENTITY " << _entity->id << ": Signal to " << eid << " ...");
    int sockerr;

    if(_servers[eid] == INVALID_SOCKET)
        connect(eid, timeout_us);

    sockerr = ::send(_servers[eid], (const char*)&_entity->id, 4, 0);
    if(sockerr != 4)
        THROW_ED247_ERROR("Test entity [" << _entity->id << "] failed to send data to server of sync entity [" << eid << "] (" << sockerr << ")");
}

void Client::connect(uint32_t eid, uint32_t timeout_us)
{
    int sockerr = -1;
    uint64_t begin_us = get_time_us();

    _servers[eid] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(_servers[eid] == INVALID_SOCKET)
        THROW_ED247_ERROR("Failed to create client socket of sync entity [" << _entity->id << "]");

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    std::string ip_address = synchro::get_env_variable("ED247_SYNC_IP_ADDRESS");
    ip_address = ip_address.empty() ? DEFAULT_IP : ip_address;
#ifdef _MSC_VER
    inet_pton(AF_INET, ip_address.c_str(), &addr.sin_addr.s_addr);
#else
    addr.sin_addr.s_addr = inet_addr(ip_address.c_str());
#endif
    addr.sin_port = htons(DEFAULT_PORT + eid);

    while((get_time_us() - begin_us) < timeout_us &&
        (sockerr = ::connect(_servers[eid], (struct sockaddr *) &addr, sizeof(sockaddr_in))) != 0)
        sleep_us(1000);
    
    if(sockerr){
        close(_servers[eid]);
        THROW_ED247_ERROR("Test entity [" << _entity->id << "] failed to connect to sync entity [" << eid << "] (" << sockerr << ")");
    }
}

// Entity

Entity::Entity(uint32_t eid):
        id(eid),
        server(this),
        client(this)
{}

}

synchro::Entity *_syncer;

void sync_wait(uint32_t remote_id)
{
    SAY("###### SYNC WAIT ######");
    if(!_syncer->wait(remote_id))
        THROW_ED247_ERROR("Test entity [" << _syncer->id << "] failed to wait for [" << remote_id << "]");
}

void sync_send(uint32_t remote_id)
{
    SAY("###### SYNC SEND ######");
    _syncer->send(remote_id);
}

void sync_sync(uint32_t remote_id)
{
    SAY("###### SYNC SYNC ######");
    if(_syncer->id == SYNCER_ID_MASTER){
        sync_wait(remote_id); sync_send(remote_id);
    }else{
        sync_send(remote_id); sync_wait(remote_id);
    }
}

void sync_init(uint32_t src_id)
{
    SAY("###### SYNC INIT ######");
    synchro::Entity::init();
    _syncer = new synchro::Entity(src_id);
}

void sync_stop()
{
  SAY("###### SYNC STOP ######");
    delete _syncer;
}
