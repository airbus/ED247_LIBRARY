/*
 * Types conversion to/from strings
 */
#include "ed247.h"
#include "ed247_types.h"


const char * ed247_status_string(
  ed247_status_t status)
{
  switch(status){
  case ED247_STATUS_SUCCESS: return ed247::defines::status::SUCCESS.c_str();
  case ED247_STATUS_FAILURE: return ed247::defines::status::FAILURE.c_str();
  case ED247_STATUS_TIMEOUT: return ed247::defines::status::TIMEOUT.c_str();
  case ED247_STATUS_NODATA:  return ed247::defines::status::NODATA.c_str();
  default:                   return ed247::defines::Unknown.c_str();
  }
}

const char * ed247_standard_string(
  ed247_standard_t standard)
{
  switch(standard){
  case ED247_STANDARD_ED247:  return ed247::defines::standard::ED247.c_str();
  case ED247_STANDARD_ED247A: return ed247::defines::standard::ED247A.c_str();
  default:                    return ed247::defines::Unknown.c_str();
  }
}

const char * ed247_direction_string(
  ed247_direction_t direction)
{
  switch(direction){
  case ED247_DIRECTION_IN:    return ed247::defines::direction::INPUT.c_str();
  case ED247_DIRECTION_OUT:   return ed247::defines::direction::OUTPUT.c_str();
  case ED247_DIRECTION_INOUT: return ed247::defines::direction::INOUT.c_str();
  default:                    return ed247::defines::Unknown.c_str();
  }
}

ed247_standard_t ed247_standard_from_string(
  const char *standard)
{
  using namespace ed247::defines::standard;
  if(ED247.compare(standard) == 0) {
    return ED247_STANDARD_ED247;
  } else if(ED247A.compare(standard) == 0) {
    return ED247_STANDARD_ED247A;
  } else {
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
  default:                return ed247::defines::Unknown.c_str();
  }
}

ed247_yesno_t ed247_yesno_from_string(
  const char *yesno)
{
  using namespace ed247::defines;
  if (yesno::NO.compare(yesno) == 0) {
    return ED247_YESNO_NO;
  }else if(yesno::YES.compare(yesno) == 0){
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
  default:                           return ed247::defines::Unknown.c_str();
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
  case ED247_SIGNAL_TYPE_ANALOG:   return ed247::defines::signal_type::ANALOG.c_str();
  case ED247_SIGNAL_TYPE_DISCRETE: return ed247::defines::signal_type::DISCRETE.c_str();
  case ED247_SIGNAL_TYPE_NAD:      return ed247::defines::signal_type::NAD.c_str();
  case ED247_SIGNAL_TYPE_VNAD:     return ed247::defines::signal_type::VNAD.c_str();
  default:                         return ed247::defines::Unknown.c_str();
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
  case ED247_NAD_TYPE_INT8:     return ed247::defines::nad_type::INT8.c_str();
  case ED247_NAD_TYPE_INT16:    return ed247::defines::nad_type::INT16.c_str();
  case ED247_NAD_TYPE_INT32:    return ed247::defines::nad_type::INT32.c_str();
  case ED247_NAD_TYPE_INT64:    return ed247::defines::nad_type::INT64.c_str();
  case ED247_NAD_TYPE_UINT8:    return ed247::defines::nad_type::UINT8.c_str();
  case ED247_NAD_TYPE_UINT16:   return ed247::defines::nad_type::UINT16.c_str();
  case ED247_NAD_TYPE_UINT32:   return ed247::defines::nad_type::UINT32.c_str();
  case ED247_NAD_TYPE_UINT64:   return ed247::defines::nad_type::UINT64.c_str();
  case ED247_NAD_TYPE_FLOAT32:  return ed247::defines::nad_type::FLOAT32.c_str();
  case ED247_NAD_TYPE_FLOAT64:  return ed247::defines::nad_type::FLOAT64.c_str();
  default:                      return ed247::defines::Unknown.c_str();
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

