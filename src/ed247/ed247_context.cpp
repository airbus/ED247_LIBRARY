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

Context::Context(
    const libed247_configuration_t & libed247_configuration):
    _configuration(libed247_configuration),
    _root(),
    _pool_interfaces(std::make_shared<UdpSocket::Pool>()),
    _pool_signals(std::make_shared<BaseSignal::Pool>()),
    _pool_streams(std::make_shared<BaseStream::Pool>(_pool_signals)),
    _pool_channels(_pool_interfaces, _pool_streams),
    _active_frames(std::make_shared<SmartListActiveFrames>()),
    _active_streams(std::make_shared<SmartListActiveStreams>())
{
    PRINT_DEBUG("# [Context] Ctor");
    _active_frames->set_managed(true);
    _active_streams->set_managed(true);
    SimulationTimeHandler::get().set_handler(libed247_set_simulation_time_ns, NULL);
    Configuration::getInstance().set(_configuration);
}

Context::~Context()
{
// #ifdef ENABLE_MEMHOOKS
//     MemoryHooksManager::getInstance().setEnable(false);
// #endif
}

void Context::initialize()
{
    Context::Builder::initialize(*this);
// #ifdef ENABLE_MEMHOOKS
//     // MemoryHooksManager
//     MemoryHooksManager::getInstance().setEnable(_configuration.enable_memory_hooks==1);
// #endif
}

const libed247_runtime_metrics_t* Context::get_runtime_metrics()
{
    _runtime_metrics.missed_frames = 0;
    for (auto channel: *(_pool_channels.channels()))
    {
        _runtime_metrics.missed_frames += channel->missed_frames();
    }
    return &_runtime_metrics;
}

// Context::Builder

Context * Context::Builder::create_filepath(std::string ecic_filepath,
    const libed247_configuration_t & libed247_configuration)
{
    // Logs
    PRINT_DEBUG("# Log level [" << Logs::strLogLevel(Logs::getInstance().getLogLevel()) << "]");
    PRINT_DEBUG("## DEBUG logs enabled");
    PRINT_INFO("## INFO logs enabled");
    PRINT_WARNING("## WARNING enabled");

    Logs::getInstance().configure(libed247_configuration);

    // Create context
    Context * context = new Context(libed247_configuration);

    PRINT_DEBUG("## ECIC filepath [" << ecic_filepath << "]");
    
    // Load
    try{
        context->_root = std::dynamic_pointer_cast<xml::Root>(xml::load_filepath(ecic_filepath));
    }catch(...){
        delete context;
        context = nullptr;
        throw;
    }

    return context;
}

Context * Context::Builder::create_content(std::string ecic_content,
    const libed247_configuration_t & libed247_configuration)
{
    // Logs
    PRINT_DEBUG("# Log level [" << Logs::strLogLevel(Logs::getInstance().getLogLevel()) << "]");
    PRINT_DEBUG("## DEBUG logs enabled");
    PRINT_INFO("## INFO logs enabled");
    PRINT_WARNING("## WARNING enabled");

    Logs::getInstance().configure(libed247_configuration);

    // Create context
    Context * context = new Context(libed247_configuration);

    PRINT_DEBUG("## ECIC content [" << ecic_content << "]");
    
    // Load
    try{
        context->_root = std::dynamic_pointer_cast<xml::Root>(xml::load_content(ecic_content));
    }catch(...){
        delete context;
        context = nullptr;
        throw;
    }

    return context;
}

void Context::Builder::initialize(Context & context)
{
    // Create channels
    context._active_frames->clear();
    for(auto & sp_channel_configuration : context._root->channels){
        auto sp_channel = context._pool_channels.get(sp_channel_configuration); // Add component identifier
        auto frame_element = std::make_shared<ed247_frame_t>();
        frame_element->channel = sp_channel.get();
        context._active_frames->push_back(frame_element);
    }
    context._active_frames->reset();
    context._pool_streams->streams()->reset();

    context._active_streams->resize(context._pool_streams->size(), nullptr);
    copy(context._pool_streams->streams()->begin(),
        context._pool_streams->streams()->end(),
        context._active_streams->begin());
    context._active_streams->reset();
    context._pool_streams->streams()->reset();
    context._pool_channels.channels()->reset();
}

}