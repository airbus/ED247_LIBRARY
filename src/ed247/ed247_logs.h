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
#ifndef _ED247_LOGS_H_
#define _ED247_LOGS_H_

#include "ed247.h"
#include <string.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <exception>


// Prefix traces by file:line
#define LOG_SHORTFILE       (strrchr("/" __FILE__, '/') + 1)
#define LOG_STREAM_FILELINE LOG_SHORTFILE << ":" << __LINE__ << " "

// Print a trace regardless log level
#define SAY(m) do { SAY_STREAM(ED247_LOG_STREAM, m); } while (0)

// Print a trace depending on log level
#define PRINT_ERROR(m)   do { if (ED247_LOG_ENABLED(ED247_LOG_LEVEL_ERROR))   ED247_LOG_STREAM << LOG_STREAM_FILELINE << "[ERROR] " << m << std::endl; } while (0)
#define PRINT_WARNING(m) do { if (ED247_LOG_ENABLED(ED247_LOG_LEVEL_WARNING)) ED247_LOG_STREAM << LOG_STREAM_FILELINE << "[WARN] "  << m << std::endl; } while (0)
#define PRINT_INFO(m)    do { if (ED247_LOG_ENABLED(ED247_LOG_LEVEL_INFO))    ED247_LOG_STREAM << LOG_STREAM_FILELINE << m << std::endl; } while (0)
#define PRINT_DEBUG(m)   do { if (ED247_LOG_ENABLED(ED247_LOG_LEVEL_DEBUG))   ED247_LOG_STREAM << LOG_STREAM_FILELINE << m << std::endl; } while (0)
#define PRINT_CRAZY(m)   do { if (ED247_LOG_ENABLED(ED247_LOG_LEVEL_CRAZY))   ED247_LOG_STREAM << LOG_STREAM_FILELINE << m << std::endl; } while (0)

#ifdef SIMULINK_LOGGER_ENABLED
# include "Logger.hpp"
# define ED247_LOG_STREAM LOG(info, 0)
#else
# define ED247_LOG_STREAM ed247::log::get().stream()
#endif

#define ED247_LOG_ENABLED(level) ed247::log::get().enabled(level)
#define SAY_STREAM(stream, m)    do { (stream) << LOG_STREAM_FILELINE << m << std::endl; } while (0)

// FRIEND_TEST macro will be defined by gtest only while building unitary tests
#ifndef FRIEND_TEST
#define FRIEND_TEST(...)
#endif

// Logger internals
namespace ed247 {
  struct LIBED247_EXPORT log
  {
    static constexpr const char* ENV_VAR_LEVEL    = "ED247_LOG_LEVEL";
    static constexpr const char* ENV_VAR_FILEPATH = "ED247_LOG_FILEPATH";

    // Get the logger. May create it (see reset()).
    static log& get() { return *((_logger)? _logger : create_logger()); }

    // Change logger parameters.
    // Defaut values are ED247_LOG_LEVEL_DEFAULT and std::cerr.
    // The env variables have always the priority (see ENV_VAR_*).
    void reset(ed247_log_level_t level = ED247_LOG_LEVEL_DEFAULT, const char* filepath = nullptr);

    // Return the stream to write to.
    std::ostream& stream() { return (_fstream.is_open())? _fstream : std::cerr; }

    // Return the log level.
    ed247_log_level_t level() { return get()._level; }

    // Return true if log is enabled at `level'
    bool enabled(ed247_log_level_t level) { return _level >= level; }

    // Convert `level' to a string
    static std::string level_name(ed247_log_level_t level);

  private:
    static log* create_logger();
    static void delete_logger();

    log() : _level(ED247_LOG_LEVEL_UNSET) { reset(); }
    ~log();
    void set_level(ed247_log_level_t level);

    static log*       _logger;
    ed247_log_level_t _level;
    std::ofstream     _fstream;
    std::string       _filepath;

    FRIEND_TEST(LogConfigurationTest, LoggingByEnv);
    FRIEND_TEST(LogConfigurationTest, LoggingByArgs);
  };
}

// Generate a backtrace (if supported)
extern LIBED247_EXPORT void ed247_log_backtrace();

// ED247 exception
namespace ed247 {
  class exception : public std::exception
  {
  public:
    exception(std::string message) : _message (message) {};
    virtual ~exception() throw () override {}
    inline const char* what() const noexcept override { return _message.c_str(); }
  private:
    std::string _message;
  };
}
#define THROW_ED247_ERROR(message)                  \
  do {                                              \
    PRINT_ERROR(message);                           \
    throw ed247::exception(strize() << message);    \
  } while(0)


// ED247 enums to stream
inline std::ostream& operator<<(std::ostream& stream, const ed247_log_level_t& level)     { return (stream << ed247::log::level_name(level));     }
inline std::ostream& operator<<(std::ostream& stream, const ed247_status_t& status)       { return (stream << ed247_status_string(status));       }
inline std::ostream& operator<<(std::ostream& stream, const ed247_direction_t& direction) { return (stream << ed247_direction_string(direction)); }
inline std::ostream& operator<<(std::ostream& stream, const ed247_stream_type_t& stype)   { return (stream << ed247_stream_type_string(stype));   }
inline std::ostream& operator<<(std::ostream& stream, const ed247_signal_type_t& stype)   { return (stream << ed247_signal_type_string(stype));   }
inline std::ostream& operator<<(std::ostream& stream, const ed247_nad_type_t& ntype)      { return (stream << ed247_nad_type_string(ntype));      }

// Helper to create a string from stream: std::string foo = strize() << "hello " << 42;
struct strize {
  template<typename T> strize& operator<<(T value) { _content << value; return *this; }
  operator std::string() const { return _content.str(); }
private:
  std::stringstream _content;
};


// Helper to convert a payload to hexa stream: stream << "data: " << hex_stream(data, 4) << std::endl;
class hex_stream
{
public:
  hex_stream(const void* payload, int len) : _payload((const uint8_t*)payload), _len(len) {}
  LIBED247_EXPORT friend std::ostream& operator<<(std::ostream& stream, const hex_stream&);
private:
  const uint8_t* _payload;
  int            _len;
};

#endif
