/******************************************************************************
 * The MIT Licence
 *
 * Copyright (c) 2019 Airbus Operations S.A.S
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

#include "ed247.h"
#include "ed247_internals.h"
#include "ed247_logs.h"
#include "ed247_context.h"
#include "ed247_memhooks.h"

#include <memory>

/***********
 * Defines *
 ***********/

#ifdef LIBED247_VERSION
# define QUOTE(x) #x
# define EXPAND(x) QUOTE(x)
const char* _ed247_version = EXPAND(LIBED247_VERSION);
#else
const char* _ed247_version = "unversioned";
#endif

#define LIBED247_CATCH(topic)                                   \
    catch(std::exception &ex)                                   \
    {                                                           \
        LOG_DEBUG() << "## " << topic << " failed" << LOG_END;  \
        LOG_ERROR() << ex.what() << LOG_END;                    \
        return ED247_STATUS_FAILURE;                            \
    }                                                           \
    catch(...)                                                  \
    {                                                           \
        LOG_DEBUG() << "## " << topic << " failed" << LOG_END;  \
        LOG_ERROR() << "Unknown error" << LOG_END;              \
        return ED247_STATUS_FAILURE;                            \
    }

/************
 * # Common *
 ************/

const char * libed247_errors()
{
    return ed247::Logs::getInstance().errors().c_str();
}

const char * ed247_status_string(
    ed247_status_t status)
{
    switch(status){
        case ED247_STATUS_SUCCESS:          return ed247::defines::status::SUCCESS.c_str();
        case ED247_STATUS_FAILURE:          return ed247::defines::status::FAILURE.c_str();
        case ED247_STATUS_TIMEOUT:          return ed247::defines::status::TIMEOUT.c_str();
        case ED247_STATUS_NODATA:           return ed247::defines::status::NODATA.c_str();
        case ED247_STATUS_STOP:             return ed247::defines::status::STOP.c_str();
        default:                            return "Unknown";
    }
}

const char * ed247_standard_string(
    ed247_standard_t standard)
{
    switch(standard){
        case ED247_STANDARD_ED247:  return ed247::defines::standard::ED247.c_str();
        case ED247_STANDARD_ED247A: return ed247::defines::standard::ED247A.c_str();
        default:                    return "Unknown";
    }
}

const char * ed247_direction_string(
    ed247_direction_t direction)
{
    switch(direction){
        case ED247_DIRECTION_IN:    return ed247::defines::direction::INPUT.c_str();
        case ED247_DIRECTION_OUT:   return ed247::defines::direction::OUTPUT.c_str();
        case ED247_DIRECTION_INOUT: return ed247::defines::direction::INOUT.c_str();
        default:                    return "Unknown";
    }
}

ed247_standard_t ed247_standard_from_string(
    const char *standard)
{
    using namespace ed247::defines::standard;
    if(ED247.compare(standard) == 0){
        return ED247_STANDARD_ED247;
    }else if(ED247A.compare(standard) == 0){
        return ED247_STANDARD_ED247A;
    }else{
        return ED247_STANDARD__INVALID;
    }
}

ed247_direction_t ed247_direction_from_string(
    const char *direction)
{
    using namespace ed247::defines;
    if(direction::INPUT.compare(direction) == 0){
        return ED247_DIRECTION_IN;
    }else if(direction::OUTPUT.compare(direction) == 0){
        return ED247_DIRECTION_OUT;
    }else if(direction::INOUT.compare(direction) == 0){
        return ED247_DIRECTION_INOUT;
    }else{
        return ED247_DIRECTION__INVALID;
    }
}

const char * ed247_yesno_string(
    ed247_yesno_t yesno)
{
    using namespace ed247::defines::yesno;
    switch(yesno){
        case ED247_YESNO_NO:    return NO.c_str();
        case ED247_YESNO_YES:   return YES.c_str();
        default:                return "Unknown";
    }
}

ed247_yesno_t ed247_yesno_from_string(
    const char *yesno)
{
    using namespace ed247::defines::yesno;
    std::string yn(yesno);
    std::transform(yn.begin(), yn.end(), yn.begin(), ::toupper);
    if(yn.compare("NO") == 0){
        return ED247_YESNO_NO;
    }else if(yn.compare("0") == 0){
        LOG_WARNING() << "0 is also accepted to say NO" << LOG_END
        return ED247_YESNO_NO;
    }else if(yn.compare("YES") == 0){
        return ED247_YESNO_YES;
    }else if(yn.compare("1") == 0){
        LOG_WARNING() << "1 is also accepted to say YES" << LOG_END
        return ED247_YESNO_YES;
    }else{
        return ED247_YESNO__INVALID;
    }
}

const char * ed247_component_type_string(
    ed247_component_type_t component_type)
{
    switch(component_type){
        case ED247_COMPONENT_TYPE_VIRTUAL:  return ed247::defines::component_type::VIRTUAL.c_str();
        case ED247_COMPONENT_TYPE_BRIDGE:   return ed247::defines::component_type::BRIDGE.c_str();
        default:                            return ed247::defines::Unknown.c_str();
    }
}

ed247_component_type_t ed247_component_type_from_string(
    const char *component_type)
{
    using namespace ed247::defines::component_type;
    if(VIRTUAL.compare(component_type) == 0){
        return ED247_COMPONENT_TYPE_VIRTUAL;
    }else if(BRIDGE.compare(component_type) == 0){
        return ED247_COMPONENT_TYPE_BRIDGE;
    }else{
        return ED247_COMPONENT_TYPE__INVALID;
    }
}

const char * ed247_stream_type_string(
    ed247_stream_type_t stream_type)
{
    switch(stream_type){
        case ED247_STREAM_TYPE_A664:       return ed247::defines::stream_type::A664.c_str();
        case ED247_STREAM_TYPE_A429:       return ed247::defines::stream_type::A429.c_str();
        case ED247_STREAM_TYPE_A825:       return ed247::defines::stream_type::A825.c_str();
        case ED247_STREAM_TYPE_M1553:      return ed247::defines::stream_type::M1553.c_str();
        case ED247_STREAM_TYPE_SERIAL:     return ed247::defines::stream_type::SERIAL.c_str();
        case ED247_STREAM_TYPE_AUDIO:      return ed247::defines::stream_type::AUDIO.c_str();
        case ED247_STREAM_TYPE_VIDEO:      return ed247::defines::stream_type::VIDEO.c_str();
        case ED247_STREAM_TYPE_ETHERNET:   return ed247::defines::stream_type::ETHERNET.c_str();
        case ED247_STREAM_TYPE_ANALOG:     return ed247::defines::stream_type::ANALOG.c_str();
        case ED247_STREAM_TYPE_DISCRETE:   return ed247::defines::stream_type::DISCRETE.c_str();
        case ED247_STREAM_TYPE_NAD:        return ed247::defines::stream_type::NAD.c_str();
        case ED247_STREAM_TYPE_VNAD:       return ed247::defines::stream_type::VNAD.c_str();
        default:                        return ed247::defines::Unknown.c_str();
    }
}

ed247_stream_type_t ed247_stream_type_from_string(
    const char *stream_type)
{
    using namespace ed247::defines::stream_type;
    if(A664.compare(stream_type) == 0){
        return ED247_STREAM_TYPE_A664;
    }else if(A429.compare(stream_type) == 0){
        return ED247_STREAM_TYPE_A429;
    }else if(A825.compare(stream_type) == 0){
        return ED247_STREAM_TYPE_A825;
    }else if(M1553.compare(stream_type) == 0){
        return ED247_STREAM_TYPE_M1553;
    }else if(SERIAL.compare(stream_type) == 0){
        return ED247_STREAM_TYPE_SERIAL;
    }else if(AUDIO.compare(stream_type) == 0){
        return ED247_STREAM_TYPE_AUDIO;
    }else if(VIDEO.compare(stream_type) == 0){
        return ED247_STREAM_TYPE_VIDEO;
    }else if(ETHERNET.compare(stream_type) == 0){
        return ED247_STREAM_TYPE_ETHERNET;
    }else if(ANALOG.compare(stream_type) == 0){
        return ED247_STREAM_TYPE_ANALOG;
    }else if(DISCRETE.compare(stream_type) == 0){
        return ED247_STREAM_TYPE_DISCRETE;
    }else if(NAD.compare(stream_type) == 0){
        return ED247_STREAM_TYPE_NAD;
    }else if(VNAD.compare(stream_type) == 0){
        return ED247_STREAM_TYPE_VNAD;
    }else{
        return ED247_STREAM_TYPE__INVALID;
    }
}

const char * ed247_signal_type_string(
    ed247_signal_type_t signal_type)
{
    switch(signal_type){
        case ED247_SIGNAL_TYPE_ANALOG:     return ed247::defines::signal_type::ANALOG.c_str();
        case ED247_SIGNAL_TYPE_DISCRETE:   return ed247::defines::signal_type::DISCRETE.c_str();
        case ED247_SIGNAL_TYPE_NAD:        return ed247::defines::signal_type::NAD.c_str();
        case ED247_SIGNAL_TYPE_VNAD:       return ed247::defines::signal_type::VNAD.c_str();
        default:                        return ed247::defines::Unknown.c_str();
    }
}

ed247_signal_type_t ed247_signal_type_from_string(
    const char *signal_type)
{
    using namespace ed247::defines::signal_type;
    if(ANALOG.compare(signal_type) == 0){
        return ED247_SIGNAL_TYPE_ANALOG;
    }else if(DISCRETE.compare(signal_type) == 0){
        return ED247_SIGNAL_TYPE_DISCRETE;
    }else if(NAD.compare(signal_type) == 0){
        return ED247_SIGNAL_TYPE_NAD;
    }else if(VNAD.compare(signal_type) == 0){
        return ED247_SIGNAL_TYPE_VNAD;
    }else{
        return ED247_SIGNAL_TYPE__INVALID;
    }
}

const char * ed247_nad_type_string(
    ed247_nad_type_t nad_type)
{
    switch(nad_type){
        case ED247_NAD_TYPE_INT8:       return ed247::defines::nad_type::INT8.c_str();
        case ED247_NAD_TYPE_INT16:      return ed247::defines::nad_type::INT16.c_str();
        case ED247_NAD_TYPE_INT32:      return ed247::defines::nad_type::INT32.c_str();
        case ED247_NAD_TYPE_INT64:      return ed247::defines::nad_type::INT64.c_str();
        case ED247_NAD_TYPE_UINT8:      return ed247::defines::nad_type::UINT8.c_str();
        case ED247_NAD_TYPE_UINT16:     return ed247::defines::nad_type::UINT16.c_str();
        case ED247_NAD_TYPE_UINT32:     return ed247::defines::nad_type::UINT32.c_str();
        case ED247_NAD_TYPE_UINT64:     return ed247::defines::nad_type::UINT64.c_str();
        case ED247_NAD_TYPE_FLOAT32:    return ed247::defines::nad_type::FLOAT32.c_str();
        case ED247_NAD_TYPE_FLOAT64:    return ed247::defines::nad_type::FLOAT64.c_str();
        default:                        return ed247::defines::Unknown.c_str();
    }
}

ed247_nad_type_t ed247_nad_type_from_string(
    const char *nad_type)
{
    using namespace ed247::defines;
    if(nad_type::INT8.compare(nad_type) == 0){
        return ED247_NAD_TYPE_INT8;
    }else if(nad_type::INT16.compare(nad_type) == 0){
        return ED247_NAD_TYPE_INT16;
    }else if(nad_type::INT32.compare(nad_type) == 0){
        return ED247_NAD_TYPE_INT32;
    }else if(nad_type::INT64.compare(nad_type) == 0){
        return ED247_NAD_TYPE_INT64;
    }else if(nad_type::UINT8.compare(nad_type) == 0){
        return ED247_NAD_TYPE_UINT8;
    }else if(nad_type::UINT16.compare(nad_type) == 0){
        return ED247_NAD_TYPE_UINT16;
    }else if(nad_type::UINT32.compare(nad_type) == 0){
        return ED247_NAD_TYPE_UINT32;
    }else if(nad_type::UINT64.compare(nad_type) == 0){
        return ED247_NAD_TYPE_UINT64;
    }else if(nad_type::FLOAT32.compare(nad_type) == 0){
        return ED247_NAD_TYPE_FLOAT32;
    }else if(nad_type::FLOAT64.compare(nad_type) == 0){
        return ED247_NAD_TYPE_FLOAT64;
    }else{
        return ED247_NAD_TYPE__INVALID;
    }
}

/*********
 * Lists *
 *********/

ed247_status_t ed247_channel_list_next(
    ed247_channel_list_t channels,
    ed247_channel_t *channel)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Channel list next ..." << LOG_END
#endif
    if(!channels){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Channel list next failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid channels" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!channel){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Channel list next failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty channel pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *channel = nullptr;
    try{
        auto ed247_channels = static_cast<ed247::SmartListChannels*>(channels);
        auto && next = ed247_channels->next_ok();
        *channel = next ? next->get() : nullptr;
    }
    LIBED247_CATCH("Channel list next")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Channel list next success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_channel_list_free(
    ed247_channel_list_t channels)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Channel list free ..." << LOG_END;
#endif
    if(!channels){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Channel list free failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid channels" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_channels = static_cast<ed247::SmartListChannels*>(channels);
        if(!ed247_channels->managed()){
#ifdef LIBED247_VERBOSE_DEBUG
            LOG_DEBUG() << "## Channel list deleted" << LOG_END;
#endif
            delete ed247_channels;
        }
    }
    LIBED247_CATCH("Channel list free")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Channel list free success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_channel_list_size(
    ed247_channel_list_t channels,
    size_t * size)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Channel list size ..." << LOG_END;
#endif
    if(!channels){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Channel list size failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid channels" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!size){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Channel list size failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid size pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_channels = static_cast<ed247::SmartListChannels*>(channels);
        *size = ed247_channels->size();
    }
    LIBED247_CATCH("Channel list size")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Channel list size success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_list_next(
    ed247_stream_list_t streams,
    ed247_stream_t *stream)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Stream list next ..." << LOG_END;
#endif
    if(!streams){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Stream list next failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid streams" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!stream){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Stream list next failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty stream pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *stream = nullptr;
    try{
        auto ed247_streams = static_cast<ed247::SmartListStreams*>(streams);
        auto && next = ed247_streams->next_ok();
        *stream = next ? next->get() : nullptr;
    }
    LIBED247_CATCH("Stream list next")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Stream list next success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_list_free(
    ed247_stream_list_t streams)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Stream list free ..." << LOG_END;
#endif
    if(!streams){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Stream list free failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid streams" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_streams = static_cast<ed247::SmartListStreams*>(streams);
        if(!ed247_streams->managed())
            delete ed247_streams;
    }
    LIBED247_CATCH("Stream list free")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Stream list free success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_list_size(
    ed247_stream_list_t streams,
    size_t * size)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Stream list size ..." << LOG_END;
#endif
    if(!streams){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Stream list size failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid streams" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!size){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Stream list size failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid size pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_streams = static_cast<ed247::SmartListStreams*>(streams);
        *size = ed247_streams->size();
    }
    LIBED247_CATCH("Stream list size")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Stream list size success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_signal_list_next(
    ed247_signal_list_t signals,
    ed247_signal_t *signal)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Signal list next ..." << LOG_END;
#endif
    if(!signals){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Signal list next failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid signals" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!signal){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Signal list next failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty signal pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *signal = nullptr;
    try{
        auto ed247_signals = (ed247::SmartListSignals*)(signals);
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "# Size [" << ed247_signals->size() << "]" << LOG_END;
#endif
        auto && next = ed247_signals->next_ok();
        *signal = next ? next->get() : nullptr;
    }
    LIBED247_CATCH("Signal list next")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Signal list next success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_signal_list_free(
    ed247_signal_list_t signals)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Signal list free ..." << LOG_END;
#endif
    if(!signals){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Signal list free failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid signals" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_signals = static_cast<ed247::SmartListSignals*>(signals);
        if(!ed247_signals->managed())
            delete ed247_signals;
    }
    LIBED247_CATCH("Signal list free")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Signal list free success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_signal_list_size(
    ed247_signal_list_t signals,
    size_t * size)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Signal list size ..." << LOG_END;
#endif
    if(!signals){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Signal list size failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid signals" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!size){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Signal list size failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid size pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_signals = static_cast<ed247::SmartListSignals*>(signals);
        *size = ed247_signals->size();
    }
    LIBED247_CATCH("Signal list size")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Signal list size success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_frame_list_next(
    ed247_frame_list_t frames,
    const ed247_frame_t ** frame)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Frame list next ..." << LOG_END;
#endif
    if(!frames){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Frame list next failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid frames" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!frame){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Frame list next failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty frame pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *frame = nullptr;
    try{
        ed247::SmartListFrames* ed247_frames = (ed247::SmartListFrames*)(frames);
        auto && next = ed247_frames->next_ok();
        *frame = next ? next->get() : nullptr;

    }
    LIBED247_CATCH("Frame list next")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Frame list next success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_frame_list_free(
    ed247_frame_list_t frames)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Frame list free ..." << LOG_END;
#endif
    if(!frames){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Frame list free failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid frames" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_frames = static_cast<ed247::SmartListFrames*>(frames);
        if(!ed247_frames->managed())
            delete ed247_frames;
    }
    LIBED247_CATCH("Frame list free")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Frame list free success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_frame_list_size(
    ed247_frame_list_t frames,
    size_t * size)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Frame list size ..." << LOG_END;
#endif
    if(!frames){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Frame list size failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid frames" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!size){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Frame list size failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid size pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_frames = static_cast<ed247::SmartListFrames*>(frames);
        *size = ed247_frames->size();
    }
    LIBED247_CATCH("Frame list size")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Frame list size success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

/******************
 * Initialization *
 ******************/

const char * ed247_get_implementation_name()
{
    return LIBED247_NAME;
}

const char * ed247_get_implementation_version()
{
    return LIBED247_VERSION;
}

ed247_status_t ed247_get_runtime_metrics(
    ed247_context_t context,
    const libed247_runtime_metrics_t ** metrics)
{
    if(!metrics){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Get metrics failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid metrics pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!context){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Get metrics failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid context pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        *metrics = NULL;
        auto ed247_context = static_cast<ed247::Context*>(context);
        *metrics = ed247_context->get_runtime_metrics();
    }
    LIBED247_CATCH("Get metrics")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Get metrics success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_set_log_level(
    ed247_log_level_t log_level)
{
    ed247::Logs::getInstance().setLogLevel(log_level);
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_get_log_level(
    ed247_log_level_t *log_level)
{
    *log_level = ed247::Logs::getInstance().getLogLevel();
    return ED247_STATUS_SUCCESS;
}

ed247_status_t libed247_register_set_simulation_time_ns_handler(
    libed247_set_simulation_time_ns_t handler)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Register simulation time handler ..." << LOG_END;
#endif
    if(!handler){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Register simulation time handler failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid handler" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        ed247::SimulationTimeHandler::get().set_handler(handler);
    }
    LIBED247_CATCH("Register simulation time handler")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Register simulation time handler success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t libed247_set_simulation_time_ns(ed247_time_sample_t time_sample)
{
#ifdef __linux__
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
    time_sample->epoch_s = (uint32_t)tp.tv_sec;
    time_sample->offset_ns = (uint32_t)((uint64_t)tp.tv_nsec/1000LL);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_sample->epoch_s = (uint32_t)tv.tv_sec;
    time_sample->offset_ns = (uint32_t)tv.tv_usec;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t libed247_update_time(
    ed247_time_sample_t time_sample,
    uint32_t epoch_s,
    uint32_t offset_ns)
{
    if(!time_sample) return ED247_STATUS_FAILURE;
    time_sample->epoch_s = epoch_s;
    time_sample->offset_ns = offset_ns;
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_load(
    const char *ed247_ecic_file_path,
    const libed247_configuration_t *libed247_configuration,
    ed247_context_t *context)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Load ..." << LOG_END;
#endif
    if(!context){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Load failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty context pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *context = nullptr;
    if(!ed247_ecic_file_path){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Load failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty file" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_context = ed247::Context::Builder::create(
            ed247_ecic_file_path,
            libed247_configuration ?
                *libed247_configuration : libed247_configuration_t(LIBED247_CONFIGURATION_DEFAULT));
        ed247_context->initialize();
        *context = ed247_context;
    }
    LIBED247_CATCH("Load")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Load success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

/***********
 * Finders *
 ***********/

ed247_status_t ed247_find_channels(
    ed247_context_t context,
    const char *regex_name,
    ed247_channel_list_t *channels)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Find channels ..." << LOG_END;
#endif
    if(!context){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Find channels failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid context" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!channels){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Find channels failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid channels pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_context = static_cast<ed247::Context*>(context);
        ed247::SmartListChannels * smart_channels = new ed247::SmartListChannels(ed247_context->getPoolChannels()->find(regex_name != nullptr ? std::string(regex_name) : std::string(".*")));
        smart_channels->set_managed(false);
        *channels = static_cast<ed247_channel_list_t>(smart_channels);
    }
    LIBED247_CATCH("Find channels")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Find channels success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_find_streams(
    ed247_context_t context,
    const char *regex_name,
    ed247_stream_list_t *streams)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Find streams ..." << LOG_END;
#endif
    if(!context){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Find streams failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid context" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!streams){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Find streams failed ..." << LOG_END;
#endif
        LOG_ERROR() << "Invalid streams pointer " << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_context = static_cast<ed247::Context*>(context);
        ed247::SmartListStreams * smart_streams = new ed247::SmartListStreams(ed247_context->getPoolStreams()->find(regex_name != nullptr ? std::string(regex_name) : std::string(".*")));
        *streams = static_cast<ed247_stream_list_t>(smart_streams);
    }
    LIBED247_CATCH("Find streams")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Find streams success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_find_channel_streams(
    ed247_channel_t channel,
    const char *regex_name,
    ed247_stream_list_t *streams)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Find channel streams ..." << LOG_END;
#endif
    if(!channel){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Find channel streams failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid channel" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!streams){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Find channel streams failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid streams pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_channel = (ed247::Channel*)(channel);
        ed247::SmartListStreams * smart_streams = new ed247::SmartListStreams(std::move(ed247_channel->find_streams(regex_name != nullptr ? std::string(regex_name) : std::string(".*"))));
        *streams = static_cast<ed247_stream_list_t>(smart_streams);
    }
    LIBED247_CATCH("Find channel streams")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Find channel streams success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_find_signals(
    ed247_context_t context,
    const char *regex_name,
    ed247_signal_list_t *signals)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Find signals ..." << LOG_END;
#endif
    if(!context){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Find signals failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid context" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!signals){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Find signals failed ..." << LOG_END;
#endif
        LOG_ERROR() << "Invalid signals pointer " << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_context = static_cast<ed247::Context*>(context);
        ed247::SmartListSignals * smart_signals = new ed247::SmartListSignals(ed247_context->getPoolSignals()->find(regex_name != nullptr ? std::string(regex_name) : std::string(".*")));
        smart_signals->reset();
        *signals = static_cast<ed247_signal_list_t>(smart_signals);
    }
    LIBED247_CATCH("Find signals")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Find signals success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_find_stream_signals(
    ed247_stream_t stream,
    const char *regex_name,
    ed247_signal_list_t *signals)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Find stream signals ..." << LOG_END;
#endif
    if(!stream){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Find stream signals failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid stream" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!signals){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Find stream signals failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid signals pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_stream = (ed247::BaseStream*)(stream);
        ed247::SmartListSignals * smart_signals = new ed247::SmartListSignals(std::move(ed247_stream->find_signals(regex_name != nullptr ? std::string(regex_name) : std::string(".*"))));
        *signals = static_cast<ed247_signal_list_t>(smart_signals);
    }
    LIBED247_CATCH("Find stream signals")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Find stream signals success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

/***********
 * Getters *
 ***********/

ed247_status_t ed247_component_get_info(
    ed247_context_t context,
    const ed247_component_info_t **info)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Get component info ..." << LOG_END;
#endif
    if(!context){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Get component info failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid context" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!info){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Get component info failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty info pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *info = nullptr;
    try{
        auto ed247_context = static_cast<ed247::Context*>(context);
        *info = &ed247_context->getRoot()->info;
    }
    LIBED247_CATCH("Get component info")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Get component info success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_component_get_streams(
    ed247_context_t context,
    ed247_stream_list_t *streams)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Get streams ..." << LOG_END;
#endif
    if(!context){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Get streams failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid context" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!streams){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Get streams failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty info pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *streams = nullptr;
    try{
        auto ed247_context = static_cast<ed247::Context*>(context);
        *streams = std::static_pointer_cast<ed247_internal_stream_list_t>(ed247_context->getPoolStreams()->streams()).get();
    }
    LIBED247_CATCH("Get streams info")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Get streams success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_component_get_channels(
    ed247_context_t context,
	ed247_channel_list_t *channels)
{
#ifdef LIBED247_VERBOSE_DEBUG
	LOG_DEBUG() << "## Get channels ..." << LOG_END;
#endif
    if(!context){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Get channels failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid context" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!channels){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Get channels failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty info pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
	*channels = nullptr;
	try{
        auto ed247_context = static_cast<ed247::Context*>(context);
		auto schannels = std::static_pointer_cast<ed247_internal_channel_list_t>(ed247_context->getPoolChannels()->channels());
		*channels = schannels.get();
		
	}
    LIBED247_CATCH("Get channels info")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Get channels success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;	
}

ed247_status_t ed247_channel_get_info(
    ed247_channel_t channel,
    const ed247_channel_info_t **info)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Get channel info ..." << LOG_END;
#endif
    if(!channel){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Get channel info failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid channel" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!info){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Get channel info failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty info pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *info = nullptr;
    try{
        auto ed247_channel = static_cast<ed247::Channel*>(channel);
        *info = &ed247_channel->get_configuration()->info;
    }
    LIBED247_CATCH("Get channel info")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Get channel info success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}


ed247_status_t ed247_channel_get_streams(
    ed247_channel_t channel,
    ed247_stream_list_t *streams)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Get streams ..." << LOG_END;
#endif
    if(!channel){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Get streams failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid channel" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!streams){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Get streams failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty info pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *streams = nullptr;
    try{
        auto ed247_channel = static_cast<ed247::Channel*>(channel);
        *streams = std::static_pointer_cast<ed247_internal_stream_list_t>(ed247_channel->sstreams()).get();
    }
    LIBED247_CATCH("Get streams info")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Get streams success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_get_info(
    ed247_stream_t stream,
    const ed247_stream_info_t **info)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Get stream info ..." << LOG_END;
#endif
    if(!stream){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Get stream info failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid stream" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!info){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Get stream info failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty info pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *info = nullptr;
    try{
        auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
        *info = &ed247_stream->get_configuration()->info;
    }
    LIBED247_CATCH("Get stream info")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Get stream info success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_get_channel(
    ed247_stream_t stream,
    ed247_channel_t *channel)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Get stream channel ..." << LOG_END;
#endif
    if(!stream){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Get stream channel failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid stream" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!channel){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Get stream channel failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty channel pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *channel = nullptr;
    try{
        auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
        auto && ed247_channel = ed247_stream->get_channel();
        *channel = ed247_channel ? ed247_channel.get() : nullptr;
    }
    LIBED247_CATCH("Get stream channel")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Get stream channel success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_signal_get_info(
    ed247_signal_t signal,
    const ed247_signal_info_t **info)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Get signal info ..." << LOG_END;
#endif
    if(!signal){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Get signal info failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid signal" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!info){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Get signal info failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty info pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *info = nullptr;
    try{
        auto ed247_signal = static_cast<ed247::BaseSignal*>(signal);
        *info = &ed247_signal->get_configuration()->info;
    }
    LIBED247_CATCH("Get signal info")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Get signal info success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_signal_get_stream(
    ed247_signal_t signal,
    ed247_stream_t *stream)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Get signal stream ..." << LOG_END;
#endif
    if(!signal){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Get signal stream failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid signal" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!stream){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Get signal stream failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty stream pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *stream = nullptr;
    try{
        auto ed247_signal = static_cast<ed247::BaseSignal*>(signal);
        auto && ed247_stream = ed247_signal->get_stream();
        *stream = ed247_stream ? ed247_stream.get() : nullptr;
    }
    LIBED247_CATCH("Get signal stream")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Get signal stream success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_get_signals(
    ed247_stream_t stream,
    ed247_signal_list_t *signals)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Stream get signals ..." << LOG_END;
#endif
    if(!stream){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Stream get signals failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid stream" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!signals){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Stream get signals failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty signals pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *signals = nullptr;
    try{
        auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
        auto ed247_signals = std::static_pointer_cast<ed247_internal_signal_list_t>(ed247_stream->signals());
        *signals = ed247_signals.get();
    }
    LIBED247_CATCH("Stream get signals")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Stream get signals success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_contains_signals(
    ed247_stream_t stream,
    uint8_t *yesno)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Stream contains signals ..." << LOG_END;
#endif
    if(!stream){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Stream contains signals failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid stream" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!yesno){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Stream contains signals failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty yesno pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *yesno = 0;
    try{
        auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
        *yesno = ed247_stream->signals()->size() != 0;
    }
    LIBED247_CATCH("Stream contains signals")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Stream contains signals success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

/**********************************
 * Stream & Signal sample helpers *
 **********************************/

ed247_status_t ed247_stream_allocate_sample(
    ed247_stream_t stream,
    void ** sample_data,
    size_t * sample_size)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Allocate stream sample ..." << LOG_END;
#endif
    if(!stream){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Allocate stream sample failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid stream" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!sample_data){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Allocate stream sample failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty sample_data pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!sample_size){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Allocate stream sample failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty sample_size pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *sample_data = nullptr;
    *sample_size = 0;
    try{
        auto ed247_stream = (ed247::BaseStream*)(stream);
        auto sample = ed247_stream->allocate_sample();
        *sample_data = sample->data();
        *sample_size = sample->capacity();
    }
    LIBED247_CATCH("Allocate stream sample")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Allocate stream sample success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_signal_allocate_sample(
    ed247_signal_t signal,
    void ** sample_data,
    size_t * sample_size)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Allocate signal sample ..." << LOG_END;
#endif
    if(!signal){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Allocate signal sample failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid signal" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!sample_data){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Allocate signal sample failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty sample_data pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!sample_size){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Allocate signal sample failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty sample_size pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *sample_data = nullptr;
    *sample_size = 0;
    try{
        auto ed247_signal = (ed247::BaseSignal*)(signal);
        auto sample = ed247_signal->allocate_sample();
        *sample_data = sample->data();
        *sample_size = sample->capacity();
    }
    LIBED247_CATCH("Allocate stream sample")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Allocate stream sample success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_get_assistant(
    ed247_stream_t stream,
    ed247_stream_assistant_t *assistant)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Get stream signal assistant ..." << LOG_END;
#endif
    if(!stream){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Get stream signal assistant failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid stream" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!assistant){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Get stream signal assistant failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty assistant pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *assistant = nullptr;
    try{
        auto ed247_stream = (ed247::BaseStream*)(stream);
        *assistant = ed247_stream->get_assistant().get();
        if(!*assistant){
#ifdef LIBED247_VERBOSE_DEBUG
            LOG_DEBUG() << "## Get stream signal assistant failed" << LOG_END;
#endif
            LOG_ERROR() << "No assistant available" << LOG_END;
            return ED247_STATUS_FAILURE;
        }
    }
    LIBED247_CATCH("Get stream signal assistant")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Get stream signal assistant success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_assistant_get_stream(
    ed247_stream_assistant_t assistant,
    ed247_stream_t *stream)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Get stream of assistant ..." << LOG_END;
#endif
    if(!assistant){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Get stream of assistant failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid assistant" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!stream){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Get stream of assistant failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty stream pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *stream = nullptr;
    try{
        auto ed247_assistant = static_cast<ed247::BaseStream::Assistant*>(assistant);
        *stream = ed247_assistant->get_stream().get();
    }
    LIBED247_CATCH("Get stream of assistant")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Get stream of assistant success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_assistant_write_signal(
    ed247_stream_assistant_t assistant,
    ed247_signal_t signal,
    const void *signal_sample_data,
    size_t signal_sample_size)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Write signal sample in assistant ..." << LOG_END;
#endif
    if(!assistant){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Write signal sample in assistant failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid assistant" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!signal){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Write signal sample in assistant failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid signal" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!signal_sample_data){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Write signal sample in assistant failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty signal sample data pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_assistant = static_cast<ed247::BaseStream::Assistant*>(assistant);
        auto && ed247_signal = static_cast<ed247::BaseSignal*>(signal)->shared_from_this();
        ed247_assistant->write(ed247_signal, signal_sample_data, signal_sample_size);
    }
    LIBED247_CATCH("Write signal sample in assistant")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Write signal sample in assistant" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_assistant_read_signal(
    ed247_stream_assistant_t assistant,
    ed247_signal_t signal,
    const void **signal_sample_data,
    size_t *signal_sample_size)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Read signal sample from assistant ..." << LOG_END;
#endif
    if(!assistant){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Read signal sample from assistant failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid assistant" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!signal){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Read signal sample from assistant failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid signal" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!signal_sample_data){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Read signal sample from assistant failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty signal sample data pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!signal_sample_size){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Read signal sample from assistant failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty signal sample size pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *signal_sample_data = nullptr;
    *signal_sample_size = 0;
    try{
        auto ed247_assistant = static_cast<ed247::BaseStream::Assistant*>(assistant);
        auto ed247_signal = static_cast<ed247::BaseSignal*>(signal)->shared_from_this();
        ed247_assistant->read(ed247_signal, signal_sample_data, signal_sample_size);
    }
    LIBED247_CATCH("Read signal sample from assistant")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Read signal sample from assistant" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_assistant_set_sample(
    ed247_stream_assistant_t assistant,
    const void *sample_data,
    size_t sample_size)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Update assistant stream sample ..." << LOG_END;
#endif
    if(!assistant){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Update assistant stream sample failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid assistant" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!sample_data){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Update assistant stream sample failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty sample data pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_assistant = static_cast<ed247::BaseStream::Assistant*>(assistant);
        ed247_assistant->read_sample(sample_data, sample_size);
    }
    LIBED247_CATCH("Update assistant stream sample")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Update assistant stream sample" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_assistant_get_sample(
    ed247_stream_assistant_t assistant,
    const void **sample_data,
    size_t *sample_size)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Retrieve assistant stream sample ..." << LOG_END;
#endif
    if(!assistant){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Retrieve assistant stream sample failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid assistant" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!sample_data){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Retrieve assistant stream sample failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty sample data pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!sample_size){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Retrieve assistant stream sample failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty sample size pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *sample_data = nullptr;
    *sample_size = 0;
    try{
        auto ed247_assistant = static_cast<ed247::BaseStream::Assistant*>(assistant);
        ed247_assistant->write_sample(sample_data, sample_size);
    }
    LIBED247_CATCH("Retrieve assistant stream sample")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Retrieve assistant stream sample" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

/************************
 * Read & write samples *
 ************************/

ed247_status_t ed247_stream_push_sample(
    ed247_stream_t stream,
    const void *sample_data,
    size_t sample_size,
    const ed247_timestamp_t *timestamp,
    bool *full)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Push stream sample ..." << LOG_END;
#endif
    if(!stream){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Push stream sample failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid stream" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!sample_data){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Push stream sample failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid sample data" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        static_cast<ed247::BaseStream*>(stream)->push_sample(sample_data, sample_size, timestamp, full);
    }
    LIBED247_CATCH("Push stream sample")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Push stream sample success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_push_samples(
    ed247_stream_t stream,
    const void *samples_data,
    const size_t *samples_size,
    size_t samples_number,
    const ed247_timestamp_t *timestamp,
    bool *full)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Push stream samples ..." << LOG_END;
#endif
    if(!stream){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Stream push samples failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid stream" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!samples_data){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Push stream samples failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid sample data" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!samples_size){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Push stream samples failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid sample size" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
        size_t sum_size = 0;
        for(uint32_t i = 0 ; i < samples_number ; i++){
            ed247_stream->push_sample((char*)samples_data+sum_size, samples_size[i], timestamp, full);
            sum_size += samples_size[i];
        }
    }
    LIBED247_CATCH("Push stream samples")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Push stream samples success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_assistant_push_sample(
    ed247_stream_assistant_t assistant,
    const ed247_timestamp_t *timestamp,
    bool *full)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Push stream sample with assistant ..." << LOG_END;
#endif
    if(!assistant){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Push stream sample with assistant failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid stream" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_assistant = static_cast<ed247::BaseStream::Assistant*>(assistant);
        ed247_assistant->push(timestamp, full);
    }
    LIBED247_CATCH("Push stream sample with assistant")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Push stream sample with assistant success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_pop_sample(
    ed247_stream_t stream,
    const void **sample_data,
    size_t *sample_size,
    const ed247_timestamp_t **data_timestamp,
    const ed247_timestamp_t **recv_timestamp,
    const ed247_sample_info_t **info,
    bool *empty)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Pop stream sample ..." << LOG_END;
#endif
    if(!stream){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Pop stream sample failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid stream" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(sample_data == nullptr){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Pop stream sample failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty sample_data pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(sample_size == nullptr){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Pop stream sample failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty sample_size pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *sample_data = nullptr;
    *sample_size = 0;
    try{
        auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
        if(ed247_stream->recv_stack().size() == 0) return ED247_STATUS_NODATA;
        const auto & sample = ed247_stream->pop_sample(empty);
        *sample_data = sample->data_const();
        *sample_size = sample->size();
        if(data_timestamp) *data_timestamp = sample->data_timestamp();
        if(recv_timestamp) *recv_timestamp = sample->recv_timestamp();
        if(info) *info = sample->info();
    }
    LIBED247_CATCH("Pop stream sample")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Pop stream sample success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_assistant_pop_sample(
    ed247_stream_assistant_t assistant,
    const ed247_timestamp_t **data_timestamp,
    const ed247_timestamp_t **recv_timestamp,
    const ed247_sample_info_t **info,
    bool *empty)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Pop stream sample with assistant ..." << LOG_END;
#endif
    if(!assistant){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Pop stream sample with assistant failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid assistant" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_assistant = static_cast<ed247::BaseStream::Assistant*>(assistant);
        if(ed247_assistant->get_stream()->recv_stack().size() == 0) return ED247_STATUS_NODATA;
        ed247_assistant->pop(data_timestamp, recv_timestamp, info, empty);
    }
    LIBED247_CATCH("Pop stream sample with assistant")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Pop stream sample with assistant success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_samples_number(
    ed247_stream_t stream,
    ed247_direction_t direction,
    size_t *size)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Stream samples number ..." << LOG_END;
#endif
    if(!stream){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Stream samples number failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid stream" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!size){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Stream samples number failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty size pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(direction != ED247_DIRECTION_IN && direction != ED247_DIRECTION_OUT){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Stream samples number failed" << LOG_END;
#endif
        LOG_ERROR() << "Unknown direction" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *size = 0;
    try{
        auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
        *size = direction == ED247_DIRECTION_IN ? ed247_stream->recv_stack().size() : ed247_stream->send_stack().size();
    }
    LIBED247_CATCH("Stream samples number")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Stream samples number success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

/******************
 * Receive & Send *
 ******************/
ed247_status_t ed247_stream_register_recv_callback(
    ed247_stream_t stream,
    ed247_stream_recv_callback_t callback)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Stream register callback ..." << LOG_END;
#endif
    if(!stream){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Stream register callback failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid stream" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!callback){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Stream register callback failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid callback" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
        if(ed247_stream->register_callback(callback) == ED247_STATUS_FAILURE){
            LOG_WARNING() << "Cannot register callback in stream [" << ed247_stream->get_name() << "]" << LOG_END;
        }
    }
    LIBED247_CATCH("Stream register callback")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Stream register callback success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_unregister_recv_callback(
    ed247_stream_t stream,
    ed247_stream_recv_callback_t callback)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Stream unregister callback ..." << LOG_END;
#endif
    if(!stream){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Stream unregister callback failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid stream" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!callback){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Stream unregister callback failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid callback" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
        if(ed247_stream->unregister_callback(callback) == ED247_STATUS_FAILURE){
            LOG_WARNING() << "Cannot unregister callback in stream [" << ed247_stream->get_name() << "]" << LOG_END;
        }
    }
    LIBED247_CATCH("Stream unregister callback")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Stream unregister callback success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_streams_register_recv_callback(
    ed247_stream_list_t streams,
    ed247_stream_recv_callback_t callback)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Streams register callback ..." << LOG_END;
#endif
    if(!streams){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Streams register callback failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid streams" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!callback){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Streams register callback failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid callback" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_streams = static_cast<ed247::SmartListStreams*>(streams);
        ed247_streams->reset(); // Reset internal iterator
        std::shared_ptr<ed247::BaseStream> * next;
        while((next = ed247_streams->next_ok()) != nullptr){
            if((*next)->register_callback(callback) != ED247_STATUS_SUCCESS){
                LOG_WARNING() << "Cannot register callback in stream [" << (*next)->get_name() << "]" << LOG_END;
            }
        }
    }
    LIBED247_CATCH("Streams register callback")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Streams register callback success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_streams_unregister_recv_callback(
    ed247_stream_list_t streams,
    ed247_stream_recv_callback_t callback)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Streams unregister callback ..." << LOG_END;
#endif
    if(!streams){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Streams unregister callback failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid streams" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!callback){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Streams unregister callback failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid callback" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_streams = static_cast<ed247::SmartListStreams*>(streams);
        ed247_streams->reset(); // Reset internal iterator
        std::shared_ptr<ed247::BaseStream> * next;
        while((next = ed247_streams->next_ok()) != nullptr){
            if((*next)->unregister_callback(callback) != ED247_STATUS_SUCCESS){
                LOG_WARNING() << "Cannot unregister callback in stream [" << (*next)->get_name() << "]" << LOG_END;
            }
        }
    }
    LIBED247_CATCH("Streams unregister callback")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Streams unregister callback success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_register_recv_callback(
    ed247_context_t context,
    ed247_stream_recv_callback_t callback)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Register streams callback ..." << LOG_END;
#endif
    if(!context){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Register streams callback failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid context" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!callback){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Register streams callback failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid callback" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_context = static_cast<ed247::Context*>(context);
        auto ed247_streams = ed247_context->getPoolStreams()->streams();
        ed247_streams->reset(); // Reset internal iterator
        std::shared_ptr<ed247::BaseStream> * next;
        while((next = ed247_streams->next_ok()) != nullptr){
            if((*next)->register_callback(callback) != ED247_STATUS_SUCCESS){
                LOG_WARNING() << "Cannot register callback in stream [" << (*next)->get_name() << "]" << LOG_END;
            }
        }
    }
    LIBED247_CATCH("Register streams callback")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Register streams callback success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_unregister_recv_callback(
    ed247_context_t context,
    ed247_stream_recv_callback_t callback)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Unregister streams callback ..." << LOG_END;
#endif
    if(!context){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Unregister streams callback failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid context" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!callback){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Unregister streams callback failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid callback" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_context = static_cast<ed247::Context*>(context);
        auto ed247_streams = ed247_context->getPoolStreams()->streams();
        ed247_streams->reset(); // Reset internal iterator
        std::shared_ptr<ed247::BaseStream> * next;
        while((next = ed247_streams->next_ok()) != nullptr){
            if((*next)->unregister_callback(callback) != ED247_STATUS_SUCCESS){
                LOG_WARNING() << "Cannot unregister callback in stream [" << (*next)->get_name() << "]" << LOG_END;
            }
        }
    }
    LIBED247_CATCH("Unregister streams callback")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Unregister streams callback success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

extern LIBED247_EXPORT ed247_status_t ed247_register_com_recv_callback(
    ed247_context_t context,
    ed247_com_callback_t callback)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Register com recv callback ..." << LOG_END;
#endif
    if(!context){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Register com recv callback failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid context" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!callback){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Register com recv callback failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid callback" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_context = static_cast<ed247::Context*>(context);
        auto ed247_interfaces = std::static_pointer_cast<ed247::UdpSocket::Pool>(ed247_context->getPoolInterfaces());
        ed247_interfaces->register_recv_callback(callback);
    }
    LIBED247_CATCH("Register com recv callback")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Register com recv callback success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

extern LIBED247_EXPORT ed247_status_t ed247_unregister_com_recv_callback(
    ed247_context_t context,
    ed247_com_callback_t callback)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Unregister com recv callback ..." << LOG_END;
#endif
    if(!context){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Unregister com recv callback failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid context" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!callback){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Unregister com recv callback failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid callback" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_context = static_cast<ed247::Context*>(context);
        auto ed247_interfaces = std::static_pointer_cast<ed247::UdpSocket::Pool>(ed247_context->getPoolInterfaces());
        ed247_interfaces->unregister_recv_callback(callback);
    }
    LIBED247_CATCH("Unregister com recv callback")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Unregister com recv callback success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

extern LIBED247_EXPORT ed247_status_t ed247_register_com_send_callback(
    ed247_context_t context,
    ed247_com_callback_t callback)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Register com send callback ..." << LOG_END;
#endif
    if(!context){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Register com send callback failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid context" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!callback){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Register com send callback failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid callback" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_context = static_cast<ed247::Context*>(context);
        auto ed247_interfaces = std::static_pointer_cast<ed247::UdpSocket::Pool>(ed247_context->getPoolInterfaces());
        ed247_interfaces->register_send_callback(callback);
    }
    LIBED247_CATCH("Register com send callback")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Register com send callback success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

extern LIBED247_EXPORT ed247_status_t ed247_unregister_com_send_callback(
    ed247_context_t context,
    ed247_com_callback_t callback)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Unregister com send callback ..." << LOG_END;
#endif
    if(!context){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Unregister com send callback failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid context" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!callback){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Unregister com send callback failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid callback" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_context = static_cast<ed247::Context*>(context);
        auto ed247_interfaces = std::static_pointer_cast<ed247::UdpSocket::Pool>(ed247_context->getPoolInterfaces());
        ed247_interfaces->unregister_send_callback(callback);
    }
    LIBED247_CATCH("Unregister com send callback")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Unregister com send callback success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_wait_frame(
    ed247_context_t context,
    ed247_stream_list_t *streams,
    int32_t timeout_us)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Wait frame ..." << LOG_END;
#endif
    if(!context){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Wait frame failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid context" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!streams){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Wait frame failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty streams pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *streams = nullptr;
    try{
        auto ed247_context = static_cast<ed247::Context*>(context);
        auto ed247_status = ed247_context->wait_frame(timeout_us);
        if(ed247_status == ED247_STATUS_SUCCESS){
            *streams = ed247_context->active_streams().get();
        }
        return ed247_status;
    }
    LIBED247_CATCH("Wait frame")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Wait frame success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_wait_during(
    ed247_context_t context,
    ed247_stream_list_t *streams,
    int32_t duration_us)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Wait during ..." << LOG_END;
#endif
    if(!context){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Wait during failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid context" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!streams){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Wait during failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty streams pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *streams = nullptr;
    try{
        auto ed247_context = static_cast<ed247::Context*>(context);
        auto ed247_status = ed247_context->wait_during(duration_us);
        if(ed247_status == ED247_STATUS_SUCCESS){
            *streams = ed247_context->active_streams().get();
        }
        return ed247_status;
    }
    LIBED247_CATCH("Wait during")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Wait during success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_send_pushed_samples(
    ed247_context_t context)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Send ..." << LOG_END;
#endif
    if(!context){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Send failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid context" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_context = static_cast<ed247::Context*>(context);
        ed247_context->send_pushed_samples();
    }
    LIBED247_CATCH("Send")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Send success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

// ADVANCED

ed247_status_t ed247_frame_encode(
    ed247_context_t context,
    ed247_frame_list_t *frames)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Frame encode ..." << LOG_END;
#endif
    if(!context){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Frame encode failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid context" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(frames == nullptr){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Frame encode failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty frames pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    *frames = nullptr;
    try{
        auto ed247_context = static_cast<ed247::Context*>(context);
        ed247_context->encode_frames();
        *frames = ed247_context->active_frames().get();
    }
    LIBED247_CATCH("Frame encode")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Frame encode success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_frame_decode(
    ed247_channel_t channel,
    const void * data,
    size_t size)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Frame decode ..." << LOG_END;
#endif
    if(!channel){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Frame decode failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid channel" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    if(!data){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Frame decode failed" << LOG_END;
#endif
        LOG_ERROR() << "Empty data pointer" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_channel = static_cast<ed247::Channel*>(channel);
        ed247_channel->decode((const char*)data, size);
    }
    LIBED247_CATCH("Frame decode")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Frame decodesuccess" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}

/**********
 * Unload *
 **********/

ed247_status_t ed247_unload(
    ed247_context_t context)
{
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Unload ..." << LOG_END;
#endif
    if(!context){
#ifdef LIBED247_VERBOSE_DEBUG
        LOG_DEBUG() << "## Unload failed" << LOG_END;
#endif
        LOG_ERROR() << "Invalid context" << LOG_END;
        return ED247_STATUS_FAILURE;
    }
    try{
        auto ed247_context = static_cast<ed247::Context*>(context);
        delete(ed247_context);
        ed247_context=nullptr;
    }
    LIBED247_CATCH("Unload")
#ifdef LIBED247_VERBOSE_DEBUG
    LOG_DEBUG() << "## Unload success" << LOG_END;
#endif
    return ED247_STATUS_SUCCESS;
}