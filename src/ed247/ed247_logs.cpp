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
#include "ed247_logs.h"
#include <iomanip>


// ed247_log_backtrace configuration
//
#ifdef __linux__
// Just print the result of backtrace_symbols. Maybe not useful...
#define BACKTRACE_SYMBOLS
// Find symbol with dladdr and demangle them
// Library need to be builded WITHOUT -fvisibility=hidden and WITH -ldl
//#define BACKTRACE_DLADDR_CXX_DEMANGLE
// Call gdb external tool
//#define BACKTRACE_GDB
#else
#define BACKTRACE_DISABLED
#endif

//
// Logger
//

// Note: Ctor and Dtor cannot be inlined to prevent symbols
// finding issue on Windows with dllexport/dllimport.
ed247::log* ed247::log::_logger = nullptr;

ed247::log* ed247::log::create_logger()
{
  _logger = new log();
  return _logger;
}

void ed247::log::delete_logger()
{
  delete ed247::log::_logger;
  ed247::log::_logger = nullptr;
}

ed247::log::~log()
{
  if (_fstream.is_open()) _fstream.close();
  _logger = nullptr;
}

// Main Ctor
void ed247::log::reset(ed247_log_level_t caller_level, const char* caller_filepath)
{
  const char* env_level;
  const char* new_filepath;
#ifdef _MSC_VER
  size_t len;
  _dupenv_s(&env_level, &len, ENV_VAR_LEVEL);
  _dupenv_s(&new_filepath, &len, ENV_VAR_FILEPATH);
#else
  env_level = getenv(ENV_VAR_LEVEL);
  new_filepath = getenv(ENV_VAR_FILEPATH);
#endif

  // Log stream (prio to env variable)
  if (new_filepath && *new_filepath) {
    SAY_STREAM(stream(), "Redirect log using env variable " << ENV_VAR_FILEPATH << " to file '" << new_filepath << "'");
  } else if (caller_filepath && *caller_filepath) {
    SAY_STREAM(stream(), "Redirect log to file '" << caller_filepath << "'");
    new_filepath = caller_filepath;
  }
  if (new_filepath && *new_filepath) {
    if (_fstream.is_open()) {
      // If we log to the same opened file, do not trucate it
      if (new_filepath != _filepath) {
        _fstream.close();
        _fstream.open(new_filepath);
      }
    }
    // fstream not already opened
    else {
      _fstream.open(new_filepath);
    }

    if (_fstream.is_open() == false) {
      SAY_STREAM(stream(), "[ERROR] Failed to open log file '" << new_filepath << "' ! Use default output.");
    } else {
      _filepath = new_filepath;
    }
  }

  // Log level (prio to env variable)
  if (env_level && *env_level) {
    char* first_invalid_char;
    long new_level = strtol(env_level, &first_invalid_char, 10);
    if (*first_invalid_char == 0) {
      SAY_STREAM(stream(), "Set log level using env variable " << ENV_VAR_LEVEL);
      set_level((ed247_log_level_t)new_level);
    } else {
      SAY_STREAM(stream(), "[ERROR] Invalid env variable " << ENV_VAR_LEVEL << " value: '" << env_level << "'");
    }
  } else if (caller_level != ED247_LOG_LEVEL_UNSET) {
    set_level(caller_level);
  } else if (_level == ED247_LOG_LEVEL_UNSET) {
    set_level(ED247_LOG_LEVEL_DEFAULT);
  } else if (_level > ED247_LOG_LEVEL_ERROR) {
    SAY_STREAM(stream(), "Log level is set to " << level_name(_level));
  }
}

void ed247::log::set_level(ed247_log_level_t new_level)
{
  new_level = (ed247_log_level_t) std::min(std::max(new_level, ED247_LOG_LEVEL_MIN), ED247_LOG_LEVEL_MAX);
  if (new_level != _level) {
    if (new_level != ED247_LOG_LEVEL_DEFAULT || _level != ED247_LOG_LEVEL_UNSET) {
      SAY_STREAM(stream(), "Set log level to " << level_name(new_level));
    }
    _level = new_level;
  }
}

std::string ed247::log::level_name(ed247_log_level_t level)
{
  if (level >= ED247_LOG_LEVEL_UNSET)   return "UNSET";
  if (level >= ED247_LOG_LEVEL_CRAZY)   return "CRAZY";
  if (level >= ED247_LOG_LEVEL_DEBUG)   return "DEBUG";
  if (level >= ED247_LOG_LEVEL_INFO)    return "INFO";
  if (level >= ED247_LOG_LEVEL_WARNING) return "WARNING";
  return "ERROR";
}


//
// Format hexa payload
//
std::ostream& operator<<(std::ostream &stream, const hex_stream& self)
{
  if (self._payload != nullptr) {
    std::ios::fmtflags old_flags = stream.flags();
    stream.setf(std::ios::hex, std::ios::basefield);
    stream.setf(std::ios::uppercase);
    char old_fill = stream.fill('0');

    for (int pos = 0; pos < self._len; pos++) {
      stream << std::setw(2) << (int)self._payload[pos] << " ";
    }

    stream.fill(old_fill);
    stream.flags(old_flags);
  }
  else {
    stream << "(null)";
  }

  return stream;
}

#ifdef BACKTRACE_DISABLED
void ed247_log_backtrace()
{
  SAY_STREAM(std::cerr, "Backtrace disabled.");
}
#endif

#ifdef BACKTRACE_SYMBOLS
#include <execinfo.h>
void ed247_log_backtrace()
{
  SAY_STREAM(std::cerr, "Backtrace:");

  const int buffer_size = 20;
  void* buffer[buffer_size];
  int nb_address = backtrace(buffer, buffer_size);
  char** symbols = backtrace_symbols(buffer, nb_address);

  for(int i = 1 ; i < nb_address ; i++) {
    std::cerr << "[BT] " << std::setw(2) << (i-1) << " " << symbols[i] << std::endl;
  }
  free(symbols);
}
#endif

#ifdef BACKTRACE_DLADDR_CXX_DEMANGLE
#include <execinfo.h>
#include <dlfcn.h>
#include <cxxabi.h>
void ed247_log_backtrace()
{
  SAY_STREAM(std::cerr, "Backtrace:");

  const int buffer_size = 20;
  void* buffer[buffer_size];
  int nb_address = backtrace(buffer, buffer_size);
  char** symbols = backtrace_symbols(buffer, nb_address);

  for(int i = 1 ; i < nb_address ; i++) {
    std::cerr << "[BT] " << std::setw(2) << (i-1) << " ";
	Dl_info info;
    if (dladdr(buffer[i], &info) && info.dli_sname) {
      int status;
      char *demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
      std::cerr << ((status == 0) ? demangled : info.dli_sname);
      free(demangled);
    } else {
      std::cerr << symbols[i];
    }
    std::cerr << std::endl;
  }
  free(symbols);
}
#endif

#ifdef BACKTRACE_GDB
#include <unistd.h>
#include <sys/wait.h>
void ed247_log_backtrace()
{
  SAY_STREAM(std::cerr, "Backtrace:");

  std::string pid = strize() << getpid();
  char name[1024];
  name[readlink("/proc/self/exe", name, 1023)] = 0;

  int gdb_pid = fork();
  if (gdb_pid == 0) {
    dup2(2,1); // redirect output to stderr
    execl("/usr/bin/gdb", "gdb", "--batch", "-n", "-ex", "thread", "-ex", "bt", name, pid.c_str(), NULL);
    abort(); // gdb failed to start
  } else {
    waitpid(gdb_pid, NULL, 0);
  }
}
#endif

