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
 * List iterators definition
 * ========================================================================= */
struct ed247_internal_channel_list_t :
  public ed247::client_iterator<ed247::channel_ptr_t>
{
  using ed247::client_iterator<value_t>::client_iterator;
  virtual ~ed247_internal_channel_list_t() {}
};
typedef ed247_internal_channel_list_t ed247_channel_iterator_t;

struct ed247_internal_stream_list_t :
  public ed247::client_iterator<ed247::stream_ptr_t>
{
  using ed247::client_iterator<value_t>::client_iterator;
  virtual ~ed247_internal_stream_list_t() {}
};
typedef ed247_internal_stream_list_t ed247_stream_iterator_t;

// Iterator that filter out streams without data
// Note: this shall be removed from the interface (wait_frame & wait_during)
struct ed247_stream_iterator_with_data_t : public ed247_stream_iterator_t
{
  using ed247_stream_iterator_t::ed247_stream_iterator_t;
  virtual client_iterator& advance() override {
    ed247_stream_iterator_t::advance();
    _iterator = std::find_if(_iterator,
                             _container->end(),
                             [](const ed247::stream_ptr_t& sp){ return sp->recv_stack().size() > 0; });
    return *this;
  }
  virtual ~ed247_stream_iterator_with_data_t() {}
};

struct ed247_internal_signal_list_t :
  public ed247::client_iterator<ed247::signal_ptr_t>
{
  using ed247::client_iterator<value_t>::client_iterator;
  virtual ~ed247_internal_signal_list_t() {}
};
typedef ed247_internal_signal_list_t ed247_signal_iterator_t;




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
 * ED247 Context - Init & general information
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
    ed247_context->initialize();
    *context = ed247_context;
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
    ed247_context->initialize();
    *context = ed247_context;
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
  }
  LIBED247_CATCH("Unload");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_component_get_info(
  ed247_context_t context,
  const ed247_component_info_t **info)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!context) {
    PRINT_ERROR(__func__ << ": Invalid context");
    return ED247_STATUS_FAILURE;
  }
  if(!info) {
    PRINT_ERROR(__func__ << ": Empty info pointer");
    return ED247_STATUS_FAILURE;
  }
  *info = nullptr;
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    *info = &ed247_context->getRoot()->info;
  }
  LIBED247_CATCH("Get component info");
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
 * ED247 Context - Get Configuration
 * ========================================================================= */
ed247_status_t ed247_get_channel_list(
  ed247_context_t        context,
  ed247_channel_list_t * channels)
{
  static ed247_channel_iterator_t channel_iterator;    // To prevent malloc(), this function will always return the same iterator

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
    channel_iterator.wrap(*(ed247_context->getPoolChannels()->channels().get()));
    *channels = &channel_iterator;
  }
  LIBED247_CATCH("Get channels info");
  return ED247_STATUS_SUCCESS;	
}

ed247_status_t ed247_find_channels(
  ed247_context_t        context,
  const char *           regex_name,
  ed247_channel_list_t * channels)
{
  static ed247_channel_iterator_t channel_iterator;    // iterator cannot be dynamic allocated since it will not be freed by list_free

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
    channel_iterator.copy(ed247_context->getPoolChannels()->find(regex_name != nullptr ? std::string(regex_name) : std::string(".*")));
    *channels = &channel_iterator;
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
      PRINT_WARNING("Cannot finnd channel '" << name << "'");
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
  static ed247_stream_iterator_t stream_iterator;    // To prevent malloc(), this function will always return the same iterator

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
    stream_iterator.wrap(*(ed247_context->getPoolStreams()->streams().get()));
    *streams = &stream_iterator;
  }
  LIBED247_CATCH("Get streams info");
  return ED247_STATUS_SUCCESS;
}


ed247_status_t ed247_find_streams(
  ed247_context_t       context,
  const char *          regex_name,
  ed247_stream_list_t * streams)
{
  static ed247_stream_iterator_t stream_iterator;    // iterator cannot be dynamic allocated since it will not be freed by list_free

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
    stream_iterator.copy(ed247_context->getPoolStreams()->find(regex_name != nullptr ? std::string(regex_name) : std::string(".*")));
    *streams = &stream_iterator;
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
      PRINT_WARNING("Cannot find stream '" << name << "'");
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
  static ed247_signal_iterator_t signal_iterator;    // iterator cannot be dynamic allocated since it will not be freed by list_free

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
    signal_iterator.copy(ed247_context->getPoolSignals()->find(regex_name != nullptr ? std::string(regex_name) : std::string(".*")));
    *signals = &signal_iterator;
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
    auto ed247_context = static_cast<ed247::Context*>(context);
    auto && ed247_signal = ed247_context->getPoolSignals()->get(std::string(name));
    *signal = ed247_signal ? ed247_signal.get() : nullptr;
    if(*signal == nullptr) {
      PRINT_WARNING("Cannot find signal '" << name << "'");
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
  static ed247_stream_iterator_with_data_t stream_iterator;    // To prevent malloc(), this function will always return the same iterator

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
      stream_iterator.wrap(*(ed247_context->getPoolStreams()->streams().get()));
      *streams = &stream_iterator;
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
  static ed247_stream_iterator_with_data_t stream_iterator;    // To prevent malloc(), this function will always return the same iterator

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
      stream_iterator.wrap(*(ed247_context->getPoolStreams()->streams().get()));
      *streams = &stream_iterator;
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
    auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
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
    auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
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
    ed247_stream_iterator_t& iterator = *(static_cast<ed247_stream_iterator_t*>(streams));
    for (auto istream = iterator.container().begin(); istream != iterator.container().end(); istream++) {
      if((*istream)->register_callback(context, callback) != ED247_STATUS_SUCCESS){
        status = ED247_STATUS_FAILURE;
        PRINT_WARNING("Cannot register callback in stream [" << (*istream)->get_name() << "]");
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
    ed247_stream_iterator_t& iterator = *(static_cast<ed247_stream_iterator_t*>(streams));
    for (auto istream = iterator.container().begin(); istream != iterator.container().end(); istream++) {
      if((*istream)->unregister_callback(context, callback) != ED247_STATUS_SUCCESS){
        status = ED247_STATUS_FAILURE;
        PRINT_WARNING("Cannot unregister callback in stream [" << (*istream)->get_name() << "]");
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

extern LIBED247_EXPORT ed247_status_t ed247_register_com_recv_callback(
  ed247_context_t      context,
  ed247_com_callback_t callback)
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
  ed247_status_t status = ED247_STATUS_FAILURE;
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    auto ed247_interfaces = std::static_pointer_cast<ed247::UdpSocket::Pool>(ed247_context->getPoolInterfaces());
    status = ed247_interfaces->register_recv_callback(callback, context);
  }
  LIBED247_CATCH("Register com recv callback");
  return status;
}

extern LIBED247_EXPORT ed247_status_t ed247_unregister_com_recv_callback(
  ed247_context_t      context,
  ed247_com_callback_t callback)
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
  ed247_status_t status = ED247_STATUS_FAILURE;
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    auto ed247_interfaces = std::static_pointer_cast<ed247::UdpSocket::Pool>(ed247_context->getPoolInterfaces());
    status = ed247_interfaces->unregister_recv_callback(callback, context);
  }
  LIBED247_CATCH("Unregister com recv callback");
  return status;
}

extern LIBED247_EXPORT ed247_status_t ed247_register_com_send_callback(
  ed247_context_t      context,
  ed247_com_callback_t callback)
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
  ed247_status_t status = ED247_STATUS_FAILURE;
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    auto ed247_interfaces = std::static_pointer_cast<ed247::UdpSocket::Pool>(ed247_context->getPoolInterfaces());
    status = ed247_interfaces->register_send_callback(callback, context);
  }
  LIBED247_CATCH("Register com send callback");
  return status;
}

extern LIBED247_EXPORT ed247_status_t ed247_unregister_com_send_callback(
  ed247_context_t      context,
  ed247_com_callback_t callback)
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
  ed247_status_t status = ED247_STATUS_FAILURE;
  try{
    auto ed247_context = static_cast<ed247::Context*>(context);
    auto ed247_interfaces = std::static_pointer_cast<ed247::UdpSocket::Pool>(ed247_context->getPoolInterfaces());
    status = ed247_interfaces->unregister_send_callback(callback, context);
  }
  LIBED247_CATCH("Unregister com send callback");
  return status;
}



/* =========================================================================
 * Simulation time
 * ========================================================================= */
ed247_status_t libed247_register_set_simulation_time_ns_handler(
  libed247_set_simulation_time_ns_t handler,
  void *                            user_data)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!handler) {
    PRINT_ERROR(__func__ << ": Invalid handler");
    return ED247_STATUS_FAILURE;
  }
  try {
    ed247::SimulationTimeHandler::get().set_handler(handler, user_data);
  }
  LIBED247_CATCH("Register simulation time handler");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t libed247_set_simulation_time_ns(
  ed247_time_sample_t time_sample,
  void *              user_data)
{
  _UNUSED(user_data);
  if(!time_sample) {
    PRINT_ERROR(__func__ << ": invalid time_sample pointer");
    return ED247_STATUS_FAILURE;
  }
#ifdef __linux__
  struct timespec tp;
  clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
  time_sample->epoch_s = (uint32_t)tp.tv_sec;
  time_sample->offset_ns = (uint32_t)((uint64_t)tp.tv_nsec);
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  time_sample->epoch_s = (uint32_t)tv.tv_sec;
  time_sample->offset_ns = (uint32_t)tv.tv_usec*1000LL;
#endif
  return ED247_STATUS_SUCCESS;
}

ed247_status_t libed247_update_time(
  ed247_time_sample_t time_sample,
  uint32_t            epoch_s,
  uint32_t            offset_ns)
{
  if(!time_sample) {
    PRINT_ERROR(__func__ << ": invalid time_sample pointer");
    return ED247_STATUS_FAILURE;
  }
  time_sample->epoch_s = epoch_s;
  time_sample->offset_ns = offset_ns;
  return ED247_STATUS_SUCCESS;
}


/* =========================================================================
 * Channel
 * ========================================================================= */
ed247_status_t ed247_channel_get_info(
  ed247_channel_t               channel,
  const ed247_channel_info_t ** info)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!channel) {
    PRINT_ERROR(__func__ << ": Invalid channel");
    return ED247_STATUS_FAILURE;
  }
  if(!info) {
    PRINT_ERROR(__func__ << ": Empty info pointer");
    return ED247_STATUS_FAILURE;
  }
  *info = nullptr;
  try{
    auto ed247_channel = static_cast<ed247::Channel*>(channel);
    *info = &ed247_channel->get_configuration()->info;
  }
  LIBED247_CATCH("Get channel info");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_channel_get_stream_list(
  ed247_channel_t       channel,
  ed247_stream_list_t * streams)
{
  static ed247_stream_iterator_t stream_iterator;    // To prevent malloc(), this function will always return the same iterator

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
    stream_iterator.wrap(*(ed247_channel->sstreams().get()));
    *streams = &stream_iterator;
  }
  LIBED247_CATCH("Get streams info");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_channel_find_streams(
  ed247_channel_t       channel,
  const char *          regex_name,
  ed247_stream_list_t * streams)
{
  static ed247_stream_iterator_t stream_iterator;    // iterator cannot be dynamic allocated since it will not be freed by list_free

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
    stream_iterator.copy(ed247_channel->find_streams(regex_name != nullptr ? std::string(regex_name) : std::string(".*")));
    *streams = &stream_iterator;
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
      PRINT_WARNING("Cannot find channel '" << name << "'");
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
  size_t *             size)
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
    *size = static_cast<ed247_channel_iterator_t*>(channels)->container_size();
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
    ed247_channel_iterator_t& iterator = *(static_cast<ed247_channel_iterator_t*>(channels));
    iterator.advance();
    *channel = iterator.valid() ? iterator.get_value().get() : nullptr;
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
    static_cast<ed247_channel_iterator_t*>(channels)->free();
  }
  LIBED247_CATCH("Channel list free");
  return ED247_STATUS_SUCCESS;
}


/* =========================================================================
 * Stream
 * ========================================================================= */
ed247_status_t ed247_stream_get_info(
  ed247_stream_t               stream,
  const ed247_stream_info_t ** info)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!stream) {
    PRINT_ERROR(__func__ << ": Invalid stream");
    return ED247_STATUS_FAILURE;
  }
  if(!info) {
    PRINT_ERROR(__func__ << ": Empty info pointer");
    return ED247_STATUS_FAILURE;
  }
  *info = nullptr;
  try{
    auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
    *info = &ed247_stream->get_configuration()->info;
  }
  LIBED247_CATCH("Get stream info");
  return ED247_STATUS_SUCCESS;
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
    auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
    *yesno = ed247_stream->signals()->size() != 0;
  }
  LIBED247_CATCH("Stream contains signals");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_get_signal_list(
  ed247_stream_t        stream,
  ed247_signal_list_t * signals)
{
  static ed247_signal_iterator_t signal_iterator;    // To prevent malloc(), this function will always return the same iterator

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
    ed247::BaseStream* ed247_stream = static_cast<ed247::BaseStream*>(stream);
    signal_iterator.wrap(*(ed247_stream->signals().get()));
    *signals = &signal_iterator;
  }
  LIBED247_CATCH("Stream get signals");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_find_signals(
  ed247_stream_t        stream,
  const char *          regex_name,
  ed247_signal_list_t * signals)
{
  static ed247_signal_iterator_t signal_iterator;    // iterator cannot be dynamic allocated since it will not be freed by list_free

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
    ed247::BaseStream* ed247_stream = (ed247::BaseStream*)(stream);
    signal_iterator.copy(ed247_stream->find_signals(regex_name != nullptr ? std::string(regex_name) : std::string(".*")));
    *signals = &signal_iterator;
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
    auto ed247_stream = (ed247::BaseStream*)(stream);
    auto && ed247_signal = ed247_stream->get_signal(std::string(name));
    *signal = ed247_signal ? ed247_signal.get() : nullptr;
    if(*signal == nullptr) {
      PRINT_WARNING("Cannot find signal '" << name << "'");
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
    auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
    auto && ed247_channel = ed247_stream->get_channel();
    *channel = ed247_channel ? ed247_channel.get() : nullptr;
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
    auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
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
    auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
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
    auto ed247_stream = (ed247::BaseStream*)(stream);
    *assistant = ed247_stream->get_assistant().get();
    if(!*assistant || !ed247_stream->get_assistant()->is_valid()){
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
  size_t *       sample_size)
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
    auto ed247_stream = (ed247::BaseStream*)(stream);
    auto sample = ed247_stream->allocate_sample();
    *sample_data = sample->data_rw();
    *sample_size = sample->capacity();
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
  size_t *          size)
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
    auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
    *size = direction == ED247_DIRECTION_IN ? ed247_stream->recv_stack().size() : ed247_stream->send_stack().size();
  }
  LIBED247_CATCH("Stream samples number");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_push_sample(
  ed247_stream_t            stream,
  const void *              sample_data,
  size_t                    sample_size,
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
    if (static_cast<ed247::BaseStream*>(stream)->push_sample(sample_data, sample_size, timestamp, full) == false) {
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Push stream sample");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_push_samples(
  ed247_stream_t            stream,
  const void *              samples_data,
  const size_t *            samples_size,
  size_t                    samples_number,
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
    auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
    size_t sum_size = 0;
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
  ed247_stream_t               stream,
  const void **                sample_data,
  size_t *                     sample_size,
  const ed247_timestamp_t **   data_timestamp,
  const ed247_timestamp_t **   recv_timestamp,
  const ed247_sample_info_t ** info,
  bool *                       empty)
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
    auto ed247_stream = static_cast<ed247::BaseStream*>(stream);
    if(ed247_stream->recv_stack().size() == 0) {
      PRINT_CRAZY("Stream '" << ed247_stream->get_name() << "': no data received.");
      return ED247_STATUS_NODATA;
    }
    auto sample = ed247_stream->pop_sample(empty);
    if (!sample) return ED247_STATUS_FAILURE;
    *sample_data = sample->data();
    *sample_size = sample->size();
    if(data_timestamp) *data_timestamp = sample->data_timestamp();
    if(recv_timestamp) *recv_timestamp = sample->recv_timestamp();
    if(info) *info = sample->info();
  }
  LIBED247_CATCH("Pop stream sample");
  return ED247_STATUS_SUCCESS;
}

/* =========================================================================
 * Stream - List
 * ========================================================================= */
ed247_status_t ed247_stream_list_size(
  ed247_stream_list_t streams,
  size_t *            size)
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
    *size = static_cast<ed247_stream_iterator_t*>(streams)->container_size();
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
    ed247_stream_iterator_t& iterator = *(static_cast<ed247_stream_iterator_t*>(streams));
    iterator.advance();
    *stream = iterator.valid() ? iterator.get_value().get() : nullptr;
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
    static_cast<ed247_stream_iterator_t*>(streams)->free();
  }
  LIBED247_CATCH("Stream list free");
  return ED247_STATUS_SUCCESS;
}

/* =========================================================================
 * Signal
 * ========================================================================= */
size_t ed247_nad_type_size(
  ed247_nad_type_t nad_type)
{
  return ed247::BaseSignal::nad_type_size(nad_type);
}

ed247_status_t ed247_signal_get_info(
  ed247_signal_t               signal,
  const ed247_signal_info_t ** info)
{
  PRINT_DEBUG("function " << __func__ << "()");

  if(!signal) {
    PRINT_ERROR(__func__ << ": Invalid signal");
    return ED247_STATUS_FAILURE;
  }
  if(!info) {
    PRINT_ERROR(__func__ << ": Empty info pointer");
    return ED247_STATUS_FAILURE;
  }
  *info = nullptr;
  try{
    auto ed247_signal = static_cast<ed247::BaseSignal*>(signal);
    *info = &ed247_signal->get_configuration()->info;
  }
  LIBED247_CATCH("Get signal info");
  return ED247_STATUS_SUCCESS;
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
    auto ed247_signal = static_cast<ed247::BaseSignal*>(signal);
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
    auto ed247_signal = static_cast<ed247::BaseSignal*>(signal);
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
    auto ed247_signal = static_cast<ed247::BaseSignal*>(signal);
    auto && ed247_stream = ed247_signal->get_stream();
    *stream = ed247_stream ? ed247_stream.get() : nullptr;
  }
  LIBED247_CATCH("Get signal stream");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_signal_allocate_sample(
  ed247_signal_t signal,
  void **        sample_data,
  size_t *       sample_size)
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
    auto ed247_signal = (ed247::BaseSignal*)(signal);
    auto sample = ed247_signal->allocate_sample();
    *sample_data = sample->data_rw();
    *sample_size = sample->capacity();
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
  size_t *            size)
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
    *size = static_cast<ed247_signal_iterator_t*>(signals)->container_size();
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
    ed247_signal_iterator_t& iterator = *(static_cast<ed247_signal_iterator_t*>(signals));
    iterator.advance();
    *signal = iterator.valid() ? iterator.get_value().get() : nullptr;
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
    static_cast<ed247_signal_iterator_t*>(signals)->free();
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
    auto ed247_assistant = static_cast<ed247::BaseStream::Assistant*>(assistant);
    *stream = ed247_assistant->get_stream().get();
  }
  LIBED247_CATCH("Get stream of assistant");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_assistant_write_signal(
  ed247_stream_assistant_t assistant,
  ed247_signal_t           signal,
  const void *             signal_sample_data,
  size_t                   signal_sample_size)
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
    auto ed247_assistant = static_cast<ed247::BaseStream::Assistant*>(assistant);
    auto && ed247_signal = static_cast<ed247::BaseSignal*>(signal)->shared_from_this();
    if (ed247_assistant->write(ed247_signal, signal_sample_data, signal_sample_size) == false) {
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
  size_t *                 signal_sample_size)
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
    auto ed247_assistant = static_cast<ed247::BaseStream::Assistant*>(assistant);
    auto ed247_signal = static_cast<ed247::BaseSignal*>(signal)->shared_from_this();
    if (ed247_assistant->read(ed247_signal, signal_sample_data, signal_sample_size) == false) {
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
    auto ed247_assistant = static_cast<ed247::BaseStream::Assistant*>(assistant);
    if (ed247_assistant->push(timestamp, full) == false) {
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Push stream sample with assistant");
  return ED247_STATUS_SUCCESS;
}

ed247_status_t ed247_stream_assistant_pop_sample(
  ed247_stream_assistant_t     assistant,
  const ed247_timestamp_t **   data_timestamp,
  const ed247_timestamp_t **   recv_timestamp,
  const ed247_sample_info_t ** info,
  bool *                       empty)
{
  PRINT_DEBUG("function " << __func__ << "()");
  if(!assistant){
    PRINT_ERROR(__func__ << ": Invalid assistant");
    return ED247_STATUS_FAILURE;
  }
  try{
    auto ed247_assistant = static_cast<ed247::BaseStream::Assistant*>(assistant);
    if(ed247_assistant->get_stream()->recv_stack().size() == 0) {
      PRINT_CRAZY("Stream '" << ed247_assistant->get_stream()->get_name() << "': no data received.");
      return ED247_STATUS_NODATA;
    }
    if (ed247_assistant->pop(data_timestamp, recv_timestamp, info, empty) == false) {
      return ED247_STATUS_FAILURE;
    }
  }
  LIBED247_CATCH("Pop stream sample with assistant");
  return ED247_STATUS_SUCCESS;
}
