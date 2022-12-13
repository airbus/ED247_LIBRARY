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

/************
 * Includes *
 ************/

#include "ed247_context.h"
#include "ed247_xml.h"
#include "ed247_channel.h"

#include <typeinfo>
#include <iostream>
#include <sstream>

/***********
 * Methods *
 ***********/

namespace ed247
{

Context::Context():
    _stream_set(_signal_set),
    _channel_set(_receiver_set, _stream_set)
{
    PRINT_DEBUG("[Context] Ctor");
}

Context::~Context()
{
}

void Context::initialize()
{
    Context::Builder::initialize(*this);
}

// Context::Builder

Context * Context::Builder::create_filepath(std::string ecic_filepath)
{
    // Create context
    Context * context = new Context();

    PRINT_DEBUG("ECIC filepath [" << ecic_filepath << "]");

    // Load
    try{
        context->_configuration = xml::load_filepath(ecic_filepath);
    }catch(...){
        delete context;
        context = nullptr;
        throw;
    }

    return context;
}

Context * Context::Builder::create_content(std::string ecic_content)
{
    // Create context
    Context * context = new Context();

    PRINT_DEBUG("ECIC content [" << ecic_content << "]");

    // Load
    try{
        context->_configuration = xml::load_content(ecic_content);
    }catch(...){
        delete context;
        context = nullptr;
        throw;
    }

    return context;
}

void Context::Builder::initialize(Context & context)
{
  for(const xml::Channel& sp_channel_configuration : context._configuration->_channel_list) {
    context._channel_set.create(&sp_channel_configuration, context._configuration->_identifier);
  }
}

}
