#include "synchronizer.h"
#include <iostream>
#include <ostream>
#include <string.h>
#include <unordered_map>


// Delay between two send retry
#define SYNCHRO_SEND_RETRY_MS (uint32_t) 200


//
// Networking
//
#ifdef __unix__
# include <sys/socket.h>
# include <netinet/in.h>
# include <sys/select.h>
# include <arpa/inet.h>
# include <unistd.h>
using synchro_socket_t = int;
const auto& synchro_close_socket = close;
# define INVALID_SOCKET (-1)
#elif _WIN32
# include <winsock2.h>
using synchro_socket_t = SOCKET;
const auto& synchro_close_socket = closesocket;
#endif

struct socket_address_t: public sockaddr_in
{
  socket_address_t(const char* ip_address, uint16_t ip_port) {
    sin_family = PF_INET;
    sin_port = htons(ip_port);
    if (ip_address == nullptr || ip_address[0] == 0) {
      sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
      sin_addr.s_addr = inet_addr(ip_address);
    }
  }
};

std::ostream & operator<<(std::ostream& os, const socket_address_t& socket_address)
{
  std::string ipaddr;
#ifdef _MSC_VER
  char straddr[INET_ADDRSTRLEN];
  struct in_addr sin_addr_temp = socket_address.sin_addr;
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


//
// Logging
//
//#define SYNCHRO_DEBUG

#define LOG_SHORTFILE       ((caller_file)? caller_file : (strrchr("/" __FILE__, '/') + 1))
#define LOG_LINE            ((caller_line)? caller_line : __LINE__)
#define LOG_STREAM_FILELINE LOG_SHORTFILE << ":" << LOG_LINE << " "

#define LOG(m) std::cerr << LOG_STREAM_FILELINE << ": " << m << std::endl
#ifdef SYNCHRO_DEBUG
# define DEBUG(m) LOG(m)
#else
# define DEBUG(m)
#endif

#define ASSERT(test, m)                                                 \
  do {                                                                  \
    if ((test) == false) {                                              \
      LOG(m);                                                           \
      exit(1);                                                          \
    }                                                                   \
  } while (0)

#define ASSERT_SOCKET(test, address, socket, m)                         \
  do {                                                                  \
    if ((test) == false) {                                              \
      synchro_close_socket(socket);                                     \
      LOG("[" << address << "] " << m << " (" << synchro_get_system_error() << ")"); \
      exit(1);                                                          \
    }                                                                   \
  } while (0)

std::string synchro_get_system_error()
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

// methods synchro_signal and synchro_wait will override these
const char* caller_file = nullptr;
const uint32_t caller_line = 0;

//
// Time
//
#include <time.h>
uint64_t get_monotonic_time_us()
{
#ifdef __unix__
# ifndef CLOCK_MONOTONIC_RAW
#  define CLOCK_MONOTONIC_RAW CLOCK_MONOTONIC
# endif
  struct timespec tp;
  clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
  return ((uint64_t)tp.tv_sec) * 1000000LL + ((uint64_t)tp.tv_nsec) / 1000LL;
#else
  LARGE_INTEGER s_frequency;
  BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
  if (s_use_qpc) {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (1000000LL * now.QuadPart) / s_frequency.QuadPart;
  } else {
    return GetTickCount() * 1000LL;
  }
#endif
}


//
// Private data
//
namespace {
  // Map actor_id => next signal id to send to this actor
  using map_actor_signal_id_t = std::unordered_map<uint32_t, uint32_t>;

  uint32_t              _actor_id  = 0;
  synchro_socket_t      _socket    = INVALID_SOCKET;  // src of sendto and dest of recvfrom
  const char*           _host_ip   = nullptr;
  uint16_t              _base_port = 0;               // actor port = _base_port + _actor_id
  fd_set                _fd_set;                      // Will only contain _socket
  int                   _nfds;
  map_actor_signal_id_t _actor_send_signal_id;
  map_actor_signal_id_t _actor_recv_signal_id;

  struct __attribute__((__packed__)) payload_t
  {
    uint32_t from_actor_id;
    uint32_t signal_id;
  };

}



//
// Init
//
void synchro_init(uint32_t actor_id, const char* host_ip, uint16_t base_port)
{
  // Allow several call of init
  if (_socket != INVALID_SOCKET) return;

#ifdef _WIN32
  {
    // Be sure to initialize win32 socket API
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
  }
#endif

  int sockerr = 0;

  _actor_id = actor_id;
  _host_ip = host_ip;
  _base_port = base_port;

  // Create and bind socket (src of sendto and dest of recvfrom)
  socket_address_t address(host_ip, base_port + actor_id);
  DEBUG("Create a socket and bind it to " << address);

  _socket = ::socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
  ASSERT_SOCKET(_socket != INVALID_SOCKET, address, INVALID_SOCKET, "Failed to create the socket!");

  sockerr = bind(_socket, (struct sockaddr *)&address, sizeof(struct sockaddr_in));
  ASSERT_SOCKET(sockerr == 0, address, _socket, "Failed to bind socket");

  // Data to wait with timeout (select)
  FD_ZERO(&_fd_set);
  FD_SET(_socket, &_fd_set);
  _nfds = _socket + 1;


#ifdef _WIN32
  {
    // On windows, if the target of a sendto is not bounded, we will receive an ICMP error
    // 10054 - "Connection reset by peer". This error will be saved by the system and
    // raised on the next recvfrom() or select() on the same socket.
    // https://copyprogramming.com/howto/windows-udp-sockets-recvfrom-fails-with-error-10054
    // We disable this behavior.
    BOOL bNewBehavior = FALSE;
    DWORD dwBytesReturned = 0;
    WSAIoctl(_socket, _WSAIOW(IOC_VENDOR, 12), &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);
  }
#endif
}


//
// Send signal and wait ack
//
void synchro_signal(uint32_t target_actor_id, const char* user_message, const char* caller_file, uint32_t caller_line)
{
  // Where to send signal
  socket_address_t destination_address(_host_ip, _base_port + target_actor_id);

  // Signal data
  uint32_t signal_id = _actor_send_signal_id[target_actor_id]++;
  payload_t sig_payload = { _actor_id, signal_id };

  // Acknowledge wait data
  int select_status = -1;
  struct ::timeval timeout;
  timeout.tv_sec = SYNCHRO_SEND_RETRY_MS / 1000;
  timeout.tv_usec = SYNCHRO_SEND_RETRY_MS % 1000;


  if (user_message) {
    LOG("[SYNC] Send signal '" << user_message << "' (id " << signal_id << ") to actor " << target_actor_id);
  } else {
    LOG("[SYNC] Send signal id " << signal_id << " to actor " << target_actor_id);
  }

  uint64_t begin = get_monotonic_time_us();

  // Send signal periodically until timeout reached or acknowledge received
  do {
    int32_t sent_size = sendto(_socket, (const char *)&sig_payload, sizeof(payload_t), 0,
                               (struct sockaddr *)&destination_address, sizeof(struct sockaddr_in));
    ASSERT_SOCKET(sent_size > 0 && (uint32_t)sent_size == sizeof(payload_t), destination_address, _socket, "sendto failed !");

    fd_set select_fd;
    memcpy(&select_fd, &_fd_set, sizeof(fd_set));
    select_status = select(_nfds, &select_fd, NULL, NULL, &timeout);
  } while (select_status <= 0 && get_monotonic_time_us() - begin < SYNCHRO_TIMEOUT_MS * 1000);

  ASSERT(select_status > 0, "Synchro signal failed! No acknowledge received!");

  // Read and check acknowledge
  payload_t ack_payload;
  int size = ::recvfrom(_socket, (char*) &ack_payload, sizeof(payload_t), 0, nullptr, 0);
  ASSERT(size == sizeof(payload_t), "Synchro signal failed! Invalid acknowledge size received!");
  ASSERT(ack_payload.from_actor_id == target_actor_id, "Synchro signal failed! Acknowledge received from actor " <<
         ack_payload.from_actor_id << " where " << target_actor_id << " was expected.");
  ASSERT(ack_payload.signal_id == signal_id, "Synchro signal failed! Acknowledge received with signal id " <<
         ack_payload.signal_id << " where " << signal_id << " was expected.");

  LOG("[SYNC] Acknowledge received.");
}


//
// Wait signal and send ack
//
void synchro_wait(uint32_t from_actor_id, const char* user_message, const char* caller_file, uint32_t caller_line)
{
  uint32_t signal_id = _actor_recv_signal_id[from_actor_id]++;

  if (user_message) {
    LOG("[SYNC] Wait signal '" << user_message << "' (id " << signal_id << ") from actor " << from_actor_id);
  } else {
    LOG("[SYNC] Wait signal id " << signal_id << " from actor " << from_actor_id);
  }

  // Wait data
  struct ::timeval timeout;
  timeout.tv_sec = SYNCHRO_TIMEOUT_MS / 1000;
  timeout.tv_usec = SYNCHRO_TIMEOUT_MS % 1000;
  fd_set select_fd;

  // Signal data
  payload_t sig_payload;
  int size = 0;

  // Read all signals until the one we expect or timeout reached
  do {
    memcpy(&select_fd, &_fd_set, sizeof(fd_set));
    int select_status = select(_nfds, &select_fd, NULL, NULL, &timeout);
    ASSERT(select_status > 0, "Synchro wait failed! No message received!");

    size = ::recvfrom(_socket, (char*) &sig_payload, sizeof(payload_t), 0, nullptr, 0);
    ASSERT(size == sizeof(payload_t), "Synchro wait failed! Invalid message size received!");

    if (sig_payload.from_actor_id != from_actor_id) {
      // Here we expect to drop old messages from a sync with another actor but this API mail
      // fail with more than two actors
      DEBUG("Dropping signal received from an unexpected actor id " << sig_payload.from_actor_id);
      continue;
    }

    if (sig_payload.signal_id < signal_id) {
      DEBUG("Dropping signal id " << sig_payload.signal_id << " received from a previous sync");
      continue;
    }

  } while (sig_payload.signal_id < signal_id);

  // Check signal content
  ASSERT(sig_payload.signal_id == signal_id, "Synchro wait failed! Message received with signal id " <<
         sig_payload.signal_id << " where " << signal_id << " was expected.");

  // Send acknowledge
  socket_address_t destination_address(_host_ip, _base_port + from_actor_id);
  payload_t ack_payload = { _actor_id, signal_id };
  int32_t sent_size = sendto(_socket, (const char *)&ack_payload, sizeof(payload_t), 0,
                             (struct sockaddr *)&destination_address, sizeof(struct sockaddr_in));
  ASSERT_SOCKET(sent_size > 0 && (uint32_t)sent_size == sizeof(payload_t), destination_address, _socket, "sendto failed !");

  LOG("[SYNC] Signal received.");
}
