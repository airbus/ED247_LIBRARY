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

#ifndef _ED247_LOGS_H_
#define _ED247_LOGS_H_

#include "ed247.h"

#include <string.h>
#include <ostream>
#include <iostream>
#include <iomanip>
#include <exception>
#include <fstream>
#include <sstream>

#define LOG_DEBUG()     ed247::Logs::log(ED247_LOG_LEVEL_DEBUG)
#define LOG_INFO()      ed247::Logs::log(ED247_LOG_LEVEL_INFO)
#define LOG_WARNING()   ed247::Logs::log(ED247_LOG_LEVEL_WARNING)
#define LOG_ERROR()     ed247::Logs::log(ED247_LOG_LEVEL_ERROR)
#define LOG_END         std::endl;

#define IF_PRINT if(Configuration::getInstance().get().enable_logs_during_send_receive)

#define PRINT_DEBUG(x) LOG_DEBUG() << x << LOG_END
#define PRINT_INFO(x) LOG_INFO() << x << LOG_END
#define PRINT_WARNING(x) LOG_WARNING() << x << LOG_END
#define PRINT_ERROR(x) LOG_ERROR() << x << LOG_END

#define THROW_ED247_ERROR(ex,message)                               \
    do{                                                             \
        LOG_ERROR() << __FILE__ << ":" << __LINE__ << std::endl     \
            << message << LOG_END;                                  \
        std::ostringstream oss;                                     \
        oss << message;                                             \
        throw ed247::exception(ex,oss.str());                       \
    }while(0)

#define ASSERT_ED247(condition, message)                            \
    if(!(condition)){                                               \
        LOG_ERROR() << __FILE__ << ":" << __LINE__ << std::endl     \
            << message << LOG_END;                                  \
        assert(condition);                                          \
    }

namespace ed247
{

class Logs
{
    public:

        const char * ENV_ED247_LOG_LEVEL = "ED247_LOG_LEVEL";
        const char * ENV_ED247_LOG_FILEPATH = "ED247_LOG_FILEPATH";

        static Logs & getInstance()
        {
            static Logs instance;
            return instance;
        }

        static void configure(const libed247_configuration_t & libed247_configuration)
        {
            Logs & instance = getInstance();
            if(libed247_configuration.log_level != ED247_LOG_LEVEL__INVALID) instance.setLogLevel(libed247_configuration.log_level);
            if(libed247_configuration.log_filepath != NULL) instance.setLogFilepath(libed247_configuration.log_filepath);
        }

        static Logs & log(const ed247_log_level_t & log_current)
        {
            Logs & instance = getInstance();
            instance.prepare(log_current);
            return instance;
        }

        void setLogLevel(const int & log_level);
        const ed247_log_level_t & getLogLevel() const;

        void setLogFilepath(const char *filepath);

        void prepare(const ed247_log_level_t & log_current);

        static const std::string & strLogLevel(ed247_log_level_t);

        template <typename T>
        Logs & operator << (const T & message)
        {
            if(_log_current <= _log_level){
                std::ostream & stream = (_log_current == ED247_LOG_LEVEL_ERROR) ? _stream_err : _stream_out;
                stream << message;
                if(_stream_file.is_open())
                    _stream_file << message;
            }
            if(_log_current == ED247_LOG_LEVEL_ERROR){
                _errors << message;
            }
            return *this;
        }
        Logs & operator << (std::ostream& (*pf) (std::ostream&))
        {
            if(_log_current <= _log_level){
                std::ostream & stream = (_log_current == ED247_LOG_LEVEL_ERROR) ? _stream_err : _stream_out;
                stream << pf;
                if(_stream_file.is_open())
                    _stream_file << pf;
            }
            if(_log_current == ED247_LOG_LEVEL_ERROR){
                _errors << pf;
            }
            return *this;
        }

        std::string & errors()
        {
             std::string newerr = _errors.str();
            static std::string errors;
            if(newerr.empty()){
                errors = "No error";
            }else{
                errors = newerr;
            }
            return errors;
        }

    private:
        Logs();
        void setLogCurrent(const ed247_log_level_t & log_current);

        ~Logs()= default;
        Logs(const Logs&)= delete;
        Logs& operator=(const Logs&)= delete;

    private:
        ed247_log_level_t   _log_level;
        ed247_log_level_t   _log_current;
        std::ostream        &_stream_out;
        std::ostream        &_stream_err;
        std::ofstream       _stream_file;
        std::ostringstream  _errors;
};

class ContentLogger
{
    public:
        ContentLogger() {}
        virtual ~ContentLogger() {}
        virtual void dump_content() = 0;
    
};

class exception : public std::exception
{
    public:
        inline exception(ed247_status_t status, std::string what = std::string()) :
            _status(status),
            _what(what)
        {}
        virtual ~exception() {}
        
        virtual const char *what() const noexcept;
        ed247_status_t getStatus() const
            { return _status; }

    private:
        ed247_status_t _status;
        mutable std::string _what;
};

}

#endif