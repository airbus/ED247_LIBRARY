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

#ifndef _LIBED247_H_
#define _LIBED247_H_

#ifdef __cplusplus
extern "C" {
#endif

/************
 * Includes *
 ************/

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

/***********
 * Defines *
 ***********/

#ifdef __linux__
#define LIBED247_EXPORT __attribute__ ((visibility ("default")))
#elif _WIN32
#ifdef LIBED247_EXPORTS
#define  LIBED247_EXPORT __declspec(dllexport)
#else
#define  LIBED247_EXPORT __declspec(dllimport)
#endif
#endif

/**
 * @defgroup common General
 * General resources.
 */

/**
 * @defgroup errors Errors
 * Error handling resources.
 */

/**
 * @defgroup load_unload Load & Unload
 * Loading, initializing & unloading the context.
 */

/**
 * @defgroup configuration Configuration
 * All the resources needed to retrieve pieces of information from the configuration file.
 */

/**
 * @defgroup component Component
 * @ingroup configuration
 * Resources linked to the ED247 component designated by the context.
 */

/**
 * @defgroup channel Channel
 * @ingroup configuration
 * Resources linked to the ED247 channels.
 */

/**
 * @defgroup stream Stream
 * @ingroup configuration
 * Resources linked to the ED247 streams.
 */

/**
 * @defgroup signal Signal
 * @ingroup configuration
 * Resources linked to the ED247 signals.
 */

/**
 * @defgroup read_write Read & Write
 * Read & write data from/to streams & signals.
 */

/**
 * @defgroup send_recv Send & Receive
 * Send & receive ED247 frames.
 */

/**
 * @defgroup time Time handling
 * Time considerations
 */

/**
 * @defgroup advanced Advanced features
 * Optional features such as frame encoding & decoding for custom frame send / recv.
 */

/*********
 * Types *
 *********/

/**
 * @brief Status codes
 * @ingroup error
 */
typedef enum {
    ED247_STATUS_SUCCESS = EXIT_SUCCESS,    // Success
    ED247_STATUS_FAILURE = EXIT_FAILURE,    // Failure
    ED247_STATUS_TIMEOUT,
    ED247_STATUS_NODATA,
    ED247_STATUS_STOP
} ed247_status_t;

/**
 * @brief Logging level
 * @ingroup common
 */
typedef enum {
    ED247_LOG_LEVEL_ERROR = 0,
    ED247_LOG_LEVEL_WARNING,
    ED247_LOG_LEVEL_INFO,
    ED247_LOG_LEVEL_DEBUG,
    ED247_LOG_LEVEL__INVALID
} ed247_log_level_t;

/**
 * @brief Yes / No
 * @ingroup common
 */
typedef enum {
    ED247_YESNO_NO = 0,
    ED247_YESNO_YES,
    ED247_YESNO__INVALID
} ed247_yesno_t;

/**
 * @brief ED247 Standard revisions
 * @ingroup common
 */
typedef enum {
    ED247_STANDARD__INVALID,
    ED247_STANDARD_ED247,
    ED247_STANDARD_ED247A,
    ED247_STANDARD__COUNT
} ed247_standard_t;

/**
 * @brief Component types
 * @ingroup component
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
 * @brief Signal types, with reference to stream values of ::ed247_stream_type_t
 * @ingroup signal
 */
typedef enum {
    ED247_SIGNAL_TYPE__INVALID,
    ED247_SIGNAL_TYPE_ANALOG = ED247_STREAM_TYPE_ANALOG,
    ED247_SIGNAL_TYPE_DISCRETE = ED247_STREAM_TYPE_DISCRETE,
    ED247_SIGNAL_TYPE_NAD = ED247_STREAM_TYPE_NAD,
    ED247_SIGNAL_TYPE_VNAD = ED247_STREAM_TYPE_VNAD
} ed247_signal_type_t;

/**
 * @brief Stream direction
 * @ingroup stream
 */
typedef enum {
    ED247_DIRECTION__INVALID = 0,
    ED247_DIRECTION_IN = 1,
    ED247_DIRECTION_OUT = 2,
    ED247_DIRECTION_INOUT = 3,
    ED247_DIRECTION__COUNT
} ed247_direction_t;

/**
 * @brief Unique identifier type
 * @ingroup stream
 */
typedef uint16_t ed247_uid_t;

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
    ED247_DISCRETE_INVALID = 0xFE,
    ED247_DISCRETE_TRUE = 0xFF
} ed247_discrete_t;

/**
 * @brief Library configuration structure used during load
 * @ingroup common
 */
typedef struct {
    uint8_t enable_logs_during_send_receive;
    const char *  log_filepath;
    ed247_log_level_t log_level;
} libed247_configuration_t;
#define LIBED247_CONFIGURATION_DEFAULT {0, NULL, ED247_LOG_LEVEL__INVALID}

/**
 * @brief Library runtime metrics
 * @ingroup common
 */
typedef struct {
    uint32_t missed_frames;
    uint32_t sample_timestamp_offset_overflows;
} libed247_runtime_metrics_t;
#define LIBED247_RUNTIME_METRICS_DEFAULT {0, 0}

/**
 * @brief Frame format
 * @ingroup channel
 */
typedef struct ed247_frame_format_s {
    ed247_standard_t standard_revision;
} ed247_frame_format_t;
#define LIBED247_FRAME_FORMAT_DEFAULT {ED247_STANDARD__INVALID}

/**
 * @brief Timestamp structure, seconds from EPOCH (January 1st 1970) and nanoseconds offset with reference to previous field
 * @ingroup common
 */
typedef struct {
    uint32_t epoch_s;
    uint32_t offset_ns;
} ed247_timestamp_t;
#define LIBED247_TIMESTAMP_DEFAULT {0, 0}

/**
 * @brief Component information structure
 * @ingroup component
 */
typedef struct ed247_component_info_s {
    const char * component_version;
    ed247_component_type_t component_type;
    const char * name;
    const char * comment;
    ed247_standard_t standard_revision;
    ed247_uid_t identifier;
    struct {
        const char * identifier;
        const char * comment;
    } file_producer;
} ed247_component_info_t;
#define LIBED247_COMPONENT_INFO_DEFAULT {"", ED247_COMPONENT_TYPE_VIRTUAL, NULL, "", ED247_STANDARD__INVALID, 0, {"", ""}}

/**
 * @brief Channel information structure
 * @ingroup channel
 */
typedef struct ed247_channel_info_s {
    const char * name;
    const char * comment;
    ed247_frame_format_t frame_format;
} ed247_channel_info_t;
#define LIBED247_CHANNEL_INFO_DEFAULT {NULL, "", LIBED247_FRAME_FORMAT_DEFAULT}

/**
 * @brief Stream DISCRETE dedicated information structure
 * @ingroup stream
 */

typedef struct {
    uint32_t sampling_period_us;
} ed247_stream_info_dis_t;
#define LIBED247_STREAM_INFO_DIS_DEFAULT {0}

/**
 * @brief Stream ANALOG dedicated information structure
 * @ingroup stream
 */

typedef struct {
    uint32_t sampling_period_us;
} ed247_stream_info_ana_t;
#define LIBED247_STREAM_INFO_ANA_DEFAULT {0}

/**
 * @brief Stream NAD dedicated information structure
 * @ingroup stream
 */

typedef struct {
    uint32_t sampling_period_us;
} ed247_stream_info_nad_t;
#define LIBED247_STREAM_INFO_NAD_DEFAULT {0}

/**
 * @brief Stream VNAD dedicated information structure
 * @ingroup stream
 */

typedef struct {
    uint32_t sampling_period_us;
} ed247_stream_info_vnad_t;
#define LIBED247_STREAM_INFO_VNAD_DEFAULT {0}

/**
 * @brief Stream dedicated information structures
 * @ingroup stream
 */

typedef union {
    ed247_stream_info_dis_t dis;
    ed247_stream_info_ana_t ana;
    ed247_stream_info_nad_t nad;
    ed247_stream_info_vnad_t vnad;
} ed247_stream_info_type_t;

/**
 * @brief Stream information structure
 * @ingroup stream
 */
typedef struct {
    const char * name;
    ed247_direction_t direction;
    ed247_stream_type_t type;
    const char * comment;
    const char * icd;
    ed247_uid_t uid;
    size_t sample_max_number;
    size_t sample_max_size_bytes;
    ed247_stream_info_type_t info;
} ed247_stream_info_t;
#define LIBED247_STREAM_INFO_DEFAULT {NULL, ED247_DIRECTION__INVALID, ED247_STREAM_TYPE__INVALID, "", "", 0, 1, 1, {}}

/**
 * @brief DISCRETE signal dedicated information subpart
 * @ingroup signal
 */
typedef struct {
    uint32_t byte_offset;
} ed247_signal_info_dis_t;
#define LIBED247_SIGNAL_INFO_DIS_DEFAULT {0}

/**
 * @brief ANALOG signal dedicated information subpart
 * @ingroup signal
 */
typedef struct {
    uint32_t byte_offset;
    const char * electrical_unit;
} ed247_signal_info_ana_t;
#define LIBED247_SIGNAL_INFO_ANA_DEFAULT {0, ""}

/**
 * @brief NAD signal dedicated information subpart
 * @ingroup signal
 */
typedef struct {
    uint32_t byte_offset;
    ed247_nad_type_t nad_type;
    const char * unit;
    uint32_t *dimensions;
    uint32_t dimensions_count;
} ed247_signal_info_nad_t;
#define LIBED247_SIGNAL_INFO_NAD_DEFAULT {0, ED247_NAD_TYPE__INVALID, "", NULL, 0}

/**
 * @brief VNAD signal dedicated information subpart
 * @ingroup signal
 */
typedef struct {
    ed247_nad_type_t nad_type;
    uint32_t position;
    const char * unit;
    uint32_t max_length;
} ed247_signal_info_vnad_t;
#define LIBED247_SIGNAL_INFO_VNAD_DEFAULT {ED247_NAD_TYPE__INVALID, 0, "", 1}

/**
 * @brief Signal type dedicatd information subpart
 * @ingroup signal
 */
typedef union {
    ed247_signal_info_dis_t dis;
    ed247_signal_info_ana_t ana;
    ed247_signal_info_nad_t nad;
    ed247_signal_info_vnad_t vnad;
} ed247_signal_info_type_t;

/**
 * @brief Signal information structure
 * @ingroup signal
 */
typedef struct ed247_signal_info_s {
    const char * name;
    ed247_signal_type_t type;
    const char * comment;
    const char * icd;
    ed247_signal_info_type_t info;
} ed247_signal_info_t;

/**
 * @brief Sample information
 * @ingroup stream
 */
typedef struct ed247_sample_info_s {
    ed247_uid_t component_identifier;
    uint16_t sequence_number;
    ed247_timestamp_t transport_timestamp;
} ed247_sample_info_t;
#define LIBED247_SAMPLE_INFO_DEFAULT {0, 0, {0, 0}}

/**
 * @brief Context identifier
 * @ingroup common
 */
typedef struct ed247_internal_context_t *ed247_context_t;

/**
 * @brief Channel identifier
 * @ingroup channel
 */
typedef struct ed247_internal_channel_t *ed247_channel_t;

/**
 * @brief Channel list identifier
 * @ingroup channel
 */
typedef struct ed247_internal_channel_list_t *ed247_channel_list_t;

/**
 * @brief Stream identifier
 * @ingroup stream
 */
typedef struct ed247_internal_stream_t *ed247_stream_t;

/**
 * @brief Stream list identifier
 * @ingroup stream
 */
typedef struct ed247_internal_stream_list_t *ed247_stream_list_t;

/**
 * @brief Signal identifier
 * @ingroup signal
 */
typedef struct ed247_internal_signal_t *ed247_signal_t;

/**
 * @brief Signal list identifier
 * @ingroup signal
 */
typedef struct ed247_internal_signal_list_t *ed247_signal_list_t;

/**
 * @brief Frame list identifier
 * @ingroup send_recv
 */
typedef struct ed247_internal_frame_list_t *ed247_frame_list_t;

/**
 * @brief An assistant to help building stream samples
 * @ingroup signal
 */
typedef struct ed247_internal_stream_assistant_t *ed247_stream_assistant_t;

/**
 * @brief Time basis to update in simulation time handler
 * @ingroup common
 */
typedef struct ed247_internal_time_sample_t *ed247_time_sample_t;

/**
 * @brief Pointer to the function called when timestamping at reception (simulation time)
 * @ingroup common
 */
typedef ed247_status_t (*libed247_set_simulation_time_ns_t)(ed247_time_sample_t time_sample, void *user_data);

/**
 * @brief Channel & frame data
 * @ingroup send_recv
 */
typedef struct {
    ed247_channel_t channel;
    void *data;
    size_t size;
} ed247_frame_t;

/**********
 * Common *
 **********/

/**
 * @brief Retrieve the error messages. If there is no error message, the returned message is "No error".
 * @ingroup errors
 * @return Error messages
 */
extern LIBED247_EXPORT const char * libed247_errors();

/**
 * @brief ::ed247_status_t to string conversion
 * @ingroup common
 * @param[in] status The value to convert
 * @return The corresponding string
 */
extern LIBED247_EXPORT const char * ed247_status_string(
    ed247_status_t status);

/**
 * @brief ::ed247_standard_t to string conversion
 * @ingroup common
 * @param[in] standard The value to convert
 * @return The corresponding string
 */
extern LIBED247_EXPORT const char * ed247_standard_string(
    ed247_standard_t standard);

/**
 * @brief ::ed247_standard_t from string conversion
 * @ingroup common
 * @param[in] standard The string to convert
 * @return The corresponding ::ed247_standard_t value
 */
extern LIBED247_EXPORT ed247_standard_t ed247_standard_from_string(
    const char *standard);

/**
 * @brief ::ed247_direction_t to string conversion
 * @ingroup common
 * @param[in] direction The value to convert
 * @return The corresponding string
 */
extern LIBED247_EXPORT const char * ed247_direction_string(
    ed247_direction_t direction);

/**
 * @brief ::ed247_direction_t from string conversion
 * @ingroup common
 * @param[in] direction The string to convert
 * @return The corresponding ::ed247_direction_t value
 */
extern LIBED247_EXPORT ed247_direction_t ed247_direction_from_string(
    const char *direction);

/**
 * @brief ::ed247_yesno_t to string conversion
 * @ingroup common
 * @param[in] yesno The value to convert
 * @return The corresponding string
 */
extern LIBED247_EXPORT const char * ed247_yesno_string(
    ed247_yesno_t yesno);

/**
 * @brief ::ed247_yesno_t from string conversion
 * @ingroup common
 * @param[in] yesno The string to convert
 * @return The corresponding ::ed247_yesno_t value
 */
extern LIBED247_EXPORT ed247_yesno_t ed247_yesno_from_string(
    const char *yesno);

/**
 * @brief ::ed247_component_type_t to string conversion
 * @ingroup common
 * @param[in] component_type The value to convert
 * @return The corresponding string
 */
extern LIBED247_EXPORT const char * ed247_component_type_string(
    ed247_component_type_t component_type);

/**
 * @brief ::ed247_component_type_t from string conversion
 * @ingroup common
 * @param[in] component_type The string to convert
 * @return The corresponding ::ed247_component_type_t value
 */
extern LIBED247_EXPORT ed247_component_type_t ed247_component_type_from_string(
    const char *component_type);

/**
 * @brief ::ed247_stream_type_t to string conversion
 * @ingroup common
 * @param[in] stream_type The value to convert
 * @return The corresponding string
 */
extern LIBED247_EXPORT const char * ed247_stream_type_string(
    ed247_stream_type_t stream_type);

/**
 * @brief ::ed247_stream_type_t from string conversion
 * @ingroup common
 * @param[in] stream_type The string to convert
 * @return The corresponding ::ed247_stream_type_t value
 */
extern LIBED247_EXPORT ed247_stream_type_t ed247_stream_type_from_string(
    const char *stream_type);

/**
 * @brief ::ed247_signal_type_t to string conversion
 * @ingroup common
 * @param[in] signal_type The value to convert
 * @return The corresponding string
 */
extern LIBED247_EXPORT const char * ed247_signal_type_string(
    ed247_signal_type_t signal_type);

/**
 * @brief ::ed247_signal_type_t from string conversion
 * @ingroup common
 * @param[in] signal_type The string to convert
 * @return The corresponding ::ed247_signal_type_t value
 */
extern LIBED247_EXPORT ed247_signal_type_t ed247_signal_type_from_string(
    const char *signal_type);

/**
 * @brief ::ed247_nad_type_t to string conversion
 * @ingroup common
 * @param[in] nad_type The value to convert
 * @return The corresponding string
 */
extern LIBED247_EXPORT const char * ed247_nad_type_string(
    ed247_nad_type_t nad_type);

/**
 * @brief ::ed247_nad_type_t from string conversion
 * @ingroup common
 * @param[in] nad_type The string to convert
 * @return The corresponding ::ed247_nad_type_t value
 */
extern LIBED247_EXPORT ed247_nad_type_t ed247_nad_type_from_string(
    const char *nad_type);

/**
 * @brief Size of a single element of ::ed247_nad_type_t
 * @ingroup common
 * @param[in] nad_type The NAD type
 * @return The size of the NAD type element (sizeof equivalent)
 */
extern LIBED247_EXPORT size_t ed247_nad_type_size(
    ed247_nad_type_t nad_type);

/*********
 * Lists *
 *********/

/**
 * @brief Iterate over a channel identifier list
 * @ingroup channel
 * @param[in,out] channels The channel list
 * @param[out] channel A pointer to the current item in the list.
 * A /a null value is set when the end of the list is reached.
 * The next call will return the pointer to the first item of the list.
 * @retval ED247_STATUS_SUCCESS Operation completed successfully (although end of list may be reached)
 * @retval ED247_STATUS_FAILURE Invalid parameter provided or internal error
 */
extern LIBED247_EXPORT ed247_status_t ed247_channel_list_next(
    ed247_channel_list_t channels,
    ed247_channel_t *channel);

/**
 * @brief Free channel list
 * <b>Do not use during runtime. The implementation may contain memory allocation/deallocation functions.</b>
 * @ingroup channel
 * @param[in] channels The channel list
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_channel_list_free(
    ed247_channel_list_t channels);

/**
 * @brief Get the size of the list
 * @ingroup channel
 * @param[in] channels The channel list
 * @param[out] size The size of the list
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_channel_list_size(
    ed247_channel_list_t channels,
    size_t * size);

/**
 * @brief Iterate over a stream identifier list
 * @ingroup stream
 * @param[in,out] streams The stream list
 * @param[out] stream A pointer to the current item in the list.
 * A /a null value is set when the end of the list is reached.
 * The next call will return the pointer to the first item of the list.
 * @retval ED247_STATUS_SUCCESS Operation completed successfully (although end of list may be reached)
 * @retval ED247_STATUS_FAILURE Invalid parameter provided or internal error
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_list_next(
    ed247_stream_list_t streams,
    ed247_stream_t *stream);

/**
 * @brief Free stream list
 * <b>Do not use during runtime. The implementation may contain memory allocation/deallocation functions.</b>
 * @ingroup stream
 * @param[in] streams The stream list
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_list_free(
    ed247_stream_list_t streams);

/**
 * @brief Get the size of the list
 * @ingroup stream
 * @param[in] streams The stream list
 * @param[out] size The size of the list
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_list_size(
    ed247_stream_list_t streams,
    size_t * size);

/**
 * @brief Iterate over a signal identifier list
 * @ingroup signal
 * @param[in,out] signals The signal list
 * @param[out] signal A pointer to the current item in the list.
 * A /a null value is set when the end of the list is reached.
 * The next call will return the pointer to the first item of the list.
 * @retval ED247_STATUS_SUCCESS Operation completed successfully (although end of list may be reached)
 * @retval ED247_STATUS_FAILURE Invalid parameter provided or internal error
 */
extern LIBED247_EXPORT ed247_status_t ed247_signal_list_next(
    ed247_signal_list_t signals,
    ed247_signal_t *signal);

/**
 * @brief Free signal list
 * <b>Do not use during runtime. The implementation may contain memory allocation/deallocation functions.</b>
 * @ingroup signal
 * @param[in] signals The signal list
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_signal_list_free(
    ed247_signal_list_t signals);

/**
 * @brief Get the size of the list
 * @ingroup signal
 * @param[in] signals The signal list
 * @param[out] size The size of the list
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_signal_list_size(
    ed247_signal_list_t signals,
    size_t * size);

/**
 * @brief Iterate over a frame identifier list
 * @ingroup send_recv
 * @param[in,out] frames The signal list
 * @param[out] frame A pointer to the current item in the list.
 * A /a null value is set when the end of the list is reached.
 * The next call will return the pointer to the first item of the list.
 * @retval ED247_STATUS_SUCCESS Operation completed successfully (although end of list may be reached)
 * @retval ED247_STATUS_FAILURE Invalid parameter provided or internal error
 */
extern LIBED247_EXPORT ed247_status_t ed247_frame_list_next(
    ed247_frame_list_t frames,
    const ed247_frame_t ** frame);

/**
 * @brief Free frame list
 * <b>Do not use during runtime. The implementation may contain memory allocation/deallocation functions.</b>
 * @ingroup send_recv
 * @param[in] frames The frame list
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_frame_list_free(
    ed247_frame_list_t frames);

/**
 * @brief Get the size of the list
 * @ingroup frame
 * @param[in] frames The frame list
 * @param[out] size The size of the list
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_frame_list_size(
    ed247_frame_list_t frames,
    size_t * size);

/******************
 * Initialization *
 ******************/

/**
 * @brief The name of the current implementation
 * @ingroup common
 * @return Current implementation name
 */
extern LIBED247_EXPORT const char * ed247_get_implementation_name();

/**
 * @brief The version of the current implementation
 * @ingroup common
 * @return Current implementation version
 */
extern LIBED247_EXPORT const char * ed247_get_implementation_version();

/**
 * @brief Retrieve runtime metrics
 * @ingroup common
 * @param[in] context The context for which the runtime metrics are required
 * @param[out] metrics Pointer to the runtime metrics structure
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_get_runtime_metrics(
    ed247_context_t context,
    const libed247_runtime_metrics_t ** metrics);

/**
 * @brief Setup the logging level (see ::ed247_log_level_t)
 * @ingroup common
 * @param[in] Logging level
 * @retval ED247_STATUS_SUCCESS
 */
extern LIBED247_EXPORT ed247_status_t ed247_set_log_level(
    ed247_log_level_t log_level);

/**
 * @brief Get the logging level (see ::ed247_log_level_t)
 * @ingroup common
 * @param[out] Logging level
 * @retval ED247_STATUS_SUCCESS
 */
extern LIBED247_EXPORT ed247_status_t ed247_get_log_level(
    ed247_log_level_t *log_level);

/**
 * @brief Loading function: the entry point of the library
 * @ingroup load_unload
 * @param[in] ecic_file_path The path to the ECIC configuration file
 * @param[in] configuration The configuration of the LIBED247
 * @param[out] context The loaded context identifier
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE An error occurred during the load phase (xml parsing or internal loading)
  */
extern LIBED247_EXPORT ed247_status_t ed247_load(
    const char *ecic_file_path,
    const libed247_configuration_t *configuration,
    ed247_context_t *context);

/**
 * @brief Loading function: the entry point of the library
 * @ingroup load_unload
 * @param[in] ecic_file_content The content of the ECIC configuration file
 * @param[in] configuration The configuration of the LIBED247
 * @param[out] context The loaded context identifier
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE An error occurred during the load phase (xml parsing or internal loading)
  */
extern LIBED247_EXPORT ed247_status_t ed247_load_content(
    const char *ecic_file_content,
    const libed247_configuration_t *configuration,
    ed247_context_t *context);

/********
 * Time *
 ********/

/**
 * @brief Register a callback used to timestamp sample at recpection (recv_timestamp)
 * <b>The registered function is called each time the simulation time is needed, in each stream receiving data. It is strongly encouraged to perform a manual increase of simulation time and not clock queries each time the function is called as this might lead to a high execution time.</b>
 * @ingroup time
 * @param[in] handler Handler to the function
 * @param[in] user_data Pointer to custom data
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t libed247_register_set_simulation_time_ns_handler(
    libed247_set_simulation_time_ns_t handler,
    void *user_data);

/**
 * @brief Default function to retrieve current time
 * If not overrided by the user, this function is the default one registered by libed247_register_set_simulation_time_ns_handler().
 * @ingroup time
 * @param[out] time_sample Time sample to set
 * @param[out] user_data Pointer to custom data
 * @return ED247_STATUS_SUCCESS
 * @return ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t libed247_set_simulation_time_ns(
    ed247_time_sample_t time_sample,
    void *user_data);

/**
 * @brief Update time in Simulation time calllback
 * <b>This function must be called to update component simulation time in the libed247_set_simulation_time_ns_t handler.</b>
 * @ingroup time
 * @param[in] time_sample Time sample
 * @param[in] epoch_s Number of seconds since epoch
 * @param[in] offset_ns Offset with reference to epoch_s, in nanoseconds
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t libed247_update_time(
    ed247_time_sample_t time_sample,
    uint32_t epoch_s,
    uint32_t offset_ns);

/***********
 * Finders *
 ***********/

/**
 * @brief Find all channels of the component whose name is matching the regular expression
 * <em>For example, to get a list of all the channels, use the /a * value.</em>
 * <em>The regex uses the <b>ECMAScript</b> grammar.</em>
 * <em>The regex do not embed implicit /a .* special characters at the beginning and the end.</em>
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * @ingroup channel
 * @param[in] context The context identifier.
 * @param[in] regex_name The regular expression for name matching. If null, assume '.*'.
 * @param[out] channels The list of the channels. If no value, set to null.
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE The channel list is empty
 * @note The regular expressions are not implemented in gcc4.8.x (default linux), do not use complex requests.
 */
extern LIBED247_EXPORT ed247_status_t ed247_find_channels(
    ed247_context_t context,
    const char *regex_name,
    ed247_channel_list_t *channels);

/**
 * @brief Find a channel of the component
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * @ingroup channel
 * @param[in] context The context identifier.
 * @param[in] name The name of the channel.
 * @param[out] channel The channel identifier
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE The channel list is empty
 */
extern LIBED247_EXPORT ed247_status_t ed247_get_channel(
    ed247_context_t context,
    const char *name,
    ed247_channel_t *channel);

/**
 * @brief Find all streams of the component whose name is matching the regular expression
 * <em>For example, to get a list of all the streams, use the /a * value.</em>
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * @ingroup stream
 * @param[in] context The context identifier
 * @param[in] regex_name The regular expression for name matching. If null, assume '.*'.
 * @param[out] streams The list of the streams. If no value, set to null.
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE The stream list is empty
 * @note The regular expressions are not implemented in gcc4.8.x (default linux), do not use complex requests.
 */
extern LIBED247_EXPORT ed247_status_t ed247_find_streams(
    ed247_context_t context,
    const char *regex_name,
    ed247_stream_list_t *streams);

/**
 * @brief Find a stream of the component
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * @ingroup stream
 * @param[in] context The context identifier
 * @param[in] name The name of the stream.
 * @param[out] stream The stream identifier
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE The stream list is empty
 */
extern LIBED247_EXPORT ed247_status_t ed247_get_stream(
    ed247_context_t context,
    const char *name,
    ed247_stream_t *stream);

/**
 * @brief Find all streams of the channel whose name is matching the regular expression
 * <em>For example, to get a list of all the streams, use the /a * value.</em>
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * @ingroup stream
 * @param[in] channel The channel identifier
 * @param[in] regex_name The regular expression for name matching. If null, assume '.*'.
 * @param[out] streams The list of the streams. If no value, set to null.
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE The stream list is empty
 * @note The regular expressions are not implemented in gcc4.8.x (default linux), do not use complex requests.
 */
extern LIBED247_EXPORT ed247_status_t ed247_find_channel_streams(
    ed247_channel_t channel,
    const char *regex_name,
    ed247_stream_list_t *streams);

/**
 * @brief Find a stream in a channel
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * @ingroup stream
 * @param[in] channel The channel identifier
 * @param[in] name The name of the stream.
 * @param[out] stream The stream identifier.
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE The stream list is empty
 */
extern LIBED247_EXPORT ed247_status_t ed247_get_channel_stream(
    ed247_channel_t channel,
    const char *name,
    ed247_stream_t *stream);

/**
 * @brief Find all signals of the component whose name is matching the regular expression
 * <em>For example, to get a list of all the signals, use the /a * value.</em>
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * @ingroup signal
 * @param[in] context The context identifier
 * @param[in] regex_name The regular expression for name matching. If null, assume '.*'.
 * @param[out] signals The list of the signals. If no value, set to null.
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE The stream list is empty
 * @note The regular expressions are not implemented in gcc4.8.x (default linux), do not use complex requests.
 */
extern LIBED247_EXPORT ed247_status_t ed247_find_signals(
    ed247_context_t context,
    const char *regex_name,
    ed247_signal_list_t *signals);

/**
 * @brief Find a signal of the component
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * @ingroup signal
 * @param[in] context The context identifier
 * @param[in] name The signal name.
 * @param[out] signal The signal identifier.
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE The stream list is empty
 */
extern LIBED247_EXPORT ed247_status_t ed247_get_signal(
    ed247_context_t context,
    const char *name,
    ed247_signal_t *signal);

/**
 * @brief Find all signals of the stream whose name is matching the regular expression
 * <em>For example, to get a list of all the signals, use the /a * value.</em>
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * @ingroup signal
 * @param[in] stream The stream identifier
 * @param[in] regex_name The regular expression for name matching. If null, assume '.*'.
 * @param[out] signals The list of the signals. If no value, set to null.
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE The stream list is empty
 * @note The regular expressions are not implemented in gcc4.8.x (default linux), do not use complex requests.
 */
extern LIBED247_EXPORT ed247_status_t ed247_find_stream_signals(
    ed247_stream_t stream,
    const char *regex_name,
    ed247_signal_list_t *signals);

/**
 * @brief Get a signal of the stream
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * @ingroup signal
 * @param[in] stream The stream identifier
 * @param[in] name The stream name.
 * @param[out] signal The signal identifier.
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE The stream list is empty
 */
extern LIBED247_EXPORT ed247_status_t ed247_get_stream_signal(
    ed247_stream_t stream,
    const char *name,
    ed247_signal_t *signal);

/***********
 * Getters *
 ***********/

/**
 * @brief Retrieve attributes of the component
  * @ingroup component
 * @param[in] context The context identifier
 * @param[out] info Component information
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_component_get_info(
    ed247_context_t context,
    const ed247_component_info_t **info);

/**
 * @brief Retrieve all the streams of the component
 * @ingroup component
 * @param[in] context The context identifier
 * @param[out] streams List of the streams
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_component_get_streams(
    ed247_context_t context,
    ed247_stream_list_t *streams);

/**
 * @brief Retrieve all the channels of the component
 * @ingroup component
 * @param[in] context The context identifier
 * @param[out] channels List of the channels
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_component_get_channels(
    ed247_context_t context,
    ed247_channel_list_t *channels);

/**
 * @brief Assign user data to the context
 * <b>When unloading the component, there is no memory free on this item. Free it yourself.</b>
 * @ingroup component
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
 * @ingroup component
 * @param[in] context The context identifier
 * @param[out] user_data A pointer to host pointer to user data
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_component_get_user_data(
    ed247_context_t context,
    void **user_data);

/**
 * @brief Retrieve attributes of the channel
  * @ingroup channel
 * @param[in] channel The channel identifier
 * @param[out] info Channel information
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_channel_get_info(
    ed247_channel_t channel,
    const ed247_channel_info_t **info);

/**
 * @brief Retrieve all the streams of the channel
 * @ingroup component
 * @param[in] channel The channel identifier
 * @param[out] streams List of the streams
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_channel_get_streams(
    ed247_channel_t channel,
    ed247_stream_list_t *streams);

/**
 * @brief Retrieve attributes of the stream
 * @ingroup stream
 * @param[in] stream The stream identifier
 * @param[out] info Stream information
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_get_info(
    ed247_stream_t stream,
    const ed247_stream_info_t **info);

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
    void *user_data);

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
    void **user_data);

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
    ed247_stream_t stream,
    ed247_channel_t *channel);

/**
 * @brief Retrieve attributes of the signal
  * @ingroup signal
 * @param[in] signal The signal identifier
 * @param[out] info Signal information
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_signal_get_info(
    ed247_signal_t signal,
    const ed247_signal_info_t **info);

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
    void *user_data);

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
    void **user_data);

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
    ed247_signal_t signal,
    ed247_stream_t *stream);

/**
 * @brief Return all signals of a stream
 * @ingroup signal
 * @param[in] stream The stream identifier
 * @param[in] regex_name The regular expression for name matching
 * @param[out] signals The list of the signals. If no value, set to null.
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE The stream list is empty
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_get_signals(
    ed247_stream_t stream,
    ed247_signal_list_t *signals);

/**
 * @brief Check if the stream is a signal based one (DISCRETE, ANALOG, NAD or VNAD)
 * @ingroup signal
 * @param[in] stream Stream identifier
 * @param[out] yesno Result of the check, true for yes and false for no
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_contains_signals(
    ed247_stream_t stream,
    uint8_t *yesno);

/**********************************
 * Stream & Signal sample helpers *
 **********************************/

/**
 * @brief Allocate memory of a stream sample with the right memory size. The size of the sample is deducted from the SampleMaxSizeBytes ED247 parameter of the stream.
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * <b>When not needed anymore, the memory free shall be made by the user with free(*sample_data).</b>
 * @ingroup read_write
 * @param[in] stream Stream identifier
 * @param[out] sample_data Pointer to the allocated memory
 * @param[out] sample_size Size of the memory allocated for the sample
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_allocate_sample(
    ed247_stream_t stream,
    void ** sample_data,
    size_t * sample_size);
    
/**
 * @brief Allocate memory of a signal sample with the right memory size. The size of the sample is deducted from the SampleMaxSizeBytes ED247 parameter of the signal.
 * <b>Do not use during runtime. The implementation may contain memory allocation functions.</b>
 * <b>When not needed anymore, the memory free shall be made by the user with free(*sample_data).</b>
 * @ingroup read_write
 * @param[in] signal Signal identifier
 * @param[out] sample_data Pointer to the allocated memory
 * @param[out] sample_size Size of the memory allocated for the sample
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_signal_allocate_sample(
    ed247_signal_t signal,
    void ** sample_data,
    size_t * sample_size);

/**
 * @brief Get an assistant to build stream samples based on signals.
 * @ingroup read_write
 * @param[in] stream Stream identifier
 * @param[out] assistant Stream assistant
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE When the stream is not signal based
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_get_assistant(
    ed247_stream_t stream,
    ed247_stream_assistant_t *assistant);

/**
 * @brief Get the stream associated to the assistant
 * @ingroup read_write
 * @param[in] assistant Assistant identifier
 * @param[out] stream Stream identifier pointer
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_assistant_get_stream(
    ed247_stream_assistant_t assistant,
    ed247_stream_t *stream);

/**
 * @brief Write signal sample into the assistant buffer. Once all signal are written, use ::ed247_stream_signal_assistant_push_sample() to push the sample on the stream.
 * This function returns an error if the stream direction is not ::ED247_DIRECTION_IN or ::ED247_DIRECTION_INOUT.
 * In case of DISCRETE Signal, the signal_sample_size equals to 1 byte
 * In case of ANALOG Signal, the signal_sample_size equals to 4 bytes (float). There is no need to swap the sample.
 * In case of NAD Signal, the signal_sample_size equals to the size of an atomic element, given by ed247_nad_type_size(nad_type), multiplied by all the dimensions (which is invariant). As a reminder, nad_type and dimensions can be retrieved with ::ed247_signal_get_info(). There is no need to swap the samples.
 * In case of VNAD Signal, the signal_sample_size equals to the size of an atomic element, given by ed247_nad_type_size(nad_type), multiplied by the effective size of the sample (which is variable). As a reminder, nad_type can be retrieved with ::ed247_signal_get_info(). There is no need to swap the samples.
 * @ingroup read_write
 * @param[in] assistant Assistant identifier
 * @param[in] signal Signal identifier
 * @param[in] signal_sample_data Retrieve pointer of allocated memory
 * @param[in] signal_sample_size Retrieve size of allocated memory
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_assistant_write_signal(
    ed247_stream_assistant_t assistant,
    ed247_signal_t signal,
    const void *signal_sample_data,
    size_t signal_sample_size);

/**
 * @brief Read signal sample from the assistant buffer. The assistant internal stream sample buffer is updated by calling ::ed247_stream_signal_assistant_pop_sample().
 * This function returns an error if the stream direction is not ::ED247_DIRECTION_OUT or ::ED247_DIRECTION_INOUT.
 * In case of DISCRETE Signal, the signal_sample_size equals to 1 byte
 * In case of ANALOG Signal, the signal_sample_size equals to 4 bytes (float). There is no need to swap the sample.
 * In case of NAD Signal, the signal_sample_size equals to the size of an atomic element, given by ed247_nad_type_size(nad_type), multiplied by all the dimensions (which is invariant). As a reminder, nad_type and dimensions can be retrieved with ::ed247_signal_get_info(). There is no need to swap the samples.
 * In case of VNAD Signal, the signal_sample_size equals to the size of an atomic element, given by ed247_nad_type_size(nad_type), multiplied by the effective size of the sample (which is variable). As a reminder, nad_type can be retrieved with ::ed247_signal_get_info(). There is no need to swap the samples.
 * @ingroup read_write
 * @param[in] assistant Assistant identifier
 * @param[in] signal Signal identifier
 * @param[in] signal_sample_data Retrieve pointer of the stream sample allocated in memory
 * @param[in] signal_sample_size Retrieve size of the stream sample allocated in memory
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_assistant_read_signal(
    ed247_stream_assistant_t assistant,
    ed247_signal_t signal,
    const void **signal_sample_data,
    size_t *signal_sample_size);

/*******************************
 * Read & write stream samples *
 *******************************/

/**
 * @brief Write & push a single sample in the sample buffer of the stream.
 * <b>This function may lead to the emission of frames according to packetization strategies.</b>
 * @ingroup read_write
 * @param[in] stream Stream identifier
 * @param[in] sample_data Sample data to write, copied internally
 * @param[in] sample_data_size Size of the sample data to write, in bytes
 * @param[in] data_timestamp Data timestamp associated to the sample. Set to NULL if not desired, otherwise see ::ed247_timestamp_t
 * @param[out] full Returns true if the internal stack is full. Set to NULL if not desired
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_push_sample(
    ed247_stream_t stream,
    const void *sample_data,
    size_t sample_data_size,
    const ed247_timestamp_t *data_timestamp,
    bool *full);

/**
 * @brief Write several stream samples at once.
 * <b>This function may lead to the emission of frames according to packetization strategies.</b>
 * @ingroup read_write
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
    ed247_stream_t stream,
    const void *samples_data,
    const size_t *samples_size,
    size_t samples_number,
    const ed247_timestamp_t *data_timestamp,
    bool *full);

/**
 * @brief Push the stream sample to the stream
 * <b>This function may lead to the emission of frames according to packetization strategies.</b>
 * @ingroup read_write
 * @param[in] assistant Assistant identifier
 * @param[in] data_timestamp Data timestamp associated to the sample. Set to NULL if not desired, otherwise see ::ed247_timestamp_t
 * @param[out] full Returns true if the internal stack is full. Set to NULL if not desired
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_assistant_push_sample(
    ed247_stream_assistant_t assistant,
    const ed247_timestamp_t *data_timestamp,
    bool *full);

/**
 * @brief Read & pop a sample received on the stream.
 * @ingroup read_write
 * @param[in] stream Stream identifier
 * @param[out] sample_data Pointer on the internal stream buffer sample
 * @param[out] sample_size Size of the received sample data
 * @param[out] data_timestamp Pointer on the internal buffer sample data timestamp, set to NULL if not received
 * @param[out] recv_timestamp Pointer on the internal buffer recv timestamp.
 * The value is set according to ::libed247_set_simulation_time_ns() callback that has to be specified by the user.
 * If no callback is specified, the pointer is set to NULL
 * @param[out] info Stream sample additional information
 * @param[out] empty Returns true if the internal stack is empty
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_NODATA Receive stack is empty
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_pop_sample(
    ed247_stream_t stream,
    const void **sample_data,
    size_t *sample_size,
    const ed247_timestamp_t **data_timestamp,
    const ed247_timestamp_t **recv_timestamp,
    const ed247_sample_info_t **info,
    bool *empty);

/**
 * @brief Read & pop a sample received on the stream.
 * @ingroup read_write
 * @param[in] assistant Assistant identifier
 * @param[out] data_timestamp Pointer on the internal buffer sample data timestamp, set to NULL if not received
 * @param[out] recv_timestamp Pointer on the internal buffer recv timestamp.
 * The value is set according to ::libed247_set_simulation_time_ns() callback that has to be specified by the user.
 * If no callback is specified, the pointer is set to NULL
 * @param[out] info Stream sample additional information
 * @param[out] empty Returns true if the internal stack is empty
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_NODATA Receive stack is empty
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_assistant_pop_sample(
    ed247_stream_assistant_t assistant,
    const ed247_timestamp_t **data_timestamp,
    const ed247_timestamp_t **recv_timestamp,
    const ed247_sample_info_t **info,
    bool *empty);

/**
 * @brief Number of samples in the stream stack, incremented and decremented according to ::ed247_stream_push() & ::ed247_stream_pop() functions.
 * @ingroup read_write
 * @param[in] stream Stream identifier
 * @param[in] direction Only ::ED247_DIRECTION_IN or ::ED247_DIRECTION_OUT accepted, reference the desired stack to write on
 * @param[out] size Number of samples in the stack
 * @retval ED247_STATUS_SUCCESS
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_samples_number(
    ed247_stream_t stream,
    ed247_direction_t direction,
    size_t *size);

/******************
 * Receive & Send *
 ******************/

/**
 * @brief Stream receive callback function pointer.
 * The argument stream is the stream identifier that received something.
 * @ingroup send_recv
 */
typedef ed247_status_t (*ed247_stream_recv_callback_t)(ed247_context_t context, ed247_stream_t stream);

typedef ed247_status_t (*ed247_com_callback_t)(ed247_context_t context);

/**
 * @brief Register a callback (in a stream) which is called once a frame is received and decoded.
 * @ingroup send_recv
 * @param[in] stream Stream identifier
 * @param[in] callback The callback function
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_register_recv_callback(
    ed247_context_t context,
    ed247_stream_t stream,
    ed247_stream_recv_callback_t callback);

/**
 * @brief Unregister a callback (from a stream) which is called once a frame is received and decoded.
 * @ingroup send_recv
 * @param[in] stream Stream identifier
 * @param[in] callback The callback function
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_stream_unregister_recv_callback(
    ed247_context_t context,
    ed247_stream_t stream,
    ed247_stream_recv_callback_t callback);

/**
 * @brief Register a callback (in several streams as once) which is called once a frame is received and decoded.
 * @ingroup send_recv
 * @param[in] streams Stream identifiers
 * @param[in] callback The callback function
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_streams_register_recv_callback(
    ed247_context_t context,
    ed247_stream_list_t streams,
    ed247_stream_recv_callback_t callback);

/**
 * @brief Unregister a callback (from several streams as once) which is called once a frame is received and decoded.
 * @ingroup send_recv
 * @param[in] streams Stream identifiers
 * @param[in] callback The callback function
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_streams_unregister_recv_callback(
    ed247_context_t context,
    ed247_stream_list_t streams,
    ed247_stream_recv_callback_t callback);

/**
 * @brief Register a callback (in all streams) which is called once a frame is received and decoded.
 * @ingroup send_recv
 * @param[in] streams Stream identifiers
 * @param[in] callback The callback function
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_register_recv_callback(
    ed247_context_t context,
    ed247_stream_recv_callback_t callback);

/**
 * @brief Unegister a callback (in all streams) which is called once a frame is received and decoded.
 * @ingroup send_recv
 * @param[in] streams Stream identifiers
 * @param[in] callback The callback function
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_unregister_recv_callback(
    ed247_context_t context,
    ed247_stream_recv_callback_t callback);

/**
 * @brief Register a callback which is called each time a frame is received.
 * @ingroup send_recv
 * @param[in] streams Stream identifiers
 * @param[in] callback The callback function
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_register_com_recv_callback(
    ed247_context_t context,
    ed247_com_callback_t callback);

/**
 * @brief Unregister a callback which is called each time a frame is received.
 * @ingroup send_recv
 * @param[in] streams Stream identifiers
 * @param[in] callback The callback function
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_unregister_com_recv_callback(
    ed247_context_t context,
    ed247_com_callback_t callback);

/**
 * @brief Register a callback which is called each time a frame is sent.
 * @ingroup send_recv
 * @param[in] streams Stream identifiers
 * @param[in] callback The callback function
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_register_com_send_callback(
    ed247_context_t context,
    ed247_com_callback_t callback);

/**
 * @brief Unregister a callback which is called each time a frame is sent.
 * @ingroup send_recv
 * @param[in] streams Stream identifiers
 * @param[in] callback The callback function
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_unregister_com_send_callback(
    ed247_context_t context,
    ed247_com_callback_t callback);

/**
 * @brief Blocks until the first frame is received and processed, and at least a stream has available data. 
 * If several frames has been received, they are all processed.
 * @ingroup send_recv
 * @param[in] context Context identifier
 * @param[out] streams List of streams that received samples, can be NULL
 * @param[in] timeout_us Timeout value, in microseconds
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 * @retval ED247_STATUS_TIMEOUT
 */
extern LIBED247_EXPORT ed247_status_t ed247_wait_frame(
    ed247_context_t context,
    ed247_stream_list_t *streams,
    int32_t timeout_us);

/**
 * @brief Blocks until duration is elapsed, processing all received data.
 * @ingroup send_recv
 * @param[in] context Context identifier
 * @param[out] streams List of streams that received samples, can be NULL
 * @param[in] duration_us Duration value, in microseconds
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_wait_during(
    ed247_context_t context,
    ed247_stream_list_t *streams,
    int32_t duration_us);

/**
 * @brief Send the samples that were written (::ed247_stream_write() or ::ed247_signal_write()) then pushed (::ed247_stream_push() or ::ed247_signal_push()).
 * <b>This function clear send stacks.</b>
 * @ingroup send_recv
 * @param[in] context Context identifier
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_send_pushed_samples(
    ed247_context_t context);

/**
 * @brief Create ED247 frames of each channel where one of its streams contains data to send.
 * @ingroup advanced
 * @param[in] context Context identifier
 * @param[out] frames List of produced frames
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_frame_encode(
    ed247_context_t context,
    ed247_frame_list_t *frames);

/**
 * @brief Decode and push stream samples from the ED47 frames
 * @ingroup advanced
 * @param[in] channel Channel identifier
 * @param[in] frame ED247 frame
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_frame_decode(
    ed247_channel_t channel,
    const void * data,
    size_t size);

/**********
 * Unload *
 **********/

/**
 * @brief Unload ressources linked to the given context
 * @ingroup load_unload
 * @param[in] context Context identifier
 * @retval ED247_STATUS_SUCCESS
 * @retval ED247_STATUS_FAILURE
 */
extern LIBED247_EXPORT ed247_status_t ed247_unload(
    ed247_context_t ed247_context);

#ifdef __cplusplus
};
#endif

#endif
