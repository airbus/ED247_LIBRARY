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

#include "ed247_logs.h"

#include <sstream>
#include <algorithm>

namespace {
    const std::string LOG_LABEL_TEST =    "TEST";
    const std::string LOG_LABEL_DEBUG =   "DEBUG";
    const std::string LOG_LABEL_INFO =    "INFO";
    const std::string LOG_LABEL_WARNING = "WARNING";
    const std::string LOG_LABEL_ERROR =   "ERROR";
    const std::string LOG_LABEL_UNKNOWN = "";
}

namespace ed247
{

Logs::Logs():
_log_level(ED247_LOG_LEVEL_ERROR),
_log_current(ED247_LOG_LEVEL_DEBUG),
_stream_out(std::cout),
_stream_err(std::cerr)
{
	char * str_log_level;
	char * str_log_filepath;
#ifdef _MSC_VER
	size_t len;
	_dupenv_s(&str_log_level, &len, Logs::ENV_ED247_LOG_LEVEL);
	_dupenv_s(&str_log_filepath, &len, Logs::ENV_ED247_LOG_FILEPATH);
#else
    str_log_level = getenv(Logs::ENV_ED247_LOG_LEVEL);
    str_log_filepath = getenv(Logs::ENV_ED247_LOG_FILEPATH);
#endif
    _errors.str("");
    if(str_log_level)
        setLogLevel(atoi(str_log_level));
    if(str_log_filepath){
        _stream_file.open(str_log_filepath);
        if(!_stream_file.is_open())
            std::cerr << "Failed to open logging file [" << std::string(str_log_filepath) << "]" << std::endl;
        else
            std::cout << "Logging to file [" << std::string(str_log_filepath) << "]" << std::endl;
    }
}

void Logs::setLogLevel(
    const int & log_level)
{
    _log_level = (ed247_log_level_t)log_level;
    if (log_level < (int)ED247_LOG_LEVEL_ERROR)
    {
        _log_level = ED247_LOG_LEVEL_ERROR;
    }
    else if (log_level > (int)ED247_LOG_LEVEL_TEST)
    {
        _log_level = ED247_LOG_LEVEL_TEST;
    }
    else
    {
        _log_level = (ed247_log_level_t)log_level;
    }
}

void Logs::setLogCurrent(
    const ed247_log_level_t & log_current)
{
    _log_current = log_current;
}

const ed247_log_level_t & Logs::getLogLevel() const
{
    return _log_level;
}

void Logs::prepare(
    const ed247_log_level_t & log_current)
{
    setLogCurrent(log_current);
    if(_log_current <= _log_level && _log_current != ED247_LOG_LEVEL_TEST){
        std::ostream & stream = (_log_current == ED247_LOG_LEVEL_ERROR) ? _stream_err : _stream_out;
        stream << "[ " << std::left << std::setfill(' ') << std::setw(8) << Logs::strLogLevel(_log_current) << std::setfill(' ') << " ] ";
        _stream_file << "[ " << std::left << std::setfill(' ') << std::setw(8) << Logs::strLogLevel(_log_current) << std::setfill(' ') << " ] ";
    }
}

const std::string & Logs::strLogLevel(ed247_log_level_t log_level)
{
    switch(log_level){
        case ED247_LOG_LEVEL_TEST:
            return LOG_LABEL_TEST;
        case ED247_LOG_LEVEL_DEBUG:
            return LOG_LABEL_DEBUG;
        case ED247_LOG_LEVEL_INFO:
            return LOG_LABEL_INFO;
        case ED247_LOG_LEVEL_WARNING:
            return LOG_LABEL_WARNING;
        case ED247_LOG_LEVEL_ERROR:
            return LOG_LABEL_ERROR;
        default: 
            return LOG_LABEL_UNKNOWN;
    }
}

const char * exception::what() const throw()
{
    return _what.c_str();
}

}
