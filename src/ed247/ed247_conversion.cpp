/* -*- mode: c++; c-basic-offset: 2 -*-  */
/*
 * Types conversion to/from strings
 * Implementation of ed247.h
 */
#include "ed247.h"
#include <string.h>

namespace ed247 {
  namespace type_name {

    static const char* Unknown{"Unknown"};

    namespace status {
      static const char* SUCCESS{"SUCCESS"};
      static const char* FAILURE{"FAILURE"};
      static const char* TIMEOUT{"TIMEOUT"};
      static const char* NODATA{"NODATA"};
    }

    namespace standard {
      static const char* ED247{"-"};
      static const char* ED247A{"A"};
    }

    namespace direction {
      static const char* INPUT{"In"};
      static const char* OUTPUT{"Out"};
      static const char* INOUT{"InOut"};
    }

    namespace yesno {
      static const char* NO{"No"};
      static const char* YES{"Yes"};
    }

    namespace component_type {
      static const char* VIRTUAL{"Virtual"};
      static const char* BRIDGE{"Bridge"};
    }

    namespace stream_type {
      static const char* A664{"A664"};
      static const char* A429{"A429"};
      static const char* A825{"A825"};
      static const char* M1553{"M1553"};
      static const char* SERIAL{"SERIAL"};
      static const char* AUDIO{"AUDIO"};
      static const char* VIDEO{"VIDEO"};
      static const char* ETHERNET{"ETHERNET"};
      static const char* ANALOG{"ANALOG"};
      static const char* DISCRETE{"DISCRETE"};
      static const char* NAD{"NAD"};
      static const char* VNAD{"VNAD"};
    }

    namespace signal_type {
      static const char* ANALOG{"ANALOG"};
      static const char* DISCRETE{"DISCRETE"};
      static const char* NAD{"NAD"};
      static const char* VNAD{"VNAD"};
    }

    namespace nad_type {
      static const char* INT8{"int8"};
      static const char* INT16{"int16"};
      static const char* INT32{"int32"};
      static const char* INT64{"int64"};
      static const char* UINT8{"uint8"};
      static const char* UINT16{"uint16"};
      static const char* UINT32{"uint32"};
      static const char* UINT64{"uint64"};
      static const char* FLOAT32{"float32"};
      static const char* FLOAT64{"float64"};
    }

  }
}

const char * ed247_status_string(
  ed247_status_t status)
{
  switch(status){
  case ED247_STATUS_SUCCESS: return ed247::type_name::status::SUCCESS;
  case ED247_STATUS_FAILURE: return ed247::type_name::status::FAILURE;
  case ED247_STATUS_TIMEOUT: return ed247::type_name::status::TIMEOUT;
  case ED247_STATUS_NODATA:  return ed247::type_name::status::NODATA;
  default:                   return ed247::type_name::Unknown;
  }
}

const char * ed247_standard_string(
  ed247_standard_t standard)
{
  switch(standard){
  case ED247_STANDARD_ED247:  return ed247::type_name::standard::ED247;
  case ED247_STANDARD_ED247A: return ed247::type_name::standard::ED247A;
  default:                    return ed247::type_name::Unknown;
  }
}

const char * ed247_direction_string(
  ed247_direction_t direction)
{
  switch(direction){
  case ED247_DIRECTION_IN:    return ed247::type_name::direction::INPUT;
  case ED247_DIRECTION_OUT:   return ed247::type_name::direction::OUTPUT;
  case ED247_DIRECTION_INOUT: return ed247::type_name::direction::INOUT;
  default:                    return ed247::type_name::Unknown;
  }
}

ed247_standard_t ed247_standard_from_string(
  const char *standard)
{
  using namespace ed247::type_name::standard;
  if(strcmp(ED247, standard) == 0) {
    return ED247_STANDARD_ED247;
  } else if(strcmp(ED247A, standard) == 0) {
    return ED247_STANDARD_ED247A;
  } else {
    return ED247_STANDARD__INVALID;
  }
}

ed247_direction_t ed247_direction_from_string(
  const char *direction)
{
  using namespace ed247::type_name;
  if(strcmp(direction::INPUT, direction) == 0){
    return ED247_DIRECTION_IN;
  }else if(strcmp(direction::OUTPUT, direction) == 0){
    return ED247_DIRECTION_OUT;
  }else if(strcmp(direction::INOUT, direction) == 0){
    return ED247_DIRECTION_INOUT;
  }else{
    return ED247_DIRECTION__INVALID;
  }
}

const char * ed247_yesno_string(
  ed247_yesno_t yesno)
{
  using namespace ed247::type_name::yesno;
  switch(yesno){
  case ED247_YESNO_NO:    return NO;
  case ED247_YESNO_YES:   return YES;
  default:                return ed247::type_name::Unknown;
  }
}

ed247_yesno_t ed247_yesno_from_string(
  const char *yesno)
{
  using namespace ed247::type_name;
  if (strcmp(yesno::NO, yesno) == 0) {
    return ED247_YESNO_NO;
  }else if(strcmp(yesno::YES, yesno) == 0){
    return ED247_YESNO_YES;
  }else{
    return ED247_YESNO__INVALID;
  }
}

const char * ed247_component_type_string(
  ed247_component_type_t component_type)
{
  switch(component_type){
  case ED247_COMPONENT_TYPE_VIRTUAL:  return ed247::type_name::component_type::VIRTUAL;
  case ED247_COMPONENT_TYPE_BRIDGE:   return ed247::type_name::component_type::BRIDGE;
  default:                            return ed247::type_name::Unknown;
  }
}

ed247_component_type_t ed247_component_type_from_string(
  const char *component_type)
{
  using namespace ed247::type_name::component_type;
  if(strcmp(VIRTUAL, component_type) == 0){
    return ED247_COMPONENT_TYPE_VIRTUAL;
  }else if(strcmp(BRIDGE, component_type) == 0){
    return ED247_COMPONENT_TYPE_BRIDGE;
  }else{
    return ED247_COMPONENT_TYPE__INVALID;
  }
}

const char * ed247_stream_type_string(
  ed247_stream_type_t stream_type)
{
  switch(stream_type){
  case ED247_STREAM_TYPE_A664:       return ed247::type_name::stream_type::A664;
  case ED247_STREAM_TYPE_A429:       return ed247::type_name::stream_type::A429;
  case ED247_STREAM_TYPE_A825:       return ed247::type_name::stream_type::A825;
  case ED247_STREAM_TYPE_M1553:      return ed247::type_name::stream_type::M1553;
  case ED247_STREAM_TYPE_SERIAL:     return ed247::type_name::stream_type::SERIAL;
  case ED247_STREAM_TYPE_AUDIO:      return ed247::type_name::stream_type::AUDIO;
  case ED247_STREAM_TYPE_VIDEO:      return ed247::type_name::stream_type::VIDEO;
  case ED247_STREAM_TYPE_ETHERNET:   return ed247::type_name::stream_type::ETHERNET;
  case ED247_STREAM_TYPE_ANALOG:     return ed247::type_name::stream_type::ANALOG;
  case ED247_STREAM_TYPE_DISCRETE:   return ed247::type_name::stream_type::DISCRETE;
  case ED247_STREAM_TYPE_NAD:        return ed247::type_name::stream_type::NAD;
  case ED247_STREAM_TYPE_VNAD:       return ed247::type_name::stream_type::VNAD;
  default:                           return ed247::type_name::Unknown;
  }
}

ed247_stream_type_t ed247_stream_type_from_string(
  const char *stream_type)
{
  using namespace ed247::type_name::stream_type;
  if(strcmp(A664, stream_type) == 0){
    return ED247_STREAM_TYPE_A664;
  }else if(strcmp(A429, stream_type) == 0){
    return ED247_STREAM_TYPE_A429;
  }else if(strcmp(A825, stream_type) == 0){
    return ED247_STREAM_TYPE_A825;
  }else if(strcmp(M1553, stream_type) == 0){
    return ED247_STREAM_TYPE_M1553;
  }else if(strcmp(SERIAL, stream_type) == 0){
    return ED247_STREAM_TYPE_SERIAL;
  }else if(strcmp(AUDIO, stream_type) == 0){
    return ED247_STREAM_TYPE_AUDIO;
  }else if(strcmp(VIDEO, stream_type) == 0){
    return ED247_STREAM_TYPE_VIDEO;
  }else if(strcmp(ETHERNET, stream_type) == 0){
    return ED247_STREAM_TYPE_ETHERNET;
  }else if(strcmp(ANALOG, stream_type) == 0){
    return ED247_STREAM_TYPE_ANALOG;
  }else if(strcmp(DISCRETE, stream_type) == 0){
    return ED247_STREAM_TYPE_DISCRETE;
  }else if(strcmp(NAD, stream_type) == 0){
    return ED247_STREAM_TYPE_NAD;
  }else if(strcmp(VNAD, stream_type) == 0){
    return ED247_STREAM_TYPE_VNAD;
  }else{
    return ED247_STREAM_TYPE__INVALID;
  }
}

const char * ed247_signal_type_string(
  ed247_signal_type_t signal_type)
{
  switch(signal_type){
  case ED247_SIGNAL_TYPE_ANALOG:   return ed247::type_name::signal_type::ANALOG;
  case ED247_SIGNAL_TYPE_DISCRETE: return ed247::type_name::signal_type::DISCRETE;
  case ED247_SIGNAL_TYPE_NAD:      return ed247::type_name::signal_type::NAD;
  case ED247_SIGNAL_TYPE_VNAD:     return ed247::type_name::signal_type::VNAD;
  default:                         return ed247::type_name::Unknown;
  }
}

ed247_signal_type_t ed247_signal_type_from_string(
  const char *signal_type)
{
  using namespace ed247::type_name::signal_type;
  if(strcmp(ANALOG, signal_type) == 0){
    return ED247_SIGNAL_TYPE_ANALOG;
  }else if(strcmp(DISCRETE, signal_type) == 0){
    return ED247_SIGNAL_TYPE_DISCRETE;
  }else if(strcmp(NAD, signal_type) == 0){
    return ED247_SIGNAL_TYPE_NAD;
  }else if(strcmp(VNAD, signal_type) == 0){
    return ED247_SIGNAL_TYPE_VNAD;
  }else{
    return ED247_SIGNAL_TYPE__INVALID;
  }
}

const char * ed247_nad_type_string(
  ed247_nad_type_t nad_type)
{
  switch(nad_type){
  case ED247_NAD_TYPE_INT8:     return ed247::type_name::nad_type::INT8;
  case ED247_NAD_TYPE_INT16:    return ed247::type_name::nad_type::INT16;
  case ED247_NAD_TYPE_INT32:    return ed247::type_name::nad_type::INT32;
  case ED247_NAD_TYPE_INT64:    return ed247::type_name::nad_type::INT64;
  case ED247_NAD_TYPE_UINT8:    return ed247::type_name::nad_type::UINT8;
  case ED247_NAD_TYPE_UINT16:   return ed247::type_name::nad_type::UINT16;
  case ED247_NAD_TYPE_UINT32:   return ed247::type_name::nad_type::UINT32;
  case ED247_NAD_TYPE_UINT64:   return ed247::type_name::nad_type::UINT64;
  case ED247_NAD_TYPE_FLOAT32:  return ed247::type_name::nad_type::FLOAT32;
  case ED247_NAD_TYPE_FLOAT64:  return ed247::type_name::nad_type::FLOAT64;
  default:                      return ed247::type_name::Unknown;
  }
}

ed247_nad_type_t ed247_nad_type_from_string(
  const char *nad_type)
{
  using namespace ed247::type_name;
  if(strcmp(nad_type::INT8, nad_type) == 0){
    return ED247_NAD_TYPE_INT8;
  }else if(strcmp(nad_type::INT16, nad_type) == 0){
    return ED247_NAD_TYPE_INT16;
  }else if(strcmp(nad_type::INT32, nad_type) == 0){
    return ED247_NAD_TYPE_INT32;
  }else if(strcmp(nad_type::INT64, nad_type) == 0){
    return ED247_NAD_TYPE_INT64;
  }else if(strcmp(nad_type::UINT8, nad_type) == 0){
    return ED247_NAD_TYPE_UINT8;
  }else if(strcmp(nad_type::UINT16, nad_type) == 0){
    return ED247_NAD_TYPE_UINT16;
  }else if(strcmp(nad_type::UINT32, nad_type) == 0){
    return ED247_NAD_TYPE_UINT32;
  }else if(strcmp(nad_type::UINT64, nad_type) == 0){
    return ED247_NAD_TYPE_UINT64;
  }else if(strcmp(nad_type::FLOAT32, nad_type) == 0){
    return ED247_NAD_TYPE_FLOAT32;
  }else if(strcmp(nad_type::FLOAT64, nad_type) == 0){
    return ED247_NAD_TYPE_FLOAT64;
  }else{
    return ED247_NAD_TYPE__INVALID;
  }
}
