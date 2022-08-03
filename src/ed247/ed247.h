/* -*- mode: c; c-basic-offset: 4 -*-  */
/******************************************************************************
 *
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
#ifndef _LIBED247_H_
#define _LIBED247_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __unix__
# define LIBED247_EXPORT __attribute__ ((visibility ("default")))
#elif _WIN32
# ifdef LIBED247_EXPORTS
   // We are building the DLL
#  define  LIBED247_EXPORT __declspec(dllexport)
# else
#  ifdef LIBED247_STATIC
    // We use a static version of the library
#   define LIBED247_EXPORT
#  else
    // Default: we are importing DLL  symbols
#   define LIBED247_EXPORT __declspec(dllimport)
#  endif
# endif
#endif

/* =========================================================================
 * Global
 * ========================================================================= */
/**
 * @defgroup global Global resources
 */

/* =========================================================================
 * Global constants
 * ========================================================================= */
/**
 * @brief Status codes
 * @ingroup global
 */
typedef enum {
    ED247_STATUS_SUCCESS = 0,
    ED247_STATUS_FAILURE,
    ED247_STATUS_TIMEOUT,
    ED247_STATUS_NODATA
} ed247_status_t;

/**
 * @brief Logging level
 * @ingroup global
 */
typedef enum {
    ED247_LOG_LEVEL_MIN       =   0,
    ED247_LOG_LEVEL_ERROR     =   ED247_LOG_LEVEL_MIN,
    ED247_LOG_LEVEL_DEFAULT   =   ED247_LOG_LEVEL_ERROR,
    ED247_LOG_LEVEL_WARNING   =   1,
    ED247_LOG_LEVEL_INFO      =   2,
    ED247_LOG_LEVEL_DEBUG     =   3,
    ED247_LOG_LEVEL_CRAZY     =  99,      // Will log each payload
    ED247_LOG_LEVEL_MAX       =   ED247_LOG_LEVEL_CRAZY,
    ED247_LOG_LEVEL_UNSET     = 100
} ed247_log_level_t;

/**
 * @brief Yes / No
 * @ingroup global
 */
typedef enum {
    ED247_YESNO_NO = 0,
    ED247_YESNO_YES,
    ED247_YESNO__INVALID
} ed247_yesno_t;

/**
 * @brief ED247 Standard revisions
 * @ingroup global
 */
typedef enum {
    ED247_STANDARD__INVALID,
    ED247_STANDARD_ED247,
    ED247_STANDARD_ED247A,
    ED247_STANDARD__COUNT
} ed247_standard_t;

/**
 * @brief Unique identifier type
 * @ingroup global
 */
typedef uint16_t ed247_uid_t;

/**
 * @brief Component types
 * @ingroup context
 */
typedef enum {
    ED247_COMPONENT_TYPE__INVALID,
    ED247_COMPONENT_TYPE_VIRTUAL,
    ED247_COMPONENT_TYPE_BRIDGE,
    ED247_COMPONENT_TYPE__COUNT
} ed247_component_type_t;

/**
 * @brief Stream types
 * @ingroup stream
 */
typedef enum {
    ED247_STREAM_TYPE__INVALID,
    ED247_STREAM_TYPE_A664,
    ED247_STREAM_TYPE_A429,
    ED247_STREAM_TYPE_A825,
    ED247_STREAM_TYPE_M1553,
    ED247_STREAM_TYPE_SERIAL,
    ED247_STREAM_TYPE_AUDIO,
    ED247_STREAM_TYPE_VIDEO,
    ED247_STREAM_TYPE_ETHERNET,
    ED247_STREAM_TYPE_ANALOG,
    ED247_STREAM_TYPE_DISCRETE,
    ED247_STREAM_TYPE_NAD,
    ED247_STREAM_TYPE_VNAD,
    ED247_STREAM_TYPE__COUNT
} ed247_stream_type_t;

/**
 * @brief Stream direction
 * @ingroup stream
 */
typedef enum {
    ED247_DIRECTION__INVALID = 0,
    ED247_DIRECTION_IN       = 0b01,
    ED247_DIRECTION_OUT      = 0b10,
    ED247_DIRECTION_INOUT    = ED247_DIRECTION_IN | ED247_DIRECTION_OUT
} ed247_direction_t;

/**
 * @brief Signal types
 * @ingroup signal
 */
typedef enum {
    ED247_SIGNAL_TYPE__INVALID,
    ED247_SIGNAL_TYPE_ANALOG    = ED247_STREAM_TYPE_ANALOG,
    ED247_SIGNAL_TYPE_DISCRETE  = ED247_STREAM_TYPE_DISCRETE,
    ED247_SIGNAL_TYPE_NAD       = ED247_STREAM_TYPE_NAD,
    ED247_SIGNAL_TYPE_VNAD      = ED247_STREAM_TYPE_VNAD
} ed247_signal_type_t;

/**
 * @brief NAD type
 * @ingroup signal
 */
typedef enum {
    ED247_NAD_TYPE__INVALID,
    ED247_NAD_TYPE_INT8,
    ED247_NAD_TYPE_INT16,
    ED247_NAD_TYPE_INT32,
    ED247_NAD_TYPE_INT64,
    ED247_NAD_TYPE_UINT8,
    ED247_NAD_TYPE_UINT16,
    ED247_NAD_TYPE_UINT32,
    ED247_NAD_TYPE_UINT64,
    ED247_NAD_TYPE_FLOAT32,
    ED247_NAD_TYPE_FLOAT64,
    ED247_NAD_TYPE__COUNT
} ed247_nad_type_t;

/**
 * @brief Discrete values to be used
 * @ingroup signal
 */
typedef enum {
    ED247_DISCRETE_FALSE = 0x00,
    ED247_DISCRETE_TRUE  = 0xFF
} ed247_discrete_t;



/* =========================================================================
 * Global Types
 * ========================================================================= */
/**
 * @brief Timestamp structure, seconds from EPOCH (January 1st 1970) and nanoseconds offset with reference to previous field
 * @ingroup global
 */
typedef struct {
    uint32_t epoch_s;
    uint32_t offset_ns;
} ed247_timestamp_t;
#define LIBED247_TIMESTAMP_DEFAULT ed247_timestamp_t{0, 0}

/**
 * @brief Context identifier
 * @ingroup context
 */
typedef struct ed247_internal_context_t *ed247_context_t;

/**
 * @brief Channel identifier
 * @ingroup channel
 */
typedef struct ed247_internal_channel_t *ed247_channel_t;

/**
 * @brief Channel list identifier
 * @ingroup channel_list
 */
typedef struct ed247_internal_channel_list_t *ed247_channel_list_t;

/**
 * @brief Stream identifier
 * @ingroup stream
 */
typedef struct ed247_internal_stream_t *ed247_stream_t;

/**
 * @brief Stream list identifier
 * @ingroup stream_list
 */
typedef struct ed247_internal_stream_list_t *ed247_stream_list_t;

/**
 * @brief Signal identifier
 * @ingroup signal
 */
typedef struct ed247_internal_signal_t *ed247_signal_t;

/**
 * @brief Signal list identifier
 * @ingroup signal_list
 */
typedef struct ed247_internal_signal_list_t *ed247_signal_list_t;

/**
 * @brief An assistant to help building stream samples
 * @ingroup stream_assistant
 */
typedef struct ed247_internal_stream_assistant_t *ed247_stream_assistant_t;


/* =========================================================================
 * Global Methods
 * ========================================================================= */
/**
 * @brief The name of the current implementation
 * @ingroup global
 * @return Current implementation name
 */
extern LIBED247_EXPORT const char * ed247_get_implementation_name();

/**
 * @brief The version of the current implementation
 * @ingroup global
 * @return Current implementation version
 */
extern LIBED247_EXPORT const char * ed247_get_implementation_version();

/**
 * @brief Setup the logging parameters
 * Environment variables have the priority: This function will be ignored if they are set.
 * @ingroup global
 * @param[in] Logging level
 * @retval ED247_STATUS_SUCCESS
 */
extern LIBED247_EXPORT ed247_status_t ed247_set_log(
    ed247_log_level_t log_level,
    const char *      log_filepath);

/**
 * @brief Setup the logging level (see ::ed247_log_level_t)
 * Environment variables have the priority: This function will be ignored if they are set.
 * @ingroup global
 * @param[in] Logging level
 * @retval ED247_STATUS_SUCCESS
 */
extern LIBED247_EXPORT ed247_status_t ed247_set_log_level(
    ed247_log_level_t log_level);

/**
 * @brief Get the logging level (see ::ed247_log_level_t)
 * @ingroup global
 * @param[out] Logging level
 * @retval ED247_STATUS_SUCCESS
 */
extern LIBED247_EXPORT ed247_status_t ed247_get_log_level(
    ed247_log_level_t * log_level);


/* =========================================================================
 * ED247 Context
 * ========================================================================= */
/**
 * @defgroup context ED247 Context
 * The context is the main entry point to load and use an ECIC.
 */

/**
 * @brief Library runtime metrics
 * @ingroup context
 */
typedef struct {
    uint32_t missed_frames;
    uint32_t sample_timestamp_offset_overflows;
} libed247_runtime_metrics_t;
#define LIBED247_RUNTIME_METRICS_DEFAULT libed247_runtime_metrics_t{0, 0}

/**
 * @brief Retrieve runtime metrics
 * @ingroup context
 * @param[in] context The context for which the runtime metrics are required
 * @param[out] metrics Pointer to the runtime metrics structure
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_get_runtime_metrics(
    ed247_context_t context,
    const libed247_runtime_metrics_t ** metrics);


/* =========================================================================
 * ED247 Context - Init
 * ========================================================================= */
/**
 * @defgroup context_init Initialization
 * @ingroup context
 */

/**
 * @brief Loading function: the entry point of the library
 * @ingroup context_init
 * @param[in] ecic_file_path The path to the ECIC configuration file
 * @param[in] configuration The configuration of the LIBED247
 * @param[out] context The loaded context identifier
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE An error occurred during the load phase (xml parsing or internal loading)
 */
extern LIBED247_EXPORT ed247_status_t ed247_load_file(
    const char *      ecic_file_path,
    ed247_context_t * context);

/**
 * @brief Loading function: the entry point of the library
 * @ingroup context_init
 * @param[in] ecic_file_content The content of the ECIC configuration file
 * @param[in] configuration The configuration of the LIBED247
 * @param[out] context The loaded context identifier
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE An error occurred during the load phase (xml parsing or internal loading)
 */
extern LIBED247_EXPORT ed247_status_t ed247_load_content(
    const char *      ecic_file_content,
    ed247_context_t * context);

/**
 * @brief Unload resources linked to the given context
 * @ingroup context_init
 * @param[in] context Context identifier
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_unload(
    ed247_context_t ed247_context);

/**
 * @brief Assign user data to the context
 * <b>When unloading the component, there is no memory free on this item. Free it yourself.</b>
 * @ingroup context_init
 * @param[in] context The context identifier
 * @param[in] user_data A pointer to user data
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_component_set_user_data(
    ed247_context_t context,
    void *user_data);

/**
 * @brief Retrieve user data pointer form the context
 * @ingroup context_init
 * @param[in] context The context identifier
 * @param[out] user_data A pointer to host pointer to user data
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_component_get_user_data(
    ed247_context_t context,
    void **user_data);

/* =========================================================================
 * ED247 Context - Global information
 * ========================================================================= */
/**
 * @defgroup context_info General information
 * @ingroup context
 * @{
 */
extern LIBED247_EXPORT const char* ed247_component_get_version(ed247_context_t context);
extern LIBED247_EXPORT ed247_component_type_t ed247_component_get_type(ed247_context_t context);
extern LIBED247_EXPORT const char* ed247_component_get_name(ed247_context_t context);
extern LIBED247_EXPORT const char* ed247_component_get_comment(ed247_context_t context);
extern LIBED247_EXPORT ed247_uid_t ed247_component_get_identifier(ed247_context_t context);
extern LIBED247_EXPORT ed247_standard_t ed247_component_get_standard_revision(ed247_context_t context);

extern LIBED247_EXPORT const char* ed247_file_producer_get_identifier(ed247_context_t context);
extern LIBED247_EXPORT const char* ed247_file_producer_get_comment(ed247_context_t context);
/** @} */

/* =========================================================================
 * ED247 Context - Get Configuration
 * ========================================================================= */
/**
 * @defgroup context_config Read configuration
 * @ingroup context
 */

/**
 * @brief Retrieve all the channels of the component
 * @ingroup context_config
 * Release memory with ::ed247_channel_list_free(channels)
 * To limit memory allocation, channels will be invalidate by next calls.
 * @param[in] context The context identifier
 * @param[out] channels List of the channels
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_get_channel_list(
    ed247_context_t        context,
    ed247_channel_list_t * channels);

/**
 * @brief Find all channels of the component whose name is matching the regular expression
 * <em>For example, to get a list of all the channels, use the /a * value.</em>
 * <em>The regex uses the <b>ECMAScript</b> grammar.</em>
 * <em>The regex do not embed implicit /a .* special characters at the beginning and the end.</em>
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * Release memory with ::ed247_channel_list_free(channels)
 * To limit memory allocation, channels will be invalidate by next calls.
 * @ingroup context_config
 * @param[in] context The context identifier.
 * @param[in] regex_name The regular expression for name matching. If null, assume '.*'.
 * @param[out] channels The list of the channels. If no value, set to null.
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE The channel list is empty
 * @note The regular expressions are not implemented in gcc4.8.x (default linux), do not use complex requests.
 */
extern LIBED247_EXPORT ed247_status_t ed247_find_channels(
    ed247_context_t        context,
    const char *           regex_name,
    ed247_channel_list_t * channels);

/**
 * @brief Find a channel of the component
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * @ingroup context_config
 * @param[in] context The context identifier.
 * @param[in] name The name of the channel.
 * @param[out] channel The channel identifier
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE The channel list is empty
 */
extern LIBED247_EXPORT ed247_status_t ed247_get_channel(
    ed247_context_t   context,
    const char *      name,
    ed247_channel_t * channel);

/**
 * @brief Retrieve all the streams of the component
 * Release memory with ::ed247_stream_list_free(streams)
 * To limit memory allocation, streams will be invalidate by next calls.
 * @ingroup context_config
 * @param[in] context The context identifier
 * @param[out] streams List of the streams
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_get_stream_list(
    ed247_context_t       context,
    ed247_stream_list_t * streams);

/**
 * @brief Find all streams of the component whose name is matching the regular expression
 * <em>For example, to get a list of all the streams, use the /a * value.</em>
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * Release memory with ::ed247_stream_list_free(streams)
 * To limit memory allocation, streams will be invalidate by next calls.
 * @ingroup context_config
 * @param[in] context The context identifier
 * @param[in] regex_name The regular expression for name matching. If null, assume '.*'.
 * @param[out] streams The list of the streams. If no value, set to null.
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE The stream list is empty
 * @note The regular expressions are not implemented in gcc4.8.x (default linux), do not use complex requests.
 */
extern LIBED247_EXPORT ed247_status_t ed247_find_streams(
    ed247_context_t       context,
    const char *          regex_name,
    ed247_stream_list_t * streams);


/**
 * @brief Find a stream of the component
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * @ingroup context_config
 * @param[in] context The context identifier
 * @param[in] name The name of the stream.
 * @param[out] stream The stream identifier
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE The stream list is empty
 */
extern LIBED247_EXPORT ed247_status_t ed247_get_stream(
    ed247_context_t  context,
    const char *     name,
    ed247_stream_t * stream);

/**
 * @brief Find all signals of the component whose name is matching the regular expression
 * <em>For example, to get a list of all the signals, use the /a * value.</em>
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * Release memory with ::ed247_signal_list_free(signals)
 * To limit memory allocation, signals will be invalidate by next calls.
 * @ingroup context_config
 * @param[in] context The context identifier
 * @param[in] regex_name The regular expression for name matching. If null, assume '.*'.
 * @param[out] signals The list of the signals. If no value, set to null.
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE The stream list is empty
 * @note The regular expressions are not implemented in gcc4.8.x (default linux), do not use complex requests.
 */
extern LIBED247_EXPORT ed247_status_t ed247_find_signals(
    ed247_context_t       context,
    const char *          regex_name,
    ed247_signal_list_t * signals);

/**
 * @brief Find a signal of the component
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * @ingroup context_config
 * @param[in] context The context identifier
 * @param[in] name The signal name.
 * @param[out] signal The signal identifier.
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE The stream list is empty
 */
extern LIBED247_EXPORT ed247_status_t ed247_get_signal(
    ed247_context_t  context,
    const char *     name,
    ed247_signal_t * signal);


/* =========================================================================
 * ED247 Context - Receive and send
 * ========================================================================= */
/**
 * @defgroup context_io Receive and send
 * @ingroup context
 * Receive and send data from/o network. To read/write data, see streams methods.
 */

/**
 * @brief Blocks until the first frame is received and processed, and at least a stream has available data.
 * If several frames has been received, they are all processed.
 * output streams:
 * - [new in 1.1.4] Can be set to NULL if you needn't the list. (this will prevent list computation)
 * - Release memory with ::ed247_stream_list_free(streams)
 * - To limit memory allocation, streams will be invalidate by next calls.
 * @ingroup context_io
 * @param[in] context Context identifier
 * @param[out] streams List of streams that received samples, can be NULL
 * @param[in] timeout_us Timeout value, in microseconds
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 * @retval ED247_STATUS_TIMEOUT
 */
extern LIBED247_EXPORT ed247_status_t ed247_wait_frame(
    ed247_context_t       context,
    ed247_stream_list_t * streams,
    int32_t               timeout_us);

/**
 * @brief Blocks until duration is elapsed, processing all received data.
 * output streams:
 * - [new in 1.1.4] Can be set to NULL if you needn't the list. (this will prevent list computation)
 * - Release memory with ::ed247_stream_list_free(streams)
 * - To limit memory allocation, streams will be invalidate by next calls.
 * @ingroup context_io
 * @param[in] context Context identifier
 * @param[out] streams List of streams that received samples, can be NULL
 * @param[in] duration_us Duration value, in microseconds
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_wait_during(
    ed247_context_t       context,
    ed247_stream_list_t * streams,
    int32_t               duration_us);

/**
 * @brief Send the samples that were written (::ed247_stream_write() or ::ed247_signal_write()) then pushed (::ed247_stream_push() or ::ed247_signal_push()).
 * <b>This function clear send stacks.</b>
 * @ingroup context_io
 * @param[in] context Context identifier
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_send_pushed_samples(
    ed247_context_t context);


/* =========================================================================
 * ED247 Context - Callbacks
 * ========================================================================= */
/**
 * @defgroup context_callback Receive and send callbacks
 * @ingroup context
 */

/**
 * @brief Stream receive callback function pointer.
 * The argument stream is the stream identifier that received something.
 * The decoding of the current frame will be aborted if the callback do not return ED247_STATUS_SUCCESS.
 * @ingroup context_callback
 */
typedef ed247_status_t (*ed247_stream_recv_callback_t)(ed247_context_t context, ed247_stream_t stream);

/**
 * @brief Register a callback (in a stream) which is called once a frame is received and decoded.
 * @ingroup context_callback
 * @param[in] stream Stream identifier
 * @param[in] callback The callback function
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_register_recv_callback(
    ed247_context_t              context,
    ed247_stream_t               stream,
    ed247_stream_recv_callback_t callback);

/**
 * @brief Unregister a callback (from a stream) which is called once a frame is received and decoded.
 * @ingroup context_callback
 * @param[in] stream Stream identifier
 * @param[in] callback The callback function
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_unregister_recv_callback(
    ed247_context_t              context,
    ed247_stream_t               stream,
    ed247_stream_recv_callback_t callback);

/**
 * @brief Register a callback (in several streams as once) which is called once a frame is received and decoded.
 * @ingroup context_callback
 * @param[in] streams Stream identifiers
 * @param[in] callback The callback function
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_streams_register_recv_callback(
    ed247_context_t              context,
    ed247_stream_list_t          streams,
    ed247_stream_recv_callback_t callback);

/**
 * @brief Unregister a callback (from several streams as once) which is called once a frame is received and decoded.
 * @ingroup context_callback
 * @param[in] streams Stream identifiers
 * @param[in] callback The callback function
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_streams_unregister_recv_callback(
    ed247_context_t              context,
    ed247_stream_list_t          streams,
    ed247_stream_recv_callback_t callback);

/**
 * @brief Register a callback (in all streams) which is called once a frame is received and decoded.
 * @ingroup context_callback
 * @param[in] streams Stream identifiers
 * @param[in] callback The callback function
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_register_recv_callback(
    ed247_context_t              context,
    ed247_stream_recv_callback_t callback);

/**
 * @brief Unegister a callback (in all streams) which is called once a frame is received and decoded.
 * @ingroup context_callback
 * @param[in] streams Stream identifiers
 * @param[in] callback The callback function
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_unregister_recv_callback(
    ed247_context_t              context,
    ed247_stream_recv_callback_t callback);


/* =========================================================================
 * Time handing
 * ========================================================================= */
/**
 * @defgroup time Time management
 * Functions to set simulation time for receive timestamp
 */

/**
 * @brief Prototype of a function to get time
 * @ingroup time
 */
typedef void (*ed247_get_time_t)(ed247_timestamp_t* timestamp);

/**
 * @brief Return the current time
 * This implementation use an internal and system-specific implementation.
 * This is the default function used to fill transport timestamp and receive timestamp.
 * You may have to override their value with the methods:
 * - ed247_register_transport_timestamp_callback()
 * - ed247_register_receive_timestamp_callback()
 * @ingroup time
 * @param[out] timestamp Filled with current time
 */
extern LIBED247_EXPORT void ed247_get_time(ed247_timestamp_t* timestamp);

/**
 * @brief Set the function to use to timestamp the transport (aka emit date)
 * The default function is ed247_get_time().
 * The TTS is only wrote in the stream if enabled by ECIC file.
 * The receiver can read the value in the field ed247_sample_details_t->transport_timestamp
 * returned by ed247_*_pop_sample() functions.
 * @ingroup time
 * @param[in] callback Function that will provide current time
 */
extern LIBED247_EXPORT void ed247_set_transport_timestamp_callback(ed247_get_time_t callback);

/**
 * @brief Set the function to use to timestamp the incoming streams (aka receive date)
 * The default function is ed247_get_time().
 * The library will date incoming stream during the call of ed247_wait_*() methods.
 * It will provide the value in the recv_timestamp field of ed247_*_pop_sample() functions.
 * This may be useful if you call ed247_wait_*() periodically but not ed247_*_pop_sample().
 * @ingroup time
 * @param[in] callback Function that will provide current time
 */
extern LIBED247_EXPORT void ed247_set_receive_timestamp_callback(ed247_get_time_t callback);

/**
 * @brief Return the time to timestamp the transport
 * Call either ed247_get_time() or the function set by ed247_set_transport_timestamp_callback()
 * @ingroup time
 * @param[out] timestamp Filled with the appropriate time
 */
extern LIBED247_EXPORT void ed247_get_transport_timestamp(ed247_timestamp_t* timestamp);

/**
 * @brief Return the time to timestamp the incoming streams
 * Call either ed247_get_time() or the function set by ed247_set_receive_timestamp_callback()
 * @ingroup time
 * @param[out] timestamp Filled with the appropriate time
 */
extern LIBED247_EXPORT void ed247_get_receive_timestamp(ed247_timestamp_t* timestamp);


/* =========================================================================
 * Channel
 * ========================================================================= */
/**
 * @defgroup channel Channel
 */

/**
 * @ingroup channel
 * @{
 */
extern LIBED247_EXPORT const char* ed247_channel_get_name(ed247_channel_t channel);
extern LIBED247_EXPORT const char* ed247_channel_get_comment(ed247_channel_t channel);
extern LIBED247_EXPORT ed247_standard_t ed247_channel_get_frame_standard_revision(ed247_channel_t channel);
/** @} */


/**
 * @brief Retrieve all the streams of the channel
 * Release memory with ::ed247_stream_list_free(streams)
 * To limit memory allocation, streams will be invalidate by next calls.
 * @ingroup channel
 * @param[in] channel The channel identifier
 * @param[out] streams List of the streams
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_channel_get_stream_list(
    ed247_channel_t       channel,
    ed247_stream_list_t * streams);

/**
 * @brief Find all streams of the channel whose name is matching the regular expression
 * <em>For example, to get a list of all the streams, use the /a * value.</em>
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * Release memory with ::ed247_stream_list_free(streams)
 * To limit memory allocation, streams will be invalidate by next calls.
 * @ingroup channel
 * @param[in] channel The channel identifier
 * @param[in] regex_name The regular expression for name matching. If null, assume '.*'.
 * @param[out] streams The list of the streams. If no value, set to null.
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE The stream list is empty
 * @note The regular expressions are not implemented in gcc4.8.x (default linux), do not use complex requests.
 */
extern LIBED247_EXPORT ed247_status_t ed247_channel_find_streams(
    ed247_channel_t       channel,
    const char *          regex_name,
    ed247_stream_list_t * streams);

/**
 * @brief Find a stream in a channel
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * @ingroup channel
 * @param[in] channel The channel identifier
 * @param[in] name The name of the stream.
 * @param[out] stream The stream identifier.
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE The stream list is empty
 */
extern LIBED247_EXPORT ed247_status_t ed247_channel_get_stream(
    ed247_channel_t  channel,
    const char *     name,
    ed247_stream_t * stream);

/**
 * @brief Assign user data to the channel
 * Memory has to be free by the user.
 * @ingroup channel
 * @param[in] channel The channel identifier
 * @param[in] user_data Pointer to user data
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_channel_set_user_data(
    ed247_channel_t channel,
    void *          user_data);

/**
 * @brief Retrieve user data assigned to the channel
 * @ingroup channel
 * @param[in] channel The channel identifier
 * @param[out] user_data Pointer to host pointer to user data
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_channel_get_user_data(
    ed247_channel_t channel,
    void **         user_data);


/* =========================================================================
 * Channel - List
 * ========================================================================= */
/**
 * @defgroup channel_list Channel list
 * @ingroup channel
 */

/**
 * @brief Get the size of the list
 * @ingroup channel_list
 * @param[in] channels The channel list
 * @param[out] size The size of the list
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_channel_list_size(
    ed247_channel_list_t channels,
    size_t *             size);

/**
 * @brief Iterate over a channel identifier list
 * @ingroup channel_list
 * @param[in,out] channels The channel list
 * @param[out] channel A pointer to the current item in the list.
 * A /a null value is set when the end of the list is reached.
 * The next call will return the pointer to the first item of the list.
 * @retval ED247_STATUS_SUCCESS Operation completed successfully (although end of list may be reached)
 * @retval ED247_STATUS_FAILURE Invalid parameter provided or internal error
 */
extern LIBED247_EXPORT ed247_status_t ed247_channel_list_next(
    ed247_channel_list_t channels,
    ed247_channel_t *    channel);

/**
 * @brief Free channel list
 * @ingroup channel_list
 * @param[in] channels The channel list
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_channel_list_free(
    ed247_channel_list_t channels);



/* =========================================================================
 * Stream
 * ========================================================================= */
/**
 * @defgroup stream Stream
 */

/**
 * @brief Sample Details
 * @ingroup stream
 */
typedef struct ed247_sample_details_s {
    ed247_uid_t       component_identifier;
    uint16_t          sequence_number;
    ed247_timestamp_t transport_timestamp;
} ed247_sample_details_t;
#define LIBED247_SAMPLE_DETAILS_DEFAULT ed247_sample_details_t{0, 0, LIBED247_TIMESTAMP_DEFAULT}

/**
 * @ingroup stream
 * @{
 */
extern LIBED247_EXPORT const char* ed247_stream_get_name(ed247_stream_t stream);
extern LIBED247_EXPORT ed247_direction_t ed247_stream_get_direction(ed247_stream_t stream);
extern LIBED247_EXPORT ed247_stream_type_t ed247_stream_get_type(ed247_stream_t stream);
extern LIBED247_EXPORT const char* ed247_stream_get_comment(ed247_stream_t stream);
extern LIBED247_EXPORT const char* ed247_stream_get_icd(ed247_stream_t stream);
extern LIBED247_EXPORT ed247_uid_t ed247_stream_get_uid(ed247_stream_t stream);
extern LIBED247_EXPORT size_t ed247_stream_get_sample_max_number(ed247_stream_t stream);
extern LIBED247_EXPORT size_t ed247_stream_get_sample_max_size_bytes(ed247_stream_t stream);
extern LIBED247_EXPORT size_t ed247_stream_get_sampling_period_us(ed247_stream_t stream);
/** @} */


/**
 * @brief Check if the stream is a signal based one (DISCRETE, ANALOG, NAD or VNAD)
 * @ingroup stream
 * @param[in] stream Stream identifier
 * @param[out] yesno Result of the check, true for yes and false for no
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_has_signals(
    ed247_stream_t stream,
    uint8_t *      yesno);

/**
 * @brief Return all signals of a stream
 * @ingroup stream
 * Release memory with ::ed247_signal_list_free(signals)
 * To limit memory allocation, signals will be invalidate by next calls.
 * @param[in] stream The stream identifier
 * @param[in] regex_name The regular expression for name matching
 * @param[out] signals The list of the signals. If no value, set to null.
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE The stream list is empty
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_get_signal_list(
    ed247_stream_t        stream,
    ed247_signal_list_t * signals);

/**
 * @brief Find all signals of the stream whose name is matching the regular expression
 * <em>For example, to get a list of all the signals, use the /a * value.</em>
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * @ingroup stream
 * Release memory with ::ed247_signal_list_free(signals)
 * To limit memory allocation, signals will be invalidate by next calls.
 * @param[in] stream The stream identifier
 * @param[in] regex_name The regular expression for name matching. If null, assume '.*'.
 * @param[out] signals The list of the signals. If no value, set to null.
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE The stream list is empty
 * @note The regular expressions are not implemented in gcc4.8.x (default linux), do not use complex requests.
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_find_signals(
    ed247_stream_t        stream,
    const char *          regex_name,
    ed247_signal_list_t * signals);

/**
 * @brief Get a signal of the stream
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * @ingroup stream
 * @param[in] stream The stream identifier
 * @param[in] name The stream name.
 * @param[out] signal The signal identifier.
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE The stream list is empty
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_get_signal(
    ed247_stream_t   stream,
    const char *     name,
    ed247_signal_t * signal);

/**
 * @brief Retrieve the channel of the stream
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * @ingroup stream
 * @param[in] stream The stream identifier
 * @param[out] channel The channel identifier
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_get_channel(
    ed247_stream_t    stream,
    ed247_channel_t * channel);

/**
 * @brief Assign user data to the stream
 * Memory has to be free by the user.
 * @ingroup stream
 * @param[in] stream The stream identifier
 * @param[in] user_data Pointer to user data
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_set_user_data(
    ed247_stream_t stream,
    void *         user_data);

/**
 * @brief Retrieve user data assigned to the stream
 * @ingroup stream
 * @param[in] stream The stream identifier
 * @param[out] user_data Pointer to host pointer to user data
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_get_user_data(
    ed247_stream_t stream,
    void **        user_data);


/* =========================================================================
 * Stream - Read & Write
 * ========================================================================= */
/**
 * @defgroup stream_io Read and Write
 * @ingroup stream
 * Note: to read/write signals see stream assistant
 */

/**
 * @brief Get an assistant to build stream samples based on signals.
 * @ingroup stream_io
 * @param[in] stream Stream identifier
 * @param[out] assistant Stream assistant
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE When the stream is not signal based
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_get_assistant(
    ed247_stream_t             stream,
    ed247_stream_assistant_t * assistant);

/**
 * @brief Allocate memory of a stream sample with the right memory size. The size of the sample is deducted from the SampleMaxSizeBytes ED247 parameter of the stream.
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * <b>When not needed anymore, the memory free shall be made by the user with ::ed247_stream_free_sample().</b>
 * @ingroup stream_io
 * @param[in] stream Stream identifier
 * @param[out] sample_data Pointer to the allocated memory
 * @param[out] sample_size Size of the memory allocated for the sample
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_allocate_sample(
    ed247_stream_t stream,
    void **        sample_data,
    size_t *       sample_size);

/**
 * @brief Free memory allocated by ::ed247_stream_allocate_sample().
 * @param[in] data Pointer to element to be disposed
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE when sample_data is not allocated
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_free_sample(
    void* sample_data);


/**
 * @brief Number of samples in the stream stack, incremented and decremented according to ::ed247_stream_push() & ::ed247_stream_pop() functions.
 * @ingroup stream_io
 * @param[in] stream Stream identifier
 * @param[in] direction Only ::ED247_DIRECTION_IN or ::ED247_DIRECTION_OUT accepted, reference the desired stack to write on
 * @param[out] size Number of samples in the stack
 * @retval ED247_STATUS_SUCCESS
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_samples_number(
    ed247_stream_t    stream,
    ed247_direction_t direction,
    size_t *          size);

/**
 * @brief Write & push a single sample in the sample buffer of the stream.
 * <b>This function may lead to the emission of frames according to packetization strategies.</b>
 * @ingroup stream_io
 * @param[in] stream Stream identifier
 * @param[in] sample_data Sample data to write, copied internally
 * @param[in] sample_data_size Size of the sample data to write, in bytes
 * @param[in] data_timestamp Data timestamp associated to the sample. Set to NULL if not desired, otherwise see ::ed247_timestamp_t
 * @param[out] full Returns true if the internal stack is full. Set to NULL if not desired
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_push_sample(
    ed247_stream_t            stream,
    const void *              sample_data,
    size_t                    sample_data_size,
    const ed247_timestamp_t * data_timestamp,
    bool *                    full);

/**
 * @brief Write several stream samples at once.
 * <b>This function may lead to the emission of frames according to packetization strategies.</b>
 * @ingroup stream_io
 * @param[in] stream Stream identifier
 * @param[in] samples_data Samples data to write, copied internally
 * @param[in] sample_size Size of the sample data to write, in bytes
 * @param[in] samples_number Number of samples to write. It must correspond to the number of elements in samples_data & samples_size
 * @param[in] data_timestamp Data timestamp associated to the sample. Set to NULL if not desired, otherwise see ::ed247_timestamp_t
 * @param[out] full Returns true if the internal stack is full. Set to NULL if not desired
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_push_samples(
    ed247_stream_t            stream,
    const void *              samples_data,
    const size_t *            samples_size,
    size_t                    samples_number,
    const ed247_timestamp_t * data_timestamp,
    bool *                    full);

/**
 * @brief Read & pop a sample received on the stream.
 * @ingroup stream_io
 * @param[in] stream Stream identifier
 * @param[out] sample_data Pointer on the internal stream buffer sample
 * @param[out] sample_size Size of the received sample data
 * @param[out] data_timestamp Pointer on the internal buffer sample data timestamp, set to NULL if not received
 * @param[out] recv_timestamp Pointer on the internal buffer recv timestamp.
 * The value is set according to ::libed247_set_simulation_time_ns() callback that has to be specified by the user.
 * If no callback is specified, the pointer is set to NULL
 * @param[out] sample_details Stream sample details
 * @param[out] empty Returns true if the internal stack is empty
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_NODATA Receive stack is empty
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_pop_sample(
    ed247_stream_t                  stream,
    const void **                   sample_data,
    size_t *                        sample_size,
    const ed247_timestamp_t **      data_timestamp,
    const ed247_timestamp_t **      recv_timestamp,
    const ed247_sample_details_t ** sample_details,
    bool *                          empty);


/* =========================================================================
 * Stream - List
 * ========================================================================= */
/**
 * @defgroup stream_list Stream list
 * @ingroup stream
 */

/**
 * @brief Get the size of the list
 * @ingroup stream_list
 * @param[in] streams The stream list
 * @param[out] size The size of the list
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_list_size(
    ed247_stream_list_t streams,
    size_t *            size);

/**
 * @brief Iterate over a stream identifier list
 * @ingroup stream_list
 * @param[in,out] streams The stream list
 * @param[out] stream A pointer to the current item in the list.
 * A /a null value is set when the end of the list is reached.
 * The next call will return the pointer to the first item of the list.
 * @retval ED247_STATUS_SUCCESS Operation completed successfully (although end of list may be reached)
 * @retval ED247_STATUS_FAILURE Invalid parameter provided or internal error
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_list_next(
    ed247_stream_list_t streams,
    ed247_stream_t *    stream);

/**
 * @brief Free stream list
 * <b>Do not use during runtime. The implementation may contain memory allocation/deallocation functions.</b>
 * @ingroup stream_list
 * @param[in] streams The stream list
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_list_free(
    ed247_stream_list_t streams);



/* =========================================================================
 * Signal
 * ========================================================================= */
/**
 * @defgroup signal Signal
 * Note: To read/write signals, see stream assistant
 */

/**
 * @brief Size of a single element of ::ed247_nad_type_t
 * @ingroup signal
 * @param[in] nad_type The NAD type
 * @return The size of the NAD type element (sizeof equivalent)
 */
extern LIBED247_EXPORT size_t ed247_nad_type_size(ed247_nad_type_t nad_type);

/**
 * @ingroup signal
 * @{
 */
extern LIBED247_EXPORT const char* ed247_signal_get_name(ed247_signal_t signal);
extern LIBED247_EXPORT const char* ed247_signal_get_comment(ed247_signal_t signal);
extern LIBED247_EXPORT const char* ed247_signal_get_icd(ed247_signal_t signal);
extern LIBED247_EXPORT ed247_signal_type_t ed247_signal_get_type(ed247_signal_t signal);
/** @brief Meaningless for VNAD */
extern LIBED247_EXPORT uint32_t ed247_signal_get_byte_offset(ed247_signal_t signal);
extern LIBED247_EXPORT const char* ed247_signal_analogue_get_electrical_unit(ed247_signal_t analogue_signal);
/** @brief For NAD and VNAD */
extern LIBED247_EXPORT ed247_nad_type_t ed247_signal_nad_get_type(ed247_signal_t nad_signal);
/** @brief For NAD and VNAD */
extern LIBED247_EXPORT const char* ed247_signal_nad_get_unit(ed247_signal_t nad_signal);
extern LIBED247_EXPORT uint32_t ed247_signal_nad_get_dimensions_count(ed247_signal_t nad_signal);
extern LIBED247_EXPORT uint32_t ed247_signal_nad_get_dimension(ed247_signal_t nad_signal, uint32_t dimention_id);
extern LIBED247_EXPORT uint32_t ed247_signal_vnad_get_position(ed247_signal_t vnad_signal);
extern LIBED247_EXPORT uint32_t ed247_signal_vnad_get_max_length(ed247_signal_t vnad_signal);
/** @} */

/**
 * @brief Assign user data to the signal
 * Memory has to be free by the user.
 * @ingroup signal
 * @param[in] signal The signal identifier
 * @param[in] user_data Pointer to user data
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_signal_set_user_data(
    ed247_signal_t signal,
    void *         user_data);

/**
 * @brief Retrieve user data assigned to the signal
 * @ingroup signal
 * @param[in] signal The signal identifier
 * @param[out] user_data Pointer to host pointer to user data
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_signal_get_user_data(
    ed247_signal_t signal,
    void **        user_data);

/**
 * @brief Retrieve the stream of the signal
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * @ingroup signal
 * @param[in] signal The signal identifier
 * @param[out] stream The stream identifier
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_signal_get_stream(
    ed247_signal_t   signal,
    ed247_stream_t * stream);

/**
 * @brief Allocate memory of a signal sample with the right memory size.
 * The size of the sample is deducted from the SampleMaxSizeBytes ED247 parameter of the signal.
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * <b>When not needed anymore, the memory free shall be made by the user with ::ed247_signal_free_sample().</b>
 * @ingroup signal
 * @param[in] signal Signal identifier
 * @param[out] sample_data Pointer to the allocated memory
 * @param[out] sample_size Size of the memory allocated for the sample
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_signal_allocate_sample(
    ed247_signal_t signal,
    void **        sample_data,
    size_t *       sample_size);

/**
 * @brief Free memory allocated by ::ed247_signal_allocate_sample().
 * @param[in] data Pointer to element to be disposed
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE when sample_data is not allocated
 */
extern LIBED247_EXPORT ed247_status_t ed247_signal_free_sample(
    void * sample_data);


/* =========================================================================
 * Signal - List
 * ========================================================================= */
/**
 * @defgroup signal_list Signal list
 * @ingroup signal
 */

/**
 * @brief Get the size of the list
 * @ingroup signal_list
 * @param[in] signals The signal list
 * @param[out] size The size of the list
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_signal_list_size(
    ed247_signal_list_t signals,
    size_t *            size);

/**
 * @brief Iterate over a signal identifier list
 * @ingroup signal_list
 * @param[in,out] signals The signal list
 * @param[out] signal A pointer to the current item in the list.
 * A /a null value is set when the end of the list is reached.
 * The next call will return the pointer to the first item of the list.
 * @retval ED247_STATUS_SUCCESS Operation completed successfully (although end of list may be reached)
 * @retval ED247_STATUS_FAILURE Invalid parameter provided or internal error
 */
extern LIBED247_EXPORT ed247_status_t ed247_signal_list_next(
    ed247_signal_list_t signals,
    ed247_signal_t *    signal);

/**
 * @brief Free signal list
 * <b>Do not use during runtime. The implementation may contain memory allocation/deallocation functions.</b>
 * @ingroup signal_list
 * @param[in] signals The signal list
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_signal_list_free(
    ed247_signal_list_t signals);




/* =========================================================================
 * Stream assistant
 * ========================================================================= */
/**
 * @defgroup stream_assistant Stream Assistant (read/write signals)
 * Helper methods to read and write signals within a stream
 */

/**
 * @brief Get the stream associated to the assistant
 * @ingroup stream_assistant
 * @param[in] assistant Assistant identifier
 * @param[out] stream Stream identifier pointer
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_assistant_get_stream(
    ed247_stream_assistant_t assistant,
    ed247_stream_t *         stream);

/**
 * @brief Write signal sample into the assistant buffer. Once all signal are written, use ::ed247_stream_signal_assistant_push_sample() to push the sample on the stream.
 * This function returns an error if the stream direction is not ::ED247_DIRECTION_IN or ::ED247_DIRECTION_INOUT.
 * In case of DISCRETE Signal, the signal_sample_size equals to 1 byte
 * In case of ANALOG Signal, the signal_sample_size equals to 4 bytes (float). There is no need to swap the sample.
 * In case of NAD Signal, the signal_sample_size equals to the size of an atomic element, given by ed247_nad_type_size(nad_type),
 * multiplied by all the dimensions (which is invariant).
 * There is no need to swap the samples.
 * In case of VNAD Signal, the signal_sample_size equals to the size of an atomic element, given by ed247_nad_type_size(nad_type),
 * multiplied by the effective size of the sample (which is variable).
 * There is no need to swap the samples.
 * @ingroup stream_assistant
 * @param[in] assistant Assistant identifier
 * @param[in] signal Signal identifier
 * @param[in] signal_sample_data Retrieve pointer of allocated memory
 * @param[in] signal_sample_size Retrieve size of allocated memory
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_assistant_write_signal(
    ed247_stream_assistant_t assistant,
    ed247_signal_t           signal,
    const void *             signal_sample_data,
    size_t                   signal_sample_size);

/**
 * @brief Read signal sample from the assistant buffer. The assistant internal stream sample buffer is updated by calling ::ed247_stream_signal_assistant_pop_sample().
 * This function returns an error if the stream direction is not ::ED247_DIRECTION_OUT or ::ED247_DIRECTION_INOUT.
 * In case of DISCRETE Signal, the signal_sample_size equals to 1 byte
 * In case of ANALOG Signal, the signal_sample_size equals to 4 bytes (float). There is no need to swap the sample.
 * In case of NAD Signal, the signal_sample_size equals to the size of an atomic element, given by ed247_nad_type_size(nad_type),
 * multiplied by all the dimensions (which is invariant).
 * There is no need to swap the samples.
 * In case of VNAD Signal, the signal_sample_size equals to the size of an atomic element, given by ed247_nad_type_size(nad_type),
 * multiplied by the effective size of the sample (which is variable).
 * There is no need to swap the samples.
 * @ingroup stream_assistant
 * @param[in] assistant Assistant identifier
 * @param[in] signal Signal identifier
 * @param[in] signal_sample_data Retrieve pointer of the stream sample allocated in memory
 * @param[in] signal_sample_size Retrieve size of the stream sample allocated in memory
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_assistant_read_signal(
    ed247_stream_assistant_t assistant,
    ed247_signal_t           signal,
    const void **            signal_sample_data,
    size_t *                 signal_sample_size);



/**
 * @brief Push the stream sample to the stream
 * <b>This function may lead to the emission of frames according to packetization strategies.</b>
 * @ingroup stream_assistant
 * @param[in] assistant Assistant identifier
 * @param[in] data_timestamp Data timestamp associated to the sample. Set to NULL if not desired, otherwise see ::ed247_timestamp_t
 * @param[out] full Returns true if the internal stack is full. Set to NULL if not desired
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_assistant_push_sample(
    ed247_stream_assistant_t  assistant,
    const ed247_timestamp_t * data_timestamp,
    bool *                    full);

/**
 * @brief Read & pop a sample received on the stream.
 * @ingroup stream_assistant
 * @param[in] assistant Assistant identifier
 * @param[out] data_timestamp Pointer on the internal buffer sample data timestamp, set to NULL if not received
 * @param[out] recv_timestamp Pointer on the internal buffer recv timestamp.
 * The value is set according to ::libed247_set_simulation_time_ns() callback that has to be specified by the user.
 * If no callback is specified, the pointer is set to NULL
 * @param[out] sample_details Stream sample details
 * @param[out] empty Returns true if the internal stack is empty
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_NODATA Receive stack is empty
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_assistant_pop_sample(
    ed247_stream_assistant_t        assistant,
    const ed247_timestamp_t **      data_timestamp,
    const ed247_timestamp_t **      recv_timestamp,
    const ed247_sample_details_t ** sample_details,
    bool *                          empty);

/* =========================================================================
 * Strings conversion
 * ========================================================================= */
/**
 * @defgroup strings String conversion
 * Helper to convert type to/from strings.
 */

/**
 * @brief ::ed247_status_t to string conversion
 * @ingroup strings
 * @param[in] status The value to convert
 * @return The corresponding string
 */
extern LIBED247_EXPORT const char * ed247_status_string(
    ed247_status_t status);

/**
 * @brief ::ed247_standard_t to string conversion
 * @ingroup strings
 * @param[in] standard The value to convert
 * @return The corresponding string
 */
extern LIBED247_EXPORT const char * ed247_standard_string(
    ed247_standard_t standard);

/**
 * @brief ::ed247_standard_t from string conversion
 * @ingroup strings
 * @param[in] standard The string to convert
 * @return The corresponding ::ed247_standard_t value
 */
extern LIBED247_EXPORT ed247_standard_t ed247_standard_from_string(
    const char *standard);

/**
 * @brief ::ed247_direction_t to string conversion
 * @ingroup strings
 * @param[in] direction The value to convert
 * @return The corresponding string
 */
extern LIBED247_EXPORT const char * ed247_direction_string(
    ed247_direction_t direction);

/**
 * @brief ::ed247_direction_t from string conversion
 * @ingroup strings
 * @param[in] direction The string to convert
 * @return The corresponding ::ed247_direction_t value
 */
extern LIBED247_EXPORT ed247_direction_t ed247_direction_from_string(
    const char *direction);

/**
 * @brief ::ed247_yesno_t to string conversion
 * @ingroup strings
 * @param[in] yesno The value to convert
 * @return The corresponding string
 */
extern LIBED247_EXPORT const char * ed247_yesno_string(
    ed247_yesno_t yesno);

/**
 * @brief ::ed247_yesno_t from string conversion
 * @ingroup strings
 * @param[in] yesno The string to convert
 * @return The corresponding ::ed247_yesno_t value
 */
extern LIBED247_EXPORT ed247_yesno_t ed247_yesno_from_string(
    const char *yesno);

/**
 * @brief ::ed247_component_type_t to string conversion
 * @ingroup strings
 * @param[in] component_type The value to convert
 * @return The corresponding string
 */
extern LIBED247_EXPORT const char * ed247_component_type_string(
    ed247_component_type_t component_type);

/**
 * @brief ::ed247_component_type_t from string conversion
 * @ingroup strings
 * @param[in] component_type The string to convert
 * @return The corresponding ::ed247_component_type_t value
 */
extern LIBED247_EXPORT ed247_component_type_t ed247_component_type_from_string(
    const char *component_type);

/**
 * @brief ::ed247_stream_type_t to string conversion
 * @ingroup strings
 * @param[in] stream_type The value to convert
 * @return The corresponding string
 */
extern LIBED247_EXPORT const char * ed247_stream_type_string(
    ed247_stream_type_t stream_type);

/**
 * @brief ::ed247_stream_type_t from string conversion
 * @ingroup strings
 * @param[in] stream_type The string to convert
 * @return The corresponding ::ed247_stream_type_t value
 */
extern LIBED247_EXPORT ed247_stream_type_t ed247_stream_type_from_string(
    const char *stream_type);

/**
 * @brief ::ed247_signal_type_t to string conversion
 * @ingroup strings
 * @param[in] signal_type The value to convert
 * @return The corresponding string
 */
extern LIBED247_EXPORT const char * ed247_signal_type_string(
    ed247_signal_type_t signal_type);

/**
 * @brief ::ed247_signal_type_t from string conversion
 * @ingroup strings
 * @param[in] signal_type The string to convert
 * @return The corresponding ::ed247_signal_type_t value
 */
extern LIBED247_EXPORT ed247_signal_type_t ed247_signal_type_from_string(
    const char *signal_type);

/**
 * @brief ::ed247_nad_type_t to string conversion
 * @ingroup strings
 * @param[in] nad_type The value to convert
 * @return The corresponding string
 */
extern LIBED247_EXPORT const char * ed247_nad_type_string(
    ed247_nad_type_t nad_type);

/**
 * @brief ::ed247_nad_type_t from string conversion
 * @ingroup strings
 * @param[in] nad_type The string to convert
 * @return The corresponding ::ed247_nad_type_t value
 */
extern LIBED247_EXPORT ed247_nad_type_t ed247_nad_type_from_string(
    const char *nad_type);


/* =========================================================================
 * Deprecated stuff
 * ========================================================================= */
/**
 * @defgroup deprecated
 * Do not use.
 */

#if defined(__GNUC__) || defined(__clang__)
# define DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
# define DEPRECATED __declspec(deprecated)
#endif

/**
 * @brief Deprecated. Return NULL.
 * @ingroup deprecated
 */
extern DEPRECATED LIBED247_EXPORT const char * libed247_errors();

/**
 * @brief Deprecated: use ed247_load_file.
 * @ingroup deprecated
 */
extern DEPRECATED LIBED247_EXPORT ed247_status_t ed247_load(const char *ecic_file_path,void* unused,ed247_context_t *context);

/**
 * @brief Deprecated: use ed247_get_channel_list.
 * @ingroup deprecated
 */
extern DEPRECATED LIBED247_EXPORT ed247_status_t ed247_component_get_channels(ed247_context_t context, ed247_channel_list_t * channels);

/**
 * @brief Deprecated: use ed247_get_stream_list.
 * @ingroup deprecated
 */
extern DEPRECATED LIBED247_EXPORT ed247_status_t ed247_component_get_streams(ed247_context_t context, ed247_stream_list_t * streams);

/**
 * @brief Deprecated: use ed247_channel_get_stream_list
 * @ingroup deprecated
 */
extern DEPRECATED LIBED247_EXPORT ed247_status_t ed247_channel_get_streams(ed247_channel_t channel, ed247_stream_list_t *streams);

/**
 * @brief Deprecated: use ed247_channel_find_streams
 * @ingroup deprecated
 */
extern DEPRECATED LIBED247_EXPORT ed247_status_t ed247_find_channel_streams(ed247_channel_t channel, const char * regex_name, ed247_stream_list_t * streams);

/**
 * @brief Deprecated: use ed247_channel_get_stream.
 * @ingroup deprecated
 */
extern DEPRECATED LIBED247_EXPORT ed247_status_t ed247_get_channel_stream(ed247_channel_t channel, const char * name, ed247_stream_t * stream);

/**
 * @brief Deprecated: use ed247_stream_has_signals.
 * @ingroup deprecated
 */
extern DEPRECATED LIBED247_EXPORT ed247_status_t ed247_stream_contains_signals(ed247_stream_t stream, uint8_t * yesno);

/**
 * @brief Deprecated: use ed247_stream_get_signal_list.
 * @ingroup deprecated
 */
extern DEPRECATED LIBED247_EXPORT ed247_status_t ed247_stream_get_signals(ed247_stream_t stream, ed247_signal_list_t *signals);

/**
 * @brief Deprecated: use ed247_stream_find_signals.
 * @ingroup deprecated
 */
extern DEPRECATED LIBED247_EXPORT ed247_status_t ed247_find_stream_signals(ed247_stream_t stream, const char * regex_name, ed247_signal_list_t * signals);

/**
 * @brief Deprecated: use ed247_stream_get_signal.
 * @ingroup deprecated
 */
extern DEPRECATED LIBED247_EXPORT ed247_status_t ed247_get_stream_signal(ed247_stream_t stream, const char * name, ed247_signal_t * signal);

/**
 * @brief Deprecated: use ed247_signal_free_sample or ed247_stream_free_sample.
 * @ingroup deprecated
 */
extern DEPRECATED LIBED247_EXPORT ed247_status_t ed247_free(void *data);



#ifdef __cplusplus
};
#endif

#endif
