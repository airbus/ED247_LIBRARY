/* -*- mode: c++; c-basic-offset: 2 -*-  */
//
// Define bswap_xx and htonx/ntohx functions.
//
#ifndef _ED247_BSWAP_H_
#define _ED247_BSWAP_H_


#ifdef __linux__
# define bswap_16(x) __builtin_bswap16(x)
# define bswap_32(x) __builtin_bswap32(x)
# define bswap_64(x) __builtin_bswap64(x)
#elif __QNXNTO__
# include <gulliver.h>
# define bswap_16(x) ENDIAN_RET16(x)
# define bswap_32(x) ENDIAN_RET32(x)
# define bswap_64(x) ENDIAN_RET64(x)
#elif _MSC_VER
# include <stdlib.h>
# include <winsock2.h>
# define bswap_16(x) _byteswap_ushort(x)
# define bswap_32(x) _byteswap_ulong(x)
# define bswap_64(x) _byteswap_uint64(x)
#elif _WIN32
# include <winsock2.h>
# define bswap_16(x) __builtin_bswap16(x)
# define bswap_32(x) __builtin_bswap32(x)
# define bswap_64(x) __builtin_bswap64(x)
#endif

#ifdef htons
# undef htons
#endif
#define htons(x) bswap_16(x)

#ifdef ntohs
# undef ntohs
#endif
#define ntohs(x) bswap_16(x)

#ifdef ntohl
# undef ntohl
#endif
#define ntohl(x) bswap_32(x)

#ifdef htonl
# undef htonl
#endif
#define htonl(x) bswap_32(x)

#endif
