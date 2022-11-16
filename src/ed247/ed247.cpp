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
#include "ed247.h"
#include "ed247_internals.h"
#include "ed247_client_iterator.h"
#include "ed247_logs.h"
#include "ed247_context.h"
#include "ed247_stream_assistant.h"
#include <memory>


#ifdef _PRODUCT_VERSION
const char* _ed247_version = _PRODUCT_VERSION;
#else
const char* _ed247_version = "unversioned";
#endif

#ifdef _PRODUCT_NAME
const char* _ed247_name = _PRODUCT_NAME;
#else
const char* _ed247_name = "unnamed";
#endif

#define LIBED247_CATCH(topic)                                           \
  catch(std::exception &ex)                                             \
  {                                                                     \
    PRINT_ERROR(topic << " failed (FATAL ERROR) " << ex.what());        \
    return ED247_STATUS_FAILURE;                                        \
  }                                                                     \
  catch(...)                                                            \
  {                                                                     \
    PRINT_ERROR(topic << " failed (FATAL ERROR) (unknown exception)");  \
    return ED247_STATUS_FAILURE;                                        \
  }


/* =========================================================================
 * Client lists definition
 * ========================================================================= */
struct ed247_internal_channel_list_t {};
typedef ed247::client_list<ed247_internal_channel_list_t, ed247::Channel>           ed247_channel_clist_base_t;
typedef ed247::client_list_container<ed247_internal_channel_list_t, ed247::Channel> ed247_channel_clist_vector_t;


struct ed247_internal_stream_list_t {};
typedef ed247::client_list<ed247_internal_stream_list_t, ed247::Stream>           ed247_stream_clist_base_t;
typedef ed247::client_list_container<ed247_internal_stream_list_t, ed247::Stream> ed247_stream_clist_vector_t;

// stream list where get_next() filter out stream without data
// Note: this shall be removed from the interface (wait_frame & wait_during)
struct ed247_stream_clist_vector_with_data_t : public ed247_stream_clist_vector_t
{
  virtual ed247::Stream* get_next() override {
    ed247_stream_clist_vector_t::get_next();
    _iterator = std::find_if(_iterator,
                             _container->end(),
                             [](const ed247::stream_ptr_t& sp){ return sp->recv_stack().size() > 0; });
    return get_current();
  }
};

struct ed247_internal_signal_list_t {};
typedef ed247::client_list<ed247_internal_signal_list_t, ed247::signal>           ed247_signal_clist_base_t;
typedef ed247::client_list_container<ed247_internal_signal_list_t, ed247::signal> ed247_signal_clist_vector_t;




/* =========================================================================
 * Shared
 * ========================================================================= */
const char * ed247_get_implementation_name()
{
  return _ed247_name;
}

const char * ed247_get_implementation_version()
{
  return _ed247_version;
}

extern LIBED247_EXPORT ed247_status_t ed247_set_log(
  ed247_log_level_t log_level,
  const char *      log_filepath)
{
  ed247::log::get().reset(log_level, log_filepath);
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_set_log_level(
  ed247_log_level_t log_level)
{
  ed247::log::get().reset(log_level, nullptr);
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_get_log_level(
  ed247_log_level_t * log_level)
{
  *log_level = ed247::log::get().level();
  return ED247_STATUS_SUCCESS;
}

// Deprecated
const char * libed247_errors()
{
  return nullptr;
}

ed247_status_t ed247_free(
  void * data)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!data){
    PRINT_ERROR(__func__ << ": Invalid data");
    return ED247_STATUS_FAILURE;
  }
  try{
    free(data);
  }
  LIBED247_CATCH("Free");
  return ED247_STATUS_SUCCESS;
}


/* =========================================================================
 * ED247 Context - Init
 * ========================================================================= */
ed247_status_t ed247_load_file(
  const char *      ecic_file_path,
  ed247_context_t * context)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!context) {
    PRINT_ERROR(__func__ << ": Empty context pointer");
    return ED247_STATUS_FAILURE;
  }
  *context = nullptr;
  if(!ecic_file_path){
    PRINT_ERROR(__func__ << ": Empty file");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_context = ed247::Context::Builder::create_filepath(ecic_file_path);
    try {
      ed247_context->initialize();
      *context = ed247_context;
    } catch(...) {
      delete ed247_context;
      throw;
    }
  }
  LIBED247_CATCH("Load");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_load_content(
  const char *      ecic_file_content,
  ed247_context_t * context)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!context) {
    PRINT_ERROR(__func__ << ": Empty context pointer");
    return ED247_STATUS_FAILURE;
  }
  *context = nullptr;
  if(!ecic_file_content) {
    PRINT_ERROR(__func__ << ": Empty content");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_context = ed247::Context::Builder::create_content(ecic_file_content);
    try {
      ed247_context->initialize();
      *context = ed247_context;
    } catch(...) {
      delete ed247_context;
      throw;
    }
  }
  LIBED247_CATCH("Load content");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_unload(
  ed247_context_t context)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context){
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    delete(ed247_context);
    ed247_context=nullptr;
    MEMCHECK_FREED();
    MEMCHECK_RESET();
  }
  LIBED247_CATCH("Unload");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_get_runtime_metrics(
  ed247_context_t context,
  const libed247_runtime_metrics_t ** metrics)
{
  if(!metrics) {
    PRINT_ERROR(__func__ << ": Invalid metrics pointer");
    return ED247_STATUS_FAILURE;
  }
  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    *metrics = NULL;
    auto ed247_context = static_cast<ed247::Context*>(context);
    *metrics = ed247_context->get_runtime_metrics();
  }
  LIBED247_CATCH("Get metrics");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_component_set_user_data(
  ed247_context_t context,
  void *          user_data)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    ed247_context->set_user_data(user_data);
  }
  LIBED247_CATCH("Set user data");
  return ED247_STATUS_SUCCESS;	
}

ed247_status_t ed247_component_get_user_data(
  ed247_context_t context,
  void **         user_data)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!user_data) {
    PRINT_ERROR(__func__ << ": Empty user_data pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    *user_data = ed247_context->get_user_data();
  }
  LIBED247_CATCH("Get user data");
  return ED247_STATUS_SUCCESS;	
}

// Deprecated
ed247_status_t ed247_load(
  const char * ecic_file_path,
  void *       unused,
  ed247_context_t *context)
{
  return ed247_load_file(ecic_file_path, context);
}

/* =========================================================================
 * ED247 Context - General information
 * ========================================================================= */
const char* ed247_file_producer_get_identifier(ed247_context_t context)
{
  auto ed247_context = static_cast<ed247::Context*>(context);
  return ed247_context->getConfiguration()->_file_producer_identifier.c_str();
}

const char* ed247_file_producer_get_comment(ed247_context_t context)
{
  auto ed247_context = static_cast<ed247::Context*>(context);
  return ed247_context->getConfiguration()->_file_producer_comment.c_str();
}

const char* ed247_component_get_version(ed247_context_t context)
{
  auto ed247_context = static_cast<ed247::Context*>(context);
  return ed247_context->getConfiguration()->_version.c_str();
}

ed247_component_type_t ed247_component_get_type(ed247_context_t context)
{
  auto ed247_context = static_cast<ed247::Context*>(context);
  return ed247_context->getConfiguration()->_component_type;
}

const char* ed247_component_get_name(ed247_context_t context)
{
  auto ed247_context = static_cast<ed247::Context*>(context);
  return ed247_context->getConfiguration()->_name.c_str();
}

const char* ed247_component_get_comment(ed247_context_t context)
{
  auto ed247_context = static_cast<ed247::Context*>(context);
  return ed247_context->getConfiguration()->_comment.c_str();
}

ed247_uid_t ed247_component_get_identifier(ed247_context_t context)
{
  auto ed247_context = static_cast<ed247::Context*>(context);
  return ed247_context->getConfiguration()->_identifier;
}

ed247_standard_t ed247_component_get_standard_revision(ed247_context_t context)
{
  auto ed247_context = static_cast<ed247::Context*>(context);
  return ed247_context->getConfiguration()->_standard_revision;
}

/* =========================================================================
 * ED247 Context - Get Configuration
 * ========================================================================= */
ed247_status_t ed247_get_channel_list(
  ed247_context_t        context,
  ed247_channel_list_t * channels)
{
  static ed247_channel_clist_vector_t ed247_channel_list;    // To prevent malloc(), this function will always return the same list

  PRINT_DEBUG("function " << __func__ << "()");

  if(!channels) {
    PRINT_ERROR(__func__ << ": Empty info pointer");
    return ED247_STATUS_FAILURE;
  }

  *channels = nullptr;

  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }

  try{
    ed247::Context* ed247_context = static_cast<ed247::Context*>(context);
    ed247_channel_list.wrap(*(ed247_context->getPoolChannels()->channels().get()));
    *channels = &ed247_channel_list;
  }
  LIBED247_CATCH("Get channels info");
  return ED247_STATUS_SUCCESS;	
}

ed247_status_t ed247_find_channels(
  ed247_context_t        context,
  const char *           regex_name,
  ed247_channel_list_t * channels)
{
  static ed247_channel_clist_vector_t ed247_channel_list;    // To prevent malloc(), this function will always return the same list

  PRINT_DEBUG("function " << __func__ << "()");

  if(!channels) {
    PRINT_ERROR(__func__ << ": Invalid channels pointer");
    return ED247_STATUS_FAILURE;
  }

  *channels = nullptr;

  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }

  try{
    ed247::Context* ed247_context = static_cast<ed247::Context*>(context);
    ed247_channel_list.copy(ed247_context->getPoolChannels()->find(regex_name != nullptr ? std::string(regex_name) : std::string(".*")));
    *channels = &ed247_channel_list;
  }
  LIBED247_CATCH("Find channels");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_get_channel(
  ed247_context_t   context,
  const char *      name,
  ed247_channel_t * channel)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!name) {
    PRINT_ERROR(__func__ << ": Invalid name");
    return ED247_STATUS_FAILURE;
  }
  if(!channel) {
    PRINT_ERROR(__func__ << ": Invalid channel pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    auto && ed247_channel = ed247_context->getPoolChannels()->get(std::string(name));
    *channel = ed247_channel ? ed247_channel.get() : nullptr;
    if(*channel == nullptr) {
      PRINT_INFO("Cannot finnd channel '" << name << "'");
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Get channel");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_get_stream_list(
  ed247_context_t       context,
  ed247_stream_list_t * streams)
{
  static ed247_stream_clist_vector_t ed247_stream_list;    // To prevent malloc(), this function will always return the same list

  PRINT_DEBUG("function " << __func__ << "()");

  if(!streams) {
    PRINT_ERROR(__func__ << ": Empty info pointer");
    return ED247_STATUS_FAILURE;
  }

  *streams = nullptr;

  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }

  try{
    ed247::Context* ed247_context = static_cast<ed247::Context*>(context);
    ed247_stream_list.wrap(*(ed247_context->getPoolStreams()->streams().get()));
    *streams = &ed247_stream_list;
  }
  LIBED247_CATCH("Get streams info");
  return ED247_STATUS_SUCCESS;
}


ed247_status_t ed247_find_streams(
  ed247_context_t       context,
  const char *          regex_name,
  ed247_stream_list_t * streams)
{
  static ed247_stream_clist_vector_t ed247_stream_list;    // To prevent malloc(), this function will always return the same list

  PRINT_DEBUG("function " << __func__ << "()");
  if(!streams) {
    PRINT_ERROR(__func__ << ": Invalid streams pointer");
    return ED247_STATUS_FAILURE;
  }

  *streams = nullptr;

  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  try{
    ed247::Context* ed247_context = static_cast<ed247::Context*>(context);
    ed247_stream_list.copy(ed247_context->getPoolStreams()->find(regex_name != nullptr ? std::string(regex_name) : std::string(".*")));
    *streams = &ed247_stream_list;
  }
  LIBED247_CATCH("Find streams");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_get_stream(
  ed247_context_t  context,
  const char *     name,
  ed247_stream_t * stream)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!name) {
    PRINT_ERROR(__func__ << ": Invalid name");
    return ED247_STATUS_FAILURE;
  }
  if(!stream) {
    PRINT_ERROR(__func__ << ": Invalid stream pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    auto && ed247_stream = ed247_context->getPoolStreams()->get(std::string(name));
    *stream = ed247_stream ? ed247_stream.get() : nullptr;
    if(*stream == nullptr) {
      PRINT_INFO("Cannot find stream '" << name << "'");
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Get stream");
  return ED247_STATUS_SUCCESS;
}


ed247_status_t ed247_find_signals(
  ed247_context_t       context,
  const char *          regex_name,
  ed247_signal_list_t * signals)
{
  static ed247_signal_clist_vector_t ed247_signal_list;    // To prevent malloc(), this function will always return the same list

  PRINT_DEBUG("function " << __func__ << "()");

  if(!signals) {
    PRINT_ERROR(__func__ << ": Invalid signals pointer ");
    return ED247_STATUS_FAILURE;
  }

  *signals = nullptr;

  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  try{
    ed247::Context* ed247_context = static_cast<ed247::Context*>(context);
    ed247_signal_list.copy(ed247_context->getPoolSignals()->find(regex_name != nullptr ? std::string(regex_name) : std::string(".*")));
    *signals = &ed247_signal_list;
  }
  LIBED247_CATCH("Find signals");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_get_signal(
  ed247_context_t  context,
  const char *     name,
  ed247_signal_t * signal)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!name) {
    PRINT_ERROR(__func__ << ": Invalid name");
    return ED247_STATUS_FAILURE;
  }
  if(!signal) {
    PRINT_ERROR(__func__ << ": Invalid signal pointer ");
    return ED247_STATUS_FAILURE;
  }
  try{
    ed247::Context* ed247_context = static_cast<ed247::Context*>(context);
    *signal = ed247_context->getPoolSignals()->get(name).get();
    if(*signal == nullptr) {
      PRINT_INFO("Cannot find signal '" << name << "'");
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Get signal");
  return ED247_STATUS_SUCCESS;
}

// Deprecated
ed247_status_t ed247_component_get_channels(
  ed247_context_t        context,
  ed247_channel_list_t * channels)
{
  return ed247_get_channel_list(context, channels);
}
ed247_status_t ed247_component_get_streams(
  ed247_context_t       context,
  ed247_stream_list_t * streams)
{
  return ed247_get_stream_list(context, streams);
}

/* =========================================================================
 * ED247 Context - Receive and send
 * ========================================================================= */
ed247_status_t ed247_wait_frame(
  ed247_context_t       context,
  ed247_stream_list_t * streams,
  int32_t               timeout_us)
{
  static ed247_stream_clist_vector_with_data_t ed247_stream_list;    // To prevent malloc(), this function will always return the same list

  PRINT_DEBUG("function " << __func__ << "()");

  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }

  if(streams != nullptr) *streams = nullptr;

  try{
    ed247::Context* ed247_context = static_cast<ed247::Context*>(context);
    ed247_status_t ed247_status = ed247_context->wait_frame(timeout_us);
    if(streams != nullptr && ed247_status == ED247_STATUS_SUCCESS) {
      ed247_stream_list.wrap(*(ed247_context->getPoolStreams()->streams().get()));
      *streams = &ed247_stream_list;
    } else {
      PRINT_DEBUG("ed247_wait_frame status: " << ed247_status);
    }
    return ed247_status;
  }
  LIBED247_CATCH("ed247_wait_frame");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_wait_during(
  ed247_context_t       context,
  ed247_stream_list_t * streams,
  int32_t               duration_us)
{
  static ed247_stream_clist_vector_with_data_t ed247_stream_list;    // To prevent malloc(), this function will always return the same list

  PRINT_DEBUG("function " << __func__ << "()");

  if(!context){
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }

  if(streams != nullptr) *streams = nullptr;

  try{
    ed247::Context* ed247_context = static_cast<ed247::Context*>(context);
    ed247_status_t ed247_status = ed247_context->wait_during(duration_us);
    if(streams != nullptr && ed247_status == ED247_STATUS_SUCCESS) {
      ed247_stream_list.wrap(*(ed247_context->getPoolStreams()->streams().get()));
      *streams = &ed247_stream_list;
    } else {
      PRINT_DEBUG("ed247_wait_frame status: " << ed247_status);
    }
    return ed247_status;
  }
  LIBED247_CATCH("Wait during");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_send_pushed_samples(
  ed247_context_t context)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context){
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    ed247_context->send_pushed_samples();
  }
  LIBED247_CATCH("Send");
  return ED247_STATUS_SUCCESS;
}


/* =========================================================================
 * ED247 Context - Callbacks
 * ========================================================================= */
ed247_status_t ed247_stream_register_recv_callback(
  ed247_context_t              context,
  ed247_stream_t               stream,
  ed247_stream_recv_callback_t callback)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context){
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!stream){
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!callback){
    PRINT_ERROR(__func__ << ": Invalid callback");
    return ED247_STATUS_FAILURE;
  }
  ed247_status_t status = ED247_STATUS_SUCCESS;
  try{
    auto ed247_stream = static_cast<ed247::Stream*>(stream);
    if(ed247_stream->register_callback(context, callback) == ED247_STATUS_FAILURE){
      status = ED247_STATUS_FAILURE;
      PRINT_WARNING("Cannot register callback in stream [" << ed247_stream->get_name() << "]");
    }
  }
  LIBED247_CATCH("Stream register callback");
  return status;
}

ed247_status_t ed247_stream_unregister_recv_callback(
  ed247_context_t              context,
  ed247_stream_t               stream,
  ed247_stream_recv_callback_t callback)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context){
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!stream){
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!callback){
    PRINT_ERROR(__func__ << ": Invalid callback");
    return ED247_STATUS_FAILURE;
  }
  ed247_status_t status = ED247_STATUS_SUCCESS;
  try{
    auto ed247_stream = static_cast<ed247::Stream*>(stream);
    if(ed247_stream->unregister_callback(context, callback) == ED247_STATUS_FAILURE){
      status = ED247_STATUS_FAILURE;
      PRINT_WARNING("Cannot unregister callback in stream [" << ed247_stream->get_name() << "]");
    }
  }
  LIBED247_CATCH("Stream unregister callback");
  return status;
}

ed247_status_t ed247_streams_register_recv_callback(
  ed247_context_t              context,
  ed247_stream_list_t          streams,
  ed247_stream_recv_callback_t callback)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context){
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!streams){
    PRINT_ERROR(__func__ << ": Invalid streams");
    return ED247_STATUS_FAILURE;
  }
  if(!callback){
    PRINT_ERROR(__func__ << ": Invalid callback");
    return ED247_STATUS_FAILURE;
  }
  ed247_status_t status = ED247_STATUS_SUCCESS;
  try{
    auto list = static_cast<ed247_stream_clist_base_t*>(streams);
    list->reset_iterator();
    while (ed247::Stream* stream = list->get_next()) {
      if(stream->register_callback(context, callback) != ED247_STATUS_SUCCESS) {
        status = ED247_STATUS_FAILURE;
        PRINT_WARNING("Cannot register callback in stream [" << stream->get_name() << "]");
      }
    }
  }
  LIBED247_CATCH("Streams register callback");
  return status;
}

ed247_status_t ed247_streams_unregister_recv_callback(
  ed247_context_t              context,
  ed247_stream_list_t          streams,
  ed247_stream_recv_callback_t callback)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context){
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!streams){
    PRINT_ERROR(__func__ << ": Invalid streams");
    return ED247_STATUS_FAILURE;
  }
  if(!callback){
    PRINT_ERROR(__func__ << ": Invalid callback");
    return ED247_STATUS_FAILURE;
  }
  ed247_status_t status = ED247_STATUS_SUCCESS;
  try{
    auto list = static_cast<ed247_stream_clist_base_t*>(streams);
    list->reset_iterator();
    while (ed247::Stream* stream = list->get_next()) {
      if(stream->unregister_callback(context, callback) != ED247_STATUS_SUCCESS) {
        status = ED247_STATUS_FAILURE;
        PRINT_WARNING("Cannot unregister callback in stream [" << stream->get_name() << "]");
      }
    }
  }
  LIBED247_CATCH("Streams unregister callback");
  return status;
}

ed247_status_t ed247_register_recv_callback(
  ed247_context_t              context,
  ed247_stream_recv_callback_t callback)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context){
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!callback){
    PRINT_ERROR(__func__ << ": Invalid callback");
    return ED247_STATUS_FAILURE;
  }
  ed247_status_t status = ED247_STATUS_SUCCESS;
  try{
    ed247::Context* ed247_context = static_cast<ed247::Context*>(context);
    std::shared_ptr<ed247::stream_list_t> ed247_streams = ed247_context->getPoolStreams()->streams();
    for (auto istream = ed247_streams->begin(); istream != ed247_streams->end(); istream++) {
      if((*istream)->register_callback(context, callback) != ED247_STATUS_SUCCESS){
        status = ED247_STATUS_FAILURE;
        PRINT_WARNING("Cannot register callback in stream [" << (*istream)->get_name() << "]");
      }
    }
  }
  LIBED247_CATCH("Register streams callback");
  return status;
}

ed247_status_t ed247_unregister_recv_callback(
  ed247_context_t              context,
  ed247_stream_recv_callback_t callback)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!context){
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!callback){
    PRINT_ERROR(__func__ << ": Invalid callback");
    return ED247_STATUS_FAILURE;
  }
  ed247_status_t status = ED247_STATUS_SUCCESS;
  try{
    ed247::Context* ed247_context = static_cast<ed247::Context*>(context);
    std::shared_ptr<ed247::stream_list_t> ed247_streams = ed247_context->getPoolStreams()->streams();
    for (auto istream = ed247_streams->begin(); istream != ed247_streams->end(); istream++) {
      if((*istream)->unregister_callback(context, callback) != ED247_STATUS_SUCCESS){
        status = ED247_STATUS_FAILURE;
        PRINT_WARNING("Cannot unregister callback in stream [" << (*istream)->get_name() << "]");
      }
    }
  }
  LIBED247_CATCH("Unregister streams callback");
  return status;
}

/* =========================================================================
 * Channel
 * ========================================================================= */
const char* ed247_channel_get_name(ed247_channel_t channel)
{
  auto ed247_channel = static_cast<ed247::Channel*>(channel);
  return ed247_channel->get_configuration()->_name.c_str();
}

const char* ed247_channel_get_comment(ed247_channel_t channel)
{
  auto ed247_channel = static_cast<ed247::Channel*>(channel);
  return ed247_channel->get_configuration()->_comment.c_str();
}

ed247_standard_t ed247_channel_get_frame_standard_revision(ed247_channel_t channel)
{
  auto ed247_channel = static_cast<ed247::Channel*>(channel);
  return ed247_channel->get_configuration()->_frame_standard_revision;
}

ed247_status_t ed247_channel_get_stream_list(
  ed247_channel_t       channel,
  ed247_stream_list_t * streams)
{
  static ed247_stream_clist_vector_t ed247_stream_list;    // To prevent malloc(), this function will always return the same list

  PRINT_DEBUG("function " << __func__ << "()");

  if(!streams) {
    PRINT_ERROR(__func__ << ": Empty info pointer");
    return ED247_STATUS_FAILURE;
  }

  *streams = nullptr;

  if(!channel) {
    PRINT_ERROR(__func__ << ": Invalid channel");
    return ED247_STATUS_FAILURE;
  }

  try{
    ed247::Channel* ed247_channel = static_cast<ed247::Channel*>(channel);
    ed247_stream_list.wrap(*(ed247_channel->sstreams().get()));
    *streams = &ed247_stream_list;
  }
  LIBED247_CATCH("Get streams info");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_channel_find_streams(
  ed247_channel_t       channel,
  const char *          regex_name,
  ed247_stream_list_t * streams)
{
  static ed247_stream_clist_vector_t ed247_stream_list;    // To prevent malloc(), this function will always return the same list

  PRINT_DEBUG("function " << __func__ << "()");
  if(!streams) {
    PRINT_ERROR(__func__ << ": Invalid streams pointer");
    return ED247_STATUS_FAILURE;
  }

  *streams = nullptr;

  if(!channel) {
    PRINT_ERROR(__func__ << ": Invalid channel");
    return ED247_STATUS_FAILURE;
  }
  try{
    ed247::Channel* ed247_channel = (ed247::Channel*)(channel);
    ed247_stream_list.copy(ed247_channel->find_streams(regex_name != nullptr ? std::string(regex_name) : std::string(".*")));
    *streams = &ed247_stream_list;
  }
  LIBED247_CATCH("Find channel streams");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_channel_get_stream(
  ed247_channel_t  channel,
  const char *     name,
  ed247_stream_t * stream)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!channel) {
    PRINT_ERROR(__func__ << ": Invalid channel");
    return ED247_STATUS_FAILURE;
  }
  if(!name) {
    PRINT_ERROR(__func__ << ": Invalid name");
    return ED247_STATUS_FAILURE;
  }
  if(!stream){
    PRINT_ERROR(__func__ << ": Invalid stream pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_channel = (ed247::Channel*)(channel);
    auto && ed247_stream = ed247_channel->get_stream(std::string(name));
    *stream = ed247_stream ? ed247_stream.get() : nullptr;
    if(*stream == nullptr) {
      PRINT_INFO("Cannot find channel '" << name << "'");
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Get channel stream");
  return ED247_STATUS_SUCCESS;
}


ed247_status_t ed247_channel_set_user_data(
  ed247_channel_t channel,
  void *          user_data)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!channel) {
    PRINT_ERROR(__func__ << ": nvalid channel");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_channel = static_cast<ed247::Channel*>(channel);
    ed247_channel->set_user_data(user_data);
  }
  LIBED247_CATCH("Set channel user data");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_channel_get_user_data(
  ed247_channel_t channel,
  void **         user_data)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!channel) {
    PRINT_ERROR(__func__ << ": Invalid channel");
    return ED247_STATUS_FAILURE;
  }
  if(!user_data) {
    PRINT_ERROR(__func__ << ": Invalid user_data");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_channel = static_cast<ed247::Channel*>(channel);
    ed247_channel->get_user_data(user_data);
  }
  LIBED247_CATCH("Get channel user data");
  return ED247_STATUS_SUCCESS;
}

// Deprecated
ed247_status_t ed247_channel_get_streams(
  ed247_channel_t       channel,
  ed247_stream_list_t * streams)
{
  return ed247_channel_get_stream_list(channel, streams);
}
ed247_status_t ed247_find_channel_streams(
  ed247_channel_t       channel,
  const char *          regex_name,
  ed247_stream_list_t * streams)
{
  return ed247_channel_find_streams(channel, regex_name, streams);
}
ed247_status_t ed247_get_channel_stream(
  ed247_channel_t  channel,
  const char *     name,
  ed247_stream_t * stream)
{
  return ed247_channel_get_stream(channel, name, stream);
}

/* =========================================================================
 * Channel - List
 * ========================================================================= */
ed247_status_t ed247_channel_list_size(
  ed247_channel_list_t channels,
  uint32_t *           size)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!channels) {
    PRINT_ERROR(__func__ << ": invalid channels list.");
    return ED247_STATUS_FAILURE;
  }
  if(!size) {
    PRINT_ERROR(__func__ << ": Invalid size pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    *size = static_cast<ed247_channel_clist_base_t*>(channels)->size();
  }
  LIBED247_CATCH("Channel list size");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_channel_list_next(
  ed247_channel_list_t channels,
  ed247_channel_t *    channel)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!channels) {
    PRINT_ERROR(__func__ << ": invalid channels list.");
    return ED247_STATUS_FAILURE;
  }
  if(!channel) {
    PRINT_ERROR(__func__ << ": channel pointer is NULL.");
    return ED247_STATUS_FAILURE;
  }
  *channel = nullptr;
  try{
    *channel = static_cast<ed247_channel_clist_base_t*>(channels)->get_next();
  }
  LIBED247_CATCH("Channel list next");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_channel_list_free(
  ed247_channel_list_t channels)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!channels) {
    // Nothing to do
    return ED247_STATUS_SUCCESS;
  }
  try{
    static_cast<ed247_channel_clist_base_t*>(channels)->free();
  }
  LIBED247_CATCH("Channel list free");
  return ED247_STATUS_SUCCESS;
}


/* =========================================================================
 * Stream
 * ========================================================================= */
const char* ed247_stream_get_name(ed247_stream_t stream)
{
  auto ed247_stream = static_cast<ed247::Stream*>(stream);
  return ed247_stream->get_configuration()->_name.c_str();
}

ed247_direction_t ed247_stream_get_direction(ed247_stream_t stream)
{
  auto ed247_stream = static_cast<ed247::Stream*>(stream);
  return ed247_stream->get_configuration()->_direction;
}

ed247_stream_type_t ed247_stream_get_type(ed247_stream_t stream)
{
  auto ed247_stream = static_cast<ed247::Stream*>(stream);
  return ed247_stream->get_configuration()->_type;
}

const char* ed247_stream_get_comment(ed247_stream_t stream)
{
  auto ed247_stream = static_cast<ed247::Stream*>(stream);
  return ed247_stream->get_configuration()->_comment.c_str();
}

const char* ed247_stream_get_icd(ed247_stream_t stream)
{
  auto ed247_stream = static_cast<ed247::Stream*>(stream);
  return ed247_stream->get_configuration()->_icd.c_str();
}

ed247_uid_t ed247_stream_get_uid(ed247_stream_t stream)
{
  auto ed247_stream = static_cast<ed247::Stream*>(stream);
  return ed247_stream->get_configuration()->_uid;
}

uint32_t ed247_stream_get_sample_max_number(ed247_stream_t stream)
{
  auto ed247_stream = static_cast<ed247::Stream*>(stream);
  return ed247_stream->get_configuration()->_sample_max_number;
}

uint32_t ed247_stream_get_sample_max_size_bytes(ed247_stream_t stream)
{
  auto ed247_stream = static_cast<ed247::Stream*>(stream);
  return ed247_stream->get_configuration()->_sample_max_size_bytes;
}

uint32_t ed247_stream_get_sampling_period_us(ed247_stream_t stream)
{
  auto ed247_stream = static_cast<ed247::Stream*>(stream);
  if (ed247_stream->is_signal_based() != 0) {
    return ((ed247::xml::StreamSignals*)ed247_stream->get_configuration())->_sampling_period_us;
  } else {
    return 0;
  }
}

ed247_status_t ed247_stream_has_signals(
  ed247_stream_t stream,
  uint8_t *      yesno)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!stream){
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!yesno){
    PRINT_ERROR(__func__ << ": Empty yesno pointer");
    return ED247_STATUS_FAILURE;
  }
  *yesno = 0;
  try{
    auto ed247_stream = static_cast<ed247::Stream*>(stream);
    *yesno = ed247_stream->is_signal_based();
  }
  LIBED247_CATCH("Stream contains signals");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_get_signal_list(
  ed247_stream_t        stream,
  ed247_signal_list_t * signals)
{
  static ed247_signal_clist_vector_t ed247_signal_list;    // To prevent malloc(), this function will always return the same list

  PRINT_DEBUG("function " << __func__ << "()");
  if(!signals){
    PRINT_ERROR(__func__ << ": Empty signals pointer");
    return ED247_STATUS_FAILURE;
  }

  *signals = nullptr;

  if(!stream){
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }

  try{
    ed247::Stream* ed247_stream = static_cast<ed247::Stream*>(stream);
    ed247_signal_list.wrap(*(ed247_stream->signals().get()));
    *signals = &ed247_signal_list;
  }
  LIBED247_CATCH("Stream get signals");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_find_signals(
  ed247_stream_t        stream,
  const char *          regex_name,
  ed247_signal_list_t * signals)
{
  static ed247_signal_clist_vector_t ed247_signal_list;    // To prevent malloc(), this function will always return the same list

  PRINT_DEBUG("function " << __func__ << "()");

  if(!signals){
    PRINT_ERROR(__func__ << ": Invalid signals pointer");
    return ED247_STATUS_FAILURE;
  }

  *signals = nullptr;

  if(!stream) {
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  try{
    ed247::Stream* ed247_stream = (ed247::Stream*)(stream);
    ed247_signal_list.copy(ed247_stream->find_signals(regex_name != nullptr ? std::string(regex_name) : std::string(".*")));
    *signals = &ed247_signal_list;
  }
  LIBED247_CATCH("Find stream signals");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_get_signal(
  ed247_stream_t   stream,
  const char *     name,
  ed247_signal_t * signal)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!stream) {
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!name) {
    PRINT_ERROR(__func__ << ": Invalid name");
    return ED247_STATUS_FAILURE;
  }
  if(!signal) {
    PRINT_ERROR(__func__ << ": Invalid signal pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_stream = (ed247::Stream*)(stream);
    auto && ed247_signal = ed247_stream->get_signal(std::string(name));
    *signal = ed247_signal ? ed247_signal.get() : nullptr;
    if(*signal == nullptr) {
      PRINT_INFO("Cannot find signal '" << name << "'");
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Get stream signal");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_get_channel(
  ed247_stream_t    stream,
  ed247_channel_t * channel)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!stream){
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!channel){
    PRINT_ERROR(__func__ << ": Empty channel pointer");
    return ED247_STATUS_FAILURE;
  }
  *channel = nullptr;
  try{
    auto ed247_stream = static_cast<ed247::Stream*>(stream);
    *channel = ed247_stream->get_api_channel();
  }
  LIBED247_CATCH("Get stream channel");
  return ED247_STATUS_SUCCESS;
}


ed247_status_t ed247_stream_set_user_data(
  ed247_stream_t stream,
  void *         user_data)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!stream) {
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_stream = static_cast<ed247::Stream*>(stream);
    ed247_stream->set_user_data(user_data);
  }
  LIBED247_CATCH("Set stream user data");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_get_user_data(
  ed247_stream_t stream,
  void **        user_data)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!stream) {
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!user_data){
    PRINT_ERROR(__func__ << ": Invalid user_data");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_stream = static_cast<ed247::Stream*>(stream);
    ed247_stream->get_user_data(user_data);
  }
  LIBED247_CATCH("Get stream user data");
  return ED247_STATUS_SUCCESS;
}

// Deprecated
ed247_status_t ed247_stream_contains_signals(
  ed247_stream_t stream,
  uint8_t *      yesno)
{
  return ed247_stream_has_signals(stream, yesno);
}
ed247_status_t ed247_stream_get_signals(
  ed247_stream_t        stream,
  ed247_signal_list_t * signals)
{
  return ed247_stream_get_signal_list(stream, signals);
}
ed247_status_t ed247_find_stream_signals(
  ed247_stream_t        stream,
  const char *          regex_name,
  ed247_signal_list_t * signals)
{
  return ed247_stream_find_signals(stream, regex_name, signals);
}
ed247_status_t ed247_get_stream_signal(
  ed247_stream_t   stream,
  const char *     name,
  ed247_signal_t * signal)
{
  return ed247_stream_get_signal(stream, name, signal);
}

/* =========================================================================
 * Stream - Read & Write
 * ========================================================================= */
ed247_status_t ed247_stream_get_assistant(
  ed247_stream_t             stream,
  ed247_stream_assistant_t * assistant)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!stream){
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!assistant){
    PRINT_ERROR(__func__ << ": Empty assistant pointer");
    return ED247_STATUS_FAILURE;
  }
  *assistant = nullptr;
  try{
    auto ed247_stream = (ed247::Stream*)(stream);
    *assistant = ed247_stream->get_api_assistant();
    if(*assistant == nullptr) {
      PRINT_WARNING("Stream '" << ed247_stream->get_name() << "' do not have a valid stream signal assistant.");
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Get stream signal assistant");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_allocate_sample(
  ed247_stream_t stream,
  void **        sample_data,
  uint32_t *     sample_size)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!stream){
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!sample_data){
    PRINT_ERROR(__func__ << ": Empty sample_data pointer");
    return ED247_STATUS_FAILURE;
  }
  if(!sample_size){
    PRINT_ERROR(__func__ << ": Empty sample_size pointer");
    return ED247_STATUS_FAILURE;
  }
  *sample_data = nullptr;
  *sample_size = 0;
  try{
    auto ed247_stream = (ed247::Stream*)(stream);
    *sample_size = ed247_stream->get_configuration()->_sample_max_size_bytes;
    *sample_data = malloc(*sample_size);
  }
  LIBED247_CATCH("Allocate stream sample");
  return ED247_STATUS_SUCCESS;
}

extern LIBED247_EXPORT ed247_status_t ed247_stream_free_sample(
  void* sample_data)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!sample_data){
    PRINT_ERROR(__func__ << ": Invalid sample_data");
    return ED247_STATUS_FAILURE;
  }
  try{
    free(sample_data);
  }
  LIBED247_CATCH("Free");
  return ED247_STATUS_SUCCESS;
}


ed247_status_t ed247_stream_samples_number(
  ed247_stream_t    stream,
  ed247_direction_t direction,
  uint32_t *        size)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!stream){
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!size){
    PRINT_ERROR(__func__ << ": Empty size pointer");
    return ED247_STATUS_FAILURE;
  }
  if(direction != ED247_DIRECTION_IN && direction != ED247_DIRECTION_OUT){
    PRINT_ERROR(__func__ << ": Unknown direction");
    return ED247_STATUS_FAILURE;
  }
  *size = 0;
  try{
    auto ed247_stream = static_cast<ed247::Stream*>(stream);
    *size = direction == ED247_DIRECTION_IN ? ed247_stream->recv_stack().size() : ed247_stream->send_stack().size();
  }
  LIBED247_CATCH("Stream samples number");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_push_sample(
  ed247_stream_t            stream,
  const void *              sample_data,
  uint32_t                  sample_size,
  const ed247_timestamp_t * timestamp,
  bool *                    full)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!stream){
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!sample_data){
    PRINT_ERROR(__func__ << ": Invalid sample data");
    return ED247_STATUS_FAILURE;
  }
  try{
    if (static_cast<ed247::Stream*>(stream)->push_sample(sample_data, sample_size, timestamp, full) == false) {
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Push stream sample");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_push_samples(
  ed247_stream_t            stream,
  const void *              samples_data,
  const uint32_t *          samples_size,
  uint32_t                  samples_number,
  const ed247_timestamp_t * timestamp,
  bool *                    full)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!stream){
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!samples_data){
    PRINT_ERROR(__func__ << ": Invalid sample data");
    return ED247_STATUS_FAILURE;
  }
  if(!samples_size){
    PRINT_ERROR(__func__ << ": Invalid sample size");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_stream = static_cast<ed247::Stream*>(stream);
    uint32_t sum_size = 0;
    for(uint32_t i = 0 ; i < samples_number ; i++){
      if (ed247_stream->push_sample((const char*)samples_data+sum_size, samples_size[i], timestamp, full) == false) {
        return ED247_STATUS_FAILURE;
      }
      sum_size += samples_size[i];
    }
  }
  LIBED247_CATCH("Push stream samples");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_pop_sample(
  ed247_stream_t                  stream,
  const void **                   sample_data,
  uint32_t *                      sample_size,
  const ed247_timestamp_t **      data_timestamp,
  const ed247_timestamp_t **      recv_timestamp,
  const ed247_sample_details_t ** sample_details,
  bool *                          empty)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!stream){
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(sample_data == nullptr){
    PRINT_ERROR(__func__ << ": Empty sample_data pointer");
    return ED247_STATUS_FAILURE;
  }
  if(sample_size == nullptr){
    PRINT_ERROR(__func__ << ": Empty sample_size pointer");
    return ED247_STATUS_FAILURE;
  }
  *sample_data = nullptr;
  *sample_size = 0;
  try{
    auto ed247_stream = static_cast<ed247::Stream*>(stream);
    if ((ed247_stream->get_configuration()->_direction & ED247_DIRECTION_IN) == 0) {
      PRINT_ERROR("Stream '" << ed247_stream->get_name() << "': Cannot pop from a non-input stream");
      return ED247_STATUS_FAILURE;
    }
    if (ed247_stream->recv_stack().size() == 0) {
      PRINT_CRAZY("Stream '" << ed247_stream->get_name() << "': no data received.");
      return ED247_STATUS_NODATA;
    }
    ed247::StreamSample& sample = ed247_stream->pop_sample(empty);
    *sample_data = sample.data();
    *sample_size = sample.size();
    if(data_timestamp) *data_timestamp = &sample.data_timestamp();
    if(recv_timestamp) *recv_timestamp = &sample.recv_timestamp();
    if(sample_details) *sample_details = &sample.frame_infos();
  }
  LIBED247_CATCH("Pop stream sample");
  return ED247_STATUS_SUCCESS;
}

/* =========================================================================
 * Stream - List
 * ========================================================================= */
ed247_status_t ed247_stream_list_size(
  ed247_stream_list_t streams,
  uint32_t *          size)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!streams) {
    PRINT_ERROR(__func__ << ": Invalid streams");
    return ED247_STATUS_FAILURE;
  }
  if(!size) {
    PRINT_ERROR(__func__ << ": Invalid size pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    *size = static_cast<ed247_stream_clist_base_t*>(streams)->size();
  }
  LIBED247_CATCH("Stream list size");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_list_next(
  ed247_stream_list_t streams,
  ed247_stream_t *    stream)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!streams) {
    PRINT_ERROR(__func__ << ": Invalid streams");
    return ED247_STATUS_FAILURE;
  }
  if(!stream) {
    PRINT_ERROR(__func__ << ": Empty stream pointer");
    return ED247_STATUS_FAILURE;
  }
  *stream = nullptr;
  try{
    *stream = static_cast<ed247_stream_clist_base_t*>(streams)->get_next();
  }
  LIBED247_CATCH("Stream list next");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_list_free(
  ed247_stream_list_t streams)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!streams) {
    PRINT_ERROR(__func__ << ": Invalid streams");
    return ED247_STATUS_FAILURE;
  }
  try{
    static_cast<ed247_stream_clist_base_t*>(streams)->free();
  }
  LIBED247_CATCH("Stream list free");
  return ED247_STATUS_SUCCESS;
}

/* =========================================================================
 * Signal
 * ========================================================================= */
uint32_t ed247_nad_type_size(
  ed247_nad_type_t nad_type)
{
  return ed247::xml::Signal::get_nad_type_size(nad_type);
}

const char* ed247_signal_get_name(ed247_signal_t signal)
{
  auto ed247_signal = static_cast<ed247::signal*>(signal);
  return ed247_signal->get_name().c_str();  //note: get_name() return an internal reference.
}

const char* ed247_signal_get_comment(ed247_signal_t signal)
{
  auto ed247_signal = static_cast<ed247::signal*>(signal);
  return ed247_signal->get_comment().c_str();  //note: get_comment() return an internal reference.
}

const char* ed247_signal_get_icd(ed247_signal_t signal)
{
  auto ed247_signal = static_cast<ed247::signal*>(signal);
  return ed247_signal->get_icd().c_str();   //note: get_icd() return an internal reference.
}

ed247_signal_type_t ed247_signal_get_type(ed247_signal_t signal)
{
  auto ed247_signal = static_cast<ed247::signal*>(signal);
  return ed247_signal->get_type();
}

uint32_t ed247_signal_get_byte_offset(ed247_signal_t signal)
{
  auto ed247_signal = static_cast<ed247::signal*>(signal);
  return ed247_signal->get_byte_offset();
}

const char* ed247_signal_analogue_get_electrical_unit(ed247_signal_t signal)
{
  auto ed247_signal = static_cast<ed247::signal*>(signal);
  return ed247_signal->get_analogue_electrical_unit().c_str(); //note: get_analogue_electrical_unit() return an internal reference.
}

ed247_nad_type_t ed247_signal_nad_get_type(ed247_signal_t signal)
{
  auto ed247_signal = static_cast<ed247::signal*>(signal);
  return ed247_signal->get_nad_type();
}

const char* ed247_signal_nad_get_unit(ed247_signal_t signal)
{
  auto ed247_signal = static_cast<ed247::signal*>(signal);
  return ed247_signal->get_nad_unit().c_str();  //note: get_nad_unit() return an internal reference.
}

uint32_t ed247_signal_nad_get_dimensions_count(ed247_signal_t signal)
{
  auto ed247_signal = static_cast<ed247::signal*>(signal);
  return ed247_signal->get_nad_dimensions().size();
}

uint32_t ed247_signal_nad_get_dimension(ed247_signal_t signal, uint32_t dimention_id)
{
  auto ed247_signal = static_cast<ed247::signal*>(signal);
  return ed247_signal->get_nad_dimensions()[dimention_id];
}

uint32_t ed247_signal_vnad_get_position(ed247_signal_t signal)
{
  auto ed247_signal = static_cast<ed247::signal*>(signal);
  return ed247_signal->get_vnad_position();
}

uint32_t ed247_signal_vnad_get_max_number(ed247_signal_t signal)
{
  auto ed247_signal = static_cast<ed247::signal*>(signal);
  return ed247_signal->get_vnad_max_number();
}


ed247_status_t ed247_signal_set_user_data(
  ed247_signal_t signal,
  void *         user_data)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!signal){
    PRINT_ERROR(__func__ << ": Invalid signal");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_signal = static_cast<ed247::signal*>(signal);
    ed247_signal->set_user_data(user_data);
  }
  LIBED247_CATCH("Set signal user data");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_signal_get_user_data(
  ed247_signal_t signal,
  void **        user_data)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!signal) {
    PRINT_ERROR(__func__ << ": Invalid signal");
    return ED247_STATUS_FAILURE;
  }
  if(!user_data){
    PRINT_ERROR(__func__ << ": Invalid user_data");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_signal = static_cast<ed247::signal*>(signal);
    ed247_signal->get_user_data(user_data);
  }
  LIBED247_CATCH("Get signal user data");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_signal_get_stream(
  ed247_signal_t   signal,
  ed247_stream_t * stream)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!signal){
    PRINT_ERROR(__func__ << ": Invalid signal");
    return ED247_STATUS_FAILURE;
  }
  if(!stream){
    PRINT_ERROR(__func__ << ": Empty stream pointer");
    return ED247_STATUS_FAILURE;
  }
  *stream = nullptr;
  try{
    auto ed247_signal = static_cast<ed247::signal*>(signal);
    *stream = ed247_signal->get_api_stream();
  }
  LIBED247_CATCH("Get signal stream");
  return ED247_STATUS_SUCCESS;
}


ed247_status_t ed247_signal_allocate_sample(
  ed247_signal_t signal,
  void **        sample_data,
  uint32_t *     sample_size)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!signal){
    PRINT_ERROR(__func__ << ": Invalid signal");
    return ED247_STATUS_FAILURE;
  }
  if(!sample_data){
    PRINT_ERROR(__func__ << ": Empty sample_data pointer");
    return ED247_STATUS_FAILURE;
  }
  if(!sample_size){
    PRINT_ERROR(__func__ << ": Empty sample_size pointer");
    return ED247_STATUS_FAILURE;
  }
  *sample_data = nullptr;
  *sample_size = 0;
  try{
    auto ed247_signal = (ed247::signal*)(signal);
    *sample_size = ed247_signal->get_sample_max_size_bytes();
    *sample_data = malloc(*sample_size);
  }
  LIBED247_CATCH("Allocate stream sample");
  return ED247_STATUS_SUCCESS;
}

extern LIBED247_EXPORT ed247_status_t ed247_signal_free_sample(
  void* sample_data)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!sample_data){
    PRINT_ERROR(__func__ << ": Invalid sample_data");
    return ED247_STATUS_FAILURE;
  }
  try{
    free(sample_data);
  }
  LIBED247_CATCH("Free");
  return ED247_STATUS_SUCCESS;
}


/* =========================================================================
 * Signal - List
 * ========================================================================= */
ed247_status_t ed247_signal_list_size(
  ed247_signal_list_t signals,
  uint32_t *          size)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!signals) {
    PRINT_ERROR(__func__ << ": Invalid signals");
    return ED247_STATUS_FAILURE;
  }
  if(!size) {
    PRINT_ERROR(__func__ << ": Invalid size pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    *size = static_cast<ed247_signal_clist_base_t*>(signals)->size();
  }
  LIBED247_CATCH("Signal list size");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_signal_list_next(
  ed247_signal_list_t signals,
  ed247_signal_t *    signal)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!signals) {
    PRINT_ERROR(__func__ << ": Invalid signals");
    return ED247_STATUS_FAILURE;
  }
  if(!signal) {
    PRINT_ERROR(__func__ << ": Empty signal pointer");
    return ED247_STATUS_FAILURE;
  }
  *signal = nullptr;
  try{
    *signal = static_cast<ed247_signal_clist_base_t*>(signals)->get_next();
  }
  LIBED247_CATCH("Signal list next");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_signal_list_free(
  ed247_signal_list_t signals)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!signals) {
    // Nothing to do
    return ED247_STATUS_SUCCESS;
  }
  try{
    static_cast<ed247_signal_clist_base_t*>(signals)->free();
  }
  LIBED247_CATCH("Signal list free");
  return ED247_STATUS_SUCCESS;
}

/* =========================================================================
 * Stream assistant
 * ========================================================================= */
ed247_status_t ed247_stream_assistant_get_stream(
  ed247_stream_assistant_t assistant,
  ed247_stream_t *         stream)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!assistant){
    PRINT_ERROR(__func__ << ": Invalid assistant");
    return ED247_STATUS_FAILURE;
  }
  if(!stream){
    PRINT_ERROR(__func__ << ": Empty stream pointer");
    return ED247_STATUS_FAILURE;
  }
  *stream = nullptr;
  try{
    auto ed247_assistant = static_cast<ed247::StreamAssistant*>(assistant);
    *stream = ed247_assistant->get_api_stream();
  }
  LIBED247_CATCH("Get stream of assistant");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_assistant_write_signal(
  ed247_stream_assistant_t assistant,
  ed247_signal_t           signal,
  const void *             signal_sample_data,
  uint32_t                 signal_sample_size)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!assistant){
    PRINT_ERROR(__func__ << ": Invalid assistant");
    return ED247_STATUS_FAILURE;
  }
  if(!signal){
    PRINT_ERROR(__func__ << ": Invalid signal");
    return ED247_STATUS_FAILURE;
  }
  if(!signal_sample_data){
    PRINT_ERROR(__func__ << ": Empty signal sample data pointer");
    return ED247_STATUS_FAILURE;
  }
  try{
    ed247::StreamAssistant* ed247_assistant = static_cast<ed247::StreamAssistant*>(assistant);
    ed247::signal* ed247_signal = static_cast<ed247::signal*>(signal);
    if (ed247_assistant->write(*ed247_signal, signal_sample_data, signal_sample_size) == false) {
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Write signal sample in assistant");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_assistant_read_signal(
  ed247_stream_assistant_t assistant,
  ed247_signal_t           signal,
  const void **            signal_sample_data,
  uint32_t *               signal_sample_size)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!assistant){
    PRINT_ERROR(__func__ << ": Invalid assistant");
    return ED247_STATUS_FAILURE;
  }
  if(!signal){
    PRINT_ERROR(__func__ << ": Invalid signal");
    return ED247_STATUS_FAILURE;
  }
  if(!signal_sample_data){
    PRINT_ERROR(__func__ << ": Empty signal sample data pointer");
    return ED247_STATUS_FAILURE;
  }
  if(!signal_sample_size){
    PRINT_ERROR(__func__ << ": Empty signal sample size pointer");
    return ED247_STATUS_FAILURE;
  }
  *signal_sample_data = nullptr;
  *signal_sample_size = 0;
  try{
    ed247::StreamAssistant* ed247_assistant = static_cast<ed247::StreamAssistant*>(assistant);
    ed247::signal* ed247_signal = static_cast<ed247::signal*>(signal);
    if (ed247_assistant->read(*ed247_signal, signal_sample_data, signal_sample_size) == false) {
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Read signal sample from assistant");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_assistant_push_sample(
  ed247_stream_assistant_t  assistant,
  const ed247_timestamp_t * timestamp,
  bool *full)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!assistant){
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_assistant = static_cast<ed247::StreamAssistant*>(assistant);
    if (ed247_assistant->push(timestamp, full) == false) {
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Push stream sample with assistant");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_assistant_pop_sample(
  ed247_stream_assistant_t        assistant,
  const ed247_timestamp_t **      data_timestamp,
  const ed247_timestamp_t **      recv_timestamp,
  const ed247_sample_details_t ** sample_details,
  bool *                          empty)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!assistant){
    PRINT_ERROR(__func__ << ": Invalid assistant");
    return ED247_STATUS_FAILURE;
  }
  ed247_status_t result = ED247_STATUS_FAILURE;
  try{
    auto ed247_assistant = static_cast<ed247::StreamAssistant*>(assistant);
    result = ed247_assistant->pop(data_timestamp, recv_timestamp, sample_details, empty);
  }
  LIBED247_CATCH("Pop stream sample with assistant");
  return result;
}
