/* -*- mode: c++; c-basic-offset: 2 -*-  */
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
#include "ed247_xml.h"
#include "ed247_logs.h"
#include "ed247_xsd.h"
#include <libxml/xmlschemas.h>
#include <algorithm>

/*
 * ECIC Nodes and attributes
 */
namespace ed247 {
  namespace xml {
    namespace node {
      static const std::string DataTimestamp { "DataTimestamp" };
      static const std::string FileProducer { "FileProducer" };
      static const std::string UdpSockets { "UDP_Sockets" };
      static const std::string UdpSocket { "UDP_Socket" };
      static const std::string FrameFormat { "FrameFormat" };
      static const std::string ComInterface { "ComInterface" };
      static const std::string Header { "Header" };
      static const std::string SampleTimestampOffset { "SampleTimestampOffset" };
      static const std::string Errors { "Errors" };
      static const std::string Signal { "Signal" };
      static const std::string Signals { "Signals" };
      static const std::string A429_Stream { "A429_Stream" };
      static const std::string A664_Stream { "A664_Stream" };
      static const std::string MessageSize { "MessageSize" };
      static const std::string A825_Stream { "A825_Stream" };
      static const std::string SERIAL_Stream { "SERIAL_Stream" };
      static const std::string DIS_Stream { "DIS_Stream" };
      static const std::string ANA_Stream { "ANA_Stream" };
      static const std::string NAD_Stream { "NAD_Stream" };
      static const std::string VNAD_Stream { "VNAD_Stream" };
      static const std::string Streams { "Streams" };
      static const std::string Stream { "Stream" };
      static const std::string MultiChannel { "MultiChannel" };
      static const std::string Channel { "Channel" };
      static const std::string Channels { "Channels" };
      static const std::string ED247ComponentInstanceConfiguration { "ED247ComponentInstanceConfiguration" };
    }

    namespace attr {
      static const std::string Name { "Name" };
      static const std::string Direction { "Direction" };
      static const std::string ComponentVersion { "ComponentVersion" };
      static const std::string Comment { "Comment" };
      static const std::string Identifier { "Identifier" };
      static const std::string DstIP { "DstIP" };
      static const std::string DstPort { "DstPort" };
      static const std::string SrcIP { "SrcIP" };
      static const std::string SrcPort { "SrcPort" };
      static const std::string MulticastInterfaceIP { "MulticastInterfaceIP" };
      static const std::string MulticastTTL { "MulticastTTL" };
      static const std::string TransportTimestamp { "TransportTimestamp" };
      static const std::string SampleMaxNumber { "SampleMaxNumber" };
      static const std::string SampleMaxSizeBytes { "SampleMaxSizeBytes" };
      static const std::string Position { "Position" };
      static const std::string Enable { "Enable" };
      static const std::string UID { "UID" };
      static const std::string ICD { "ICD" };
      static const std::string FifoSize { "FifoSize" };
      static const std::string ByteOffset { "ByteOffset" };
      static const std::string ComponentType { "ComponentType" };
      static const std::string StandardRevision { "StandardRevision" };
      static const std::string SamplingPeriodUs { "SamplingPeriodUs" };
      static const std::string Type { "Type" };
      static const std::string MaxNumber { "MaxNumber" };
      static const std::string MaxLength { "MaxLength" };  // Backward compatibility
      static const std::string Unit { "Unit" };
      static const std::string SampleDataTimestampOffset { "SampleDataTimestampOffset" };
      static const std::string ElectricalUnit { "ElectricalUnit" };
      static const std::string Dimensions { "Dimensions" };
    }
  }
}

/*
 * libXML2 C++ wrapping tools
 */
namespace xml {
  std::string xmlChar_as_string(const xmlChar *str)
  {
    if (!str) return std::string();
    return std::string((const char *)str);
  }

  std::string xmlNode_get_fileline(xmlNode* node) {
    std::string file_line;
    if (node != nullptr) {
      if (node->doc && node->doc->name && *node->doc->name) {
        file_line = std::string(node->doc->name) + ":";
      } else {
        file_line = "line ";
      }
      file_line += strize() << node->line;
    }
    return file_line;
  }

  template<typename T>
  void xmlAttr_get_value(const xmlAttrPtr attribute, T & variable)
  {
    std::string value = ::xml::xmlChar_as_string(xmlGetProp(attribute->parent, attribute->name));
    std::stringstream(value) >> variable;
  }
  template<>
  void xmlAttr_get_value(const xmlAttrPtr attribute, std::string& variable)
  {
    variable = ::xml::xmlChar_as_string(xmlGetProp(attribute->parent, attribute->name));
  }
  template<>
  void xmlAttr_get_value(const xmlAttrPtr attribute, ed247_component_type_t& variable)
  {
    variable = ed247_component_type_from_string((const char*)xmlGetProp(attribute->parent, attribute->name));
  }
  template<>
  void xmlAttr_get_value(const xmlAttrPtr attribute, ed247_standard_t& variable)
  {
    variable = ed247_standard_from_string((const char*)xmlGetProp(attribute->parent, attribute->name));
  }
  template<>
  void xmlAttr_get_value(const xmlAttrPtr attribute, ed247_yesno_t& variable)
  {
    variable = ed247_yesno_from_string((const char*)xmlGetProp(attribute->parent, attribute->name));
  }
  template<>
  void xmlAttr_get_value(const xmlAttrPtr attribute, ed247_direction_t& variable)
  {
    variable = ed247_direction_from_string((const char*)xmlGetProp(attribute->parent, attribute->name));
  }
  template<>
  void xmlAttr_get_value(const xmlAttrPtr attribute, ed247_nad_type_t& variable)
  {
    variable = ed247_nad_type_from_string((const char*)xmlGetProp(attribute->parent, attribute->name));
  }

  template<> void xmlAttr_get_value(const xmlAttrPtr attribute, const char * & variable) = delete;
  template<> void xmlAttr_get_value(const xmlAttrPtr attribute, char * & variable) = delete;
}

/*
 * Error handling
 */
namespace ed247 {
  namespace xml {

    static xmlError libxml_error;

    class exception : public ed247::exception {
    public:
      exception(std::string message) : ed247::exception(std::string("xml parse exception: ") + message) {};
      virtual ~exception() throw () override {}
    };

    static void libxml_structured_error(void * user_data, xmlErrorPtr error)
    {
      xmlResetError(&libxml_error);
      xmlCopyError(error, &libxml_error);
    }
  }
}

#define PARSER_WARNING(closest_node, msg)                                                                        \
  do {                                                                                                           \
  std::string message = strize() << msg;                                                                         \
  if (closest_node != nullptr) {                                                                                 \
    message += std::string(". Near ") + ::xml::xmlNode_get_fileline(closest_node);                               \
  }                                                                                                              \
  PRINT_WARNING(message);                                                                                        \
} while (0)

#define THROW_PARSER_ERROR(closest_node, msg)                                                                    \
  do {                                                                                                           \
    std::string message = strize() << msg;                                                                       \
    if (ed247::xml::libxml_error.file != nullptr) {                                                              \
      message += strize() << ". From " << ed247::xml::libxml_error.file << ':' << ed247::xml::libxml_error.line; \
    } else if (closest_node != nullptr) {                                                                        \
      message += std::string(". Near ") + ::xml::xmlNode_get_fileline(closest_node);                             \
    }                                                                                                            \
    if (ed247::xml::libxml_error.message != nullptr) {                                                           \
      message += strize() << " " << ed247::xml::libxml_error.message;                                            \
    }                                                                                                            \
    xmlResetError(&ed247::xml::libxml_error);                                                                    \
    PRINT_ERROR(message);                                                                                        \
    throw ed247::xml::exception(message);                                                                        \
  } while (0)

//
// DataTimestamp
//
ed247::xml::DataTimestamp::DataTimestamp() :
  _enable(ED247_YESNO_NO),
  _enable_sample_offset(ED247_YESNO_NO)
{
}

void ed247::xml::DataTimestamp::load(const xmlNodePtr xml_node)
{
  for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
    auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
    if(attr_name.compare(attr::Enable) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _enable);
    }else if(attr_name.compare(attr::SampleDataTimestampOffset) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _enable_sample_offset);
    }else{
      THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::DataTimestamp << "]");
    }
  }
}

//
// Errors
//
ed247::xml::Errors::Errors() :
  _enable(ED247_YESNO_NO)
{
}

void ed247::xml::Errors::load(const xmlNodePtr xml_node)
{
  for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
    auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
    if(attr_name.compare(attr::Enable) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _enable);
    }else{
      THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::Errors << "]");
    }
  }
}

//
// UdpSocket
//
ed247::xml::UdpSocket::UdpSocket() :
  _dst_ip_port(0),
  _src_ip_port(0),
  _mc_ttl(1),
  _direction(ED247_DIRECTION__INVALID)
{
}

void ed247::xml::UdpSocket::load(const xmlNodePtr xml_node)
{
  for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
    auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
    if(attr_name.compare(attr::DstIP) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _dst_ip_address);
    }else if(attr_name.compare(attr::DstPort) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _dst_ip_port);
    }else if(attr_name.compare(attr::SrcIP) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _src_ip_address);
    }else if(attr_name.compare(attr::SrcPort) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _src_ip_port);
    }else if(attr_name.compare(attr::MulticastInterfaceIP) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _mc_ip_address);
    }else if(attr_name.compare(attr::MulticastTTL) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _mc_ttl);
    }else if(attr_name.compare(attr::Direction) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _direction);
    }else{
      THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::UdpSocket << "]");
    }
  }
  if (_direction == ED247_DIRECTION_INOUT) {
    THROW_PARSER_ERROR(xml_node, "Bidirectional UDP_Socket is not supported.");
  }
}

std::ostream& operator<<(std::ostream& stream, const ed247::xml::UdpSocket& socket)
{
  return stream << "UdpSocket - "
    "DstIP[" << socket._dst_ip_address << "] DstPort[" << socket._dst_ip_port<< "] "
    "SrcIP[" << socket._src_ip_address << "] SrcPort[" << socket._src_ip_port << "] "
    "MulticastInterfaceIP[" << socket._mc_ip_address << "] MulticastTTL[" << socket._mc_ttl << "]";
}

//
// ComInterface
//
void ed247::xml::ComInterface::load(const xmlNodePtr xml_node)
{
  for(auto xml_node_iter = xml_node->children ; xml_node_iter != nullptr ; xml_node_iter = xml_node_iter->next){
    if(xml_node_iter->type != XML_ELEMENT_NODE)
      continue;
    auto node_name = ::xml::xmlChar_as_string(xml_node_iter->name);
    if(node_name.compare(node::UdpSockets) == 0){
      for(auto xml_node_child_iter = xml_node_iter->children ; xml_node_child_iter != nullptr ; xml_node_child_iter = xml_node_child_iter->next){
        if(xml_node_child_iter->type != XML_ELEMENT_NODE)
          continue;
        node_name = ::xml::xmlChar_as_string(xml_node_child_iter->name);
        if(node_name.compare(node::UdpSocket) == 0){
          _udp_sockets.emplace_back();
          _udp_sockets.back().load(xml_node_child_iter);
        }else{
          THROW_PARSER_ERROR(xml_node_child_iter, "Unknown node [" << node_name << "] in tag [" << node::UdpSockets << "]");
        }
      }
    }else{
      THROW_PARSER_ERROR(xml_node_iter, "Unknown node [" << node_name << "] in tag [" << node::ComInterface << "]");
    }
  }
}

//
// Stream base
//
ed247::xml::Stream::Stream(ed247_stream_type_t type, uint32_t sample_max_size_bytes) :
  _direction(ED247_DIRECTION__INVALID),
  _type(type),
  _uid(0),
  _sample_max_number(1),
  _sample_max_size_bytes(sample_max_size_bytes)
{
}

ed247::xml::StreamProtocoled::StreamProtocoled(ed247_stream_type_t type, uint32_t sample_max_size_bytes) :
  Stream(type, sample_max_size_bytes)
{}

ed247::xml::StreamSignals::StreamSignals(ed247_stream_type_t type, uint32_t sample_max_size_bytes) :
  Stream(type, sample_max_size_bytes),
  _sampling_period_us(0)
{}

//
// A429Stream
//
ed247::xml::A429Stream::A429Stream() :
  StreamProtocoled(ED247_STREAM_TYPE_A429, 4)
{
}

void ed247::xml::A429Stream::load(const xmlNodePtr xml_node)
{
  for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
    auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
    if(attr_name.compare(attr::Name) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _name);
    }else if(attr_name.compare(attr::Direction) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _direction);
    }else if(attr_name.compare(attr::Comment) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _comment);
    }else if(attr_name.compare(attr::ICD) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _icd);
    }else if(attr_name.compare(attr::SampleMaxNumber) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _sample_max_number);
    }else if(attr_name.compare(attr::UID) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _uid);
    }else{
      THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::A429_Stream << "]");
    }
  }

  for(auto xml_node_iter = xml_node->children ; xml_node_iter != nullptr ; xml_node_iter = xml_node_iter->next){
    if(xml_node_iter->type != XML_ELEMENT_NODE)
      continue;
    auto node_name = ::xml::xmlChar_as_string(xml_node_iter->name);
    if(node_name.compare(node::DataTimestamp) == 0){
      _data_timestamp.load(xml_node_iter);
    }else if(node_name.compare(node::Errors) == 0){
      _errors.load(xml_node_iter);
    }else{
      THROW_PARSER_ERROR(xml_node_iter, "Unknown node [" << node_name << "] in tag [" << node::A429_Stream << "]");
    }
  }
}

//
// A664Stream
//
ed247::xml::A664Stream::A664Stream() :
  StreamProtocoled(ED247_STREAM_TYPE_A664, 1471),
  _enable_message_size(ED247_YESNO_YES)
{
}

void ed247::xml::A664Stream::load(const xmlNodePtr xml_node)
{
  for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
    auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
    if(attr_name.compare(attr::Name) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _name);
    }else if(attr_name.compare(attr::Direction) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _direction);
    }else if(attr_name.compare(attr::Comment) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _comment);
    }else if(attr_name.compare(attr::ICD) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _icd);
    }else if(attr_name.compare(attr::SampleMaxNumber) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _sample_max_number);
    }else if(attr_name.compare(attr::SampleMaxSizeBytes) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _sample_max_size_bytes);
    }else if(attr_name.compare(attr::UID) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _uid);
    }else{
      THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::A664_Stream << "]");
    }
  }

  for(auto xml_node_iter = xml_node->children ; xml_node_iter != nullptr ; xml_node_iter = xml_node_iter->next){
    if(xml_node_iter->type != XML_ELEMENT_NODE)
      continue;
    auto node_name = ::xml::xmlChar_as_string(xml_node_iter->name);
    if(node_name.compare(node::DataTimestamp) == 0){
      _data_timestamp.load(xml_node_iter);
    }else if(node_name.compare(node::Errors) == 0){
      _errors.load(xml_node_iter);
    }else if(node_name.compare(node::MessageSize) == 0){
      for(auto xml_attr = xml_node_iter->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
        auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
        if(attr_name.compare(attr::Enable) == 0){
          ::xml::xmlAttr_get_value(xml_attr, _enable_message_size);
        }else{
          THROW_PARSER_ERROR(xml_node_iter, "Unknown attribute [" << attr_name << "] in tag [" << node::MessageSize << "]");
        }
      }
    }else{
      THROW_PARSER_ERROR(xml_node_iter, "Unknown node [" << node_name << "] in tag [" << node::A664_Stream << "]");
    }
    // Check message enable size & sample_max_number
    if(_enable_message_size == ED247_YESNO_NO && _sample_max_number > 1) {
      THROW_PARSER_ERROR(xml_node, "A664 Stream cannot have a SampleMaxNumber of [" << _sample_max_number << "] and not enabling Message Size encoding in frame");
    }
  }
}

//
// A825Stream
//
ed247::xml::A825Stream::A825Stream() :
  StreamProtocoled(ED247_STREAM_TYPE_A825, 69)
{
}

void ed247::xml::A825Stream::load(const xmlNodePtr xml_node)
{
  for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
    auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
    if(attr_name.compare(attr::Name) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _name);
    }else if(attr_name.compare(attr::Direction) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _direction);
    }else if(attr_name.compare(attr::Comment) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _comment);
    }else if(attr_name.compare(attr::ICD) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _icd);
    }else if(attr_name.compare(attr::SampleMaxNumber) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _sample_max_number);
    }else if(attr_name.compare(attr::UID) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _uid);
    }else{
      THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::A825_Stream << "]");
    }
  }

  for(auto xml_node_iter = xml_node->children ; xml_node_iter != nullptr ; xml_node_iter = xml_node_iter->next){
    if(xml_node_iter->type != XML_ELEMENT_NODE)
      continue;
    auto node_name = ::xml::xmlChar_as_string(xml_node_iter->name);
    if(node_name.compare(node::DataTimestamp) == 0){
      _data_timestamp.load(xml_node_iter);
    }else if(node_name.compare(node::Errors) == 0){
      _errors.load(xml_node_iter);
    }else{
      THROW_PARSER_ERROR(xml_node_iter, "Unknown node [" << node_name << "] in tag [" << node::A825_Stream << "]");
    }
  }
}

//
// SERIALStream
//
ed247::xml::SERIALStream::SERIALStream() :
  StreamProtocoled(ED247_STREAM_TYPE_SERIAL, 1)
{
}

void ed247::xml::SERIALStream::load(const xmlNodePtr xml_node)
{
  for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
    auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
    if(attr_name.compare(attr::Name) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _name);
    }else if(attr_name.compare(attr::Direction) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _direction);
    }else if(attr_name.compare(attr::Comment) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _comment);
    }else if(attr_name.compare(attr::ICD) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _icd);
    }else if(attr_name.compare(attr::SampleMaxNumber) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _sample_max_number);
    }else if(attr_name.compare(attr::SampleMaxSizeBytes) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _sample_max_size_bytes);
    }else if(attr_name.compare(attr::UID) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _uid);
    }else{
      THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::SERIAL_Stream << "]");
    }
  }

  for(auto xml_node_iter = xml_node->children ; xml_node_iter != nullptr ; xml_node_iter = xml_node_iter->next){
    if(xml_node_iter->type != XML_ELEMENT_NODE)
      continue;
    auto node_name = ::xml::xmlChar_as_string(xml_node_iter->name);
    if(node_name.compare(node::DataTimestamp) == 0){
      _data_timestamp.load(xml_node_iter);
    }else if(node_name.compare(node::Errors) == 0){
      _errors.load(xml_node_iter);
    }else{
      THROW_PARSER_ERROR(xml_node_iter, "Unknown node [" << node_name << "] in tag [" << node::SERIAL_Stream << "]");
    }
  }
}

ed247::xml::Signal::Signal(ed247_signal_type_t type) :
  _type(type),
  _byte_offset(0),
  _nad_type(ED247_NAD_TYPE__INVALID),
  _vnad_position(0),
  _vnad_max_number(0)
{
  _nad_dimensions.push_back(1);
}

uint32_t ed247::xml::Signal::get_sample_max_size_bytes() const
{
  switch(_type) {
  case ED247_SIGNAL_TYPE_DISCRETE:
    return 1;
  case ED247_SIGNAL_TYPE_ANALOG:
    return 4;
  case ED247_SIGNAL_TYPE_NAD:
  {
    uint32_t size = 1;
    for (auto dim : _nad_dimensions) {
      size *= dim;
    }
    size = size * get_nad_type_size();
    return size;
  }
  case ED247_SIGNAL_TYPE_VNAD:
    return get_nad_type_size() * _vnad_max_number;
  default:
    return 0;
  }
}

uint32_t ed247::xml::Signal::get_nad_type_size(ed247_nad_type_t nad_type)
{
  switch(nad_type) {
  case ED247_NAD_TYPE_INT8    : return sizeof(int8_t);
  case ED247_NAD_TYPE_INT16   : return sizeof(int16_t);
  case ED247_NAD_TYPE_INT32   : return sizeof(int32_t);
  case ED247_NAD_TYPE_INT64   : return sizeof(int64_t);
  case ED247_NAD_TYPE_UINT8   : return sizeof(uint8_t);
  case ED247_NAD_TYPE_UINT16  : return sizeof(uint16_t);
  case ED247_NAD_TYPE_UINT32  : return sizeof(uint32_t);
  case ED247_NAD_TYPE_UINT64  : return sizeof(uint64_t);
  case ED247_NAD_TYPE_FLOAT32 : return sizeof(float);
  case ED247_NAD_TYPE_FLOAT64 : return sizeof(double);
  default: return 0;
  }
}

//
// DISSignal
//
ed247::xml::DISSignal::DISSignal() :
  Signal(ED247_SIGNAL_TYPE_DISCRETE)
{
}

void ed247::xml::DISSignal::load(const xmlNodePtr xml_node)
{
  for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
    auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
    if(attr_name.compare(attr::Name) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _name);
    }else if(attr_name.compare(attr::Comment) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _comment);
    }else if(attr_name.compare(attr::ICD) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _icd);
    }else if(attr_name.compare(attr::ByteOffset) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _byte_offset);
    }else{
      THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::Signal << "]");
    }
  }
}

//
// ANASignal
//
ed247::xml::ANASignal::ANASignal() :
  Signal(ED247_SIGNAL_TYPE_ANALOG)
{
}

void ed247::xml::ANASignal::load(const xmlNodePtr xml_node)
{
  for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
    auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
    if(attr_name.compare(attr::Name) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _name);
    }else if(attr_name.compare(attr::Comment) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _comment);
    }else if(attr_name.compare(attr::ICD) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _icd);
    }else if(attr_name.compare(attr::ElectricalUnit) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _analogue_electrical_unit);
    }else if(attr_name.compare(attr::ByteOffset) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _byte_offset);
    }else{
      THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::Signal << "]");
    }
  }
}

//
// NADSignal
//
ed247::xml::NADSignal::NADSignal() :
  Signal(ED247_SIGNAL_TYPE_NAD)
{
}

void ed247::xml::NADSignal::load(const xmlNodePtr xml_node)
{
  for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
    auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
    if(attr_name.compare(attr::Name) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _name);
    }else if(attr_name.compare(attr::Comment) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _comment);
    }else if(attr_name.compare(attr::ICD) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _icd);
    }else if(attr_name.compare(attr::Dimensions) == 0){
      std::string testee(::xml::xmlChar_as_string(xmlGetProp(xml_node,xml_attr->name)));
      size_t pos = 0;
      uint32_t dimension;
      std::istringstream iss;
      _nad_dimensions.clear();
      // This bloc capture each dimension followed by a separator
      while((pos = testee.find_first_of("xX")) != std::string::npos){
        iss.str(testee.substr(0, pos));
        iss >> dimension;
        iss.clear(); // Reset the flags or iss will not be usable anymore
        _nad_dimensions.push_back(dimension);
        testee.erase(0,pos+1);
      }
      // Capture the last dimension separatedly because it is not followed by a separator
      iss.str(testee.substr(0, pos));
      iss >> dimension;
      _nad_dimensions.push_back(dimension);

    }else if(attr_name.compare(attr::Type) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _nad_type);
    }else if(attr_name.compare(attr::Unit) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _nad_unit);
    }else if(attr_name.compare(attr::ByteOffset) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _byte_offset);
    }else{
      THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::Signal << "]");
    }
  }
}

//
// VNADSignal
//
ed247::xml::VNADSignal::VNADSignal() :
  Signal(ED247_SIGNAL_TYPE_VNAD)
{
}

void ed247::xml::VNADSignal::load(const xmlNodePtr xml_node)
{
  for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
    auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
    if(attr_name.compare(attr::Name) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _name);
    }else if(attr_name.compare(attr::Comment) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _comment);
    }else if(attr_name.compare(attr::ICD) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _icd);
    }else if(attr_name.compare(attr::Type) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _nad_type);
    }else if(attr_name.compare(attr::Unit) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _nad_unit);
    }else if(attr_name.compare(attr::MaxNumber) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _vnad_max_number);
    }else if(attr_name.compare(attr::MaxLength) == 0){            // Backward compatibility
      ::xml::xmlAttr_get_value(xml_attr, _vnad_max_number);
    }else if(attr_name.compare(attr::Position) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _vnad_position);
    }else{
      THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::Signal << "]");
    }
  }
}


//
// DISStream
//
ed247::xml::DISStream::DISStream() :
  StreamSignals(ED247_STREAM_TYPE_DISCRETE, 0)
{
}

void ed247::xml::DISStream::load(const xmlNodePtr xml_node)
{
  for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
    auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
    if(attr_name.compare(attr::Name) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _name);
    }else if(attr_name.compare(attr::Direction) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _direction);
    }else if(attr_name.compare(attr::Comment) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _comment);
    }else if(attr_name.compare(attr::ICD) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _icd);
    }else if(attr_name.compare(attr::SampleMaxNumber) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _sample_max_number);
    }else if(attr_name.compare(attr::SampleMaxSizeBytes) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _sample_max_size_bytes);
    }else if(attr_name.compare(attr::UID) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _uid);
    }else{
      THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::DIS_Stream << "]");
    }
  }

  for(auto xml_node_iter = xml_node->children ; xml_node_iter != nullptr ; xml_node_iter = xml_node_iter->next){
    if(xml_node_iter->type != XML_ELEMENT_NODE)
      continue;
    auto node_name = ::xml::xmlChar_as_string(xml_node_iter->name);
    if(node_name.compare(node::Signals) == 0){
      for(auto xml_node_child_iter = xml_node_iter->children ; xml_node_child_iter != nullptr ; xml_node_child_iter = xml_node_child_iter->next){
        if(xml_node_child_iter->type != XML_ELEMENT_NODE)
          continue;
        node_name = ::xml::xmlChar_as_string(xml_node_child_iter->name);
        for(auto xml_attr = xml_node_iter->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
          auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
          if(attr_name.compare(attr::SamplingPeriodUs) == 0){
            ::xml::xmlAttr_get_value( xml_attr, _sampling_period_us);
          }else{
            THROW_PARSER_ERROR(xml_node_child_iter, "Unknown attribute [" << attr_name << "] in tag [" << node::DIS_Stream << "]");
          }
        }
        if(node_name.compare(node::Signal) == 0) {
          DISSignal* signal = new DISSignal();
          signal->load(xml_node_child_iter);
          if(signal->_byte_offset + signal->get_sample_max_size_bytes() > _sample_max_size_bytes)
            THROW_PARSER_ERROR(xml_node_child_iter, "Stream [" << _name << "] Signal [" << signal->_name << "]: "
                               "ByteOffset [" << signal->_byte_offset << "] + [" << signal->get_sample_max_size_bytes() << "] > "
                               "Stream SampleMaxSizeBytes [" << _sample_max_size_bytes << "]");
          _signal_list.emplace_back(signal);
        }else{
          THROW_PARSER_ERROR(xml_node_child_iter, "Unknown node [" << node_name << "] in tag [" << node::Signals << "]");
        }
      }
    }else if(node_name.compare(node::DataTimestamp) == 0){
      _data_timestamp.load(xml_node_iter);
    }else{
      THROW_PARSER_ERROR(xml_node_iter, "Unknown node [" << node_name << "] in tag [" << node::DIS_Stream << "]");
    }
  }
  // Sort according to ByteOffset
  std::sort(_signal_list.begin(), _signal_list.end(),
            [](const std::unique_ptr<Signal>& a, const std::unique_ptr<Signal>& b) {
              return (a == nullptr || b == nullptr) ? false : (a->_byte_offset < b->_byte_offset);
            });
  // Update position attribute
  uint32_t p = 0;
  for(auto & s : _signal_list){
    s->_position = p++;
  }
}

//
// ANAStream
//
ed247::xml::ANAStream::ANAStream() :
  StreamSignals(ED247_STREAM_TYPE_ANALOG, 0)
{
}

void ed247::xml::ANAStream::load(const xmlNodePtr xml_node)
{
  for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
    auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
    if(attr_name.compare(attr::Name) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _name);
    }else if(attr_name.compare(attr::Direction) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _direction);
    }else if(attr_name.compare(attr::Comment) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _comment);
    }else if(attr_name.compare(attr::ICD) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _icd);
    }else if(attr_name.compare(attr::SampleMaxNumber) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _sample_max_number);
    }else if(attr_name.compare(attr::SampleMaxSizeBytes) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _sample_max_size_bytes);
    }else if(attr_name.compare(attr::UID) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _uid);
    }else{
      THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::ANA_Stream << "]");
    }
  }

  for(auto xml_node_iter = xml_node->children ; xml_node_iter != nullptr ; xml_node_iter = xml_node_iter->next){
    if(xml_node_iter->type != XML_ELEMENT_NODE)
      continue;
    auto node_name = ::xml::xmlChar_as_string(xml_node_iter->name);
    if(node_name.compare(node::Signals) == 0){
      for(auto xml_node_child_iter = xml_node_iter->children ; xml_node_child_iter != nullptr ; xml_node_child_iter = xml_node_child_iter->next){
        if(xml_node_child_iter->type != XML_ELEMENT_NODE)
          continue;
        node_name = ::xml::xmlChar_as_string(xml_node_child_iter->name);
        for(auto xml_attr = xml_node_iter->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
          auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
          if(attr_name.compare(attr::SamplingPeriodUs) == 0){
            ::xml::xmlAttr_get_value( xml_attr, _sampling_period_us);
          }else{
            THROW_PARSER_ERROR(xml_node_child_iter, "Unknown attribute [" << attr_name << "] in tag [" << node::ANA_Stream << "]");
          }
        }
        if(node_name.compare(node::Signal) == 0){
          ANASignal* signal = new ANASignal();
          signal->load(xml_node_child_iter);
          if(signal->_byte_offset + signal->get_sample_max_size_bytes() > _sample_max_size_bytes)
            THROW_PARSER_ERROR(xml_node_child_iter, "Stream [" << _name << "] Signal [" << signal->_name << "]: "
                               "ByteOffset [" << signal->_byte_offset << "] + [" << signal->get_sample_max_size_bytes() << "] > "
                               "Stream SampleMaxSizeBytes [" << _sample_max_size_bytes << "]");
          _signal_list.emplace_back(signal);
        }else{
          THROW_PARSER_ERROR(xml_node_child_iter, "Unknown node [" << node_name << "] in tag [" << node::Signals << "]");
        }
      }
    }else if(node_name.compare(node::DataTimestamp) == 0){
      _data_timestamp.load(xml_node_iter);
    }else{
      THROW_PARSER_ERROR(xml_node_iter, "Unknown node [" << node_name << "] in tag [" << node::ANA_Stream << "]");
    }
  }
  // Sort according to ByteOffset
  std::sort(_signal_list.begin(), _signal_list.end(),
            [](const std::unique_ptr<Signal>& a, const std::unique_ptr<Signal>& b) {
              return (a == nullptr || b == nullptr) ? false : (a->_byte_offset < b->_byte_offset);
            });
  // Update position attribute
  uint32_t p = 0;
  for(auto & s : _signal_list){
    s->_position = p++;
  }
}

//
// NADStream
//
ed247::xml::NADStream::NADStream() :
  StreamSignals(ED247_STREAM_TYPE_NAD, 0)
{
}

void ed247::xml::NADStream::load(const xmlNodePtr xml_node)
{
  for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
    auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
    if(attr_name.compare(attr::Name) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _name);
    }else if(attr_name.compare(attr::Direction) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _direction);
    }else if(attr_name.compare(attr::Comment) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _comment);
    }else if(attr_name.compare(attr::ICD) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _icd);
    }else if(attr_name.compare(attr::SampleMaxNumber) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _sample_max_number);
    }else if(attr_name.compare(attr::SampleMaxSizeBytes) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _sample_max_size_bytes);
    }else if(attr_name.compare(attr::UID) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _uid);
    }else{
      THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::NAD_Stream << "]");
    }
  }

  for(auto xml_node_iter = xml_node->children ; xml_node_iter != nullptr ; xml_node_iter = xml_node_iter->next){
    if(xml_node_iter->type != XML_ELEMENT_NODE)
      continue;
    auto node_name = ::xml::xmlChar_as_string(xml_node_iter->name);
    if(node_name.compare(node::Signals) == 0){
      for(auto xml_node_child_iter = xml_node_iter->children ; xml_node_child_iter != nullptr ; xml_node_child_iter = xml_node_child_iter->next){
        if(xml_node_child_iter->type != XML_ELEMENT_NODE)
          continue;
        node_name = ::xml::xmlChar_as_string(xml_node_child_iter->name);
        for(auto xml_attr = xml_node_iter->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
          auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
          if(attr_name.compare(attr::SamplingPeriodUs) == 0){
            ::xml::xmlAttr_get_value( xml_attr, _sampling_period_us);
          }else{
            THROW_PARSER_ERROR(xml_node_child_iter, "Unknown attribute [" << attr_name << "] in tag [" << node::NAD_Stream << "]");
          }
        }
        if(node_name.compare(node::Signal) == 0){
          NADSignal* signal = new NADSignal();
          signal->load(xml_node_child_iter);
          if(signal->_byte_offset + signal->get_sample_max_size_bytes() > _sample_max_size_bytes)
            THROW_PARSER_ERROR(xml_node_child_iter, "Stream [" << _name << "] Signal [" << signal->_name << "]: "
                               "ByteOffset [" << signal->_byte_offset << "] + [" << signal->get_sample_max_size_bytes() << "] > "
                               "Stream SampleMaxSizeBytes [" << _sample_max_size_bytes << "]");
          _signal_list.emplace_back(signal);
        }else{
          THROW_PARSER_ERROR(xml_node_child_iter, "Unknown node [" << node_name << "] in tag [" << node::Signals << "]");
        }
      }
    }else if(node_name.compare(node::DataTimestamp) == 0){
      _data_timestamp.load(xml_node_iter);
    }else{
      THROW_PARSER_ERROR(xml_node_iter, "Unknown node [" << node_name << "] in tag [" << node::NAD_Stream << "]");
    }
  }
  // Sort according to ByteOffset
  std::sort(_signal_list.begin(), _signal_list.end(),
            [](const std::unique_ptr<Signal>& a, const std::unique_ptr<Signal>& b) {
              return (a == nullptr || b == nullptr) ? false : (a->_byte_offset < b->_byte_offset);
            });
  // Update position attribute
  uint32_t p = 0;
  for(auto & s : _signal_list){
    s->_position = p++;
  }
}

//
// VNADStream
//
ed247::xml::VNADStream::VNADStream() :
  StreamSignals(ED247_STREAM_TYPE_VNAD, 0)
{
}

void ed247::xml::VNADStream::load(const xmlNodePtr xml_node)
{
  for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
    auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
    if(attr_name.compare(attr::Name) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _name);
    }else if(attr_name.compare(attr::Direction) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _direction);
    }else if(attr_name.compare(attr::Comment) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _comment);
    }else if(attr_name.compare(attr::ICD) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _icd);
    }else if(attr_name.compare(attr::SampleMaxNumber) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _sample_max_number);
    }else if(attr_name.compare(attr::UID) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _uid);
    }else{
      THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::VNAD_Stream << "]");
    }
  }

  for(auto xml_node_iter = xml_node->children ; xml_node_iter != nullptr ; xml_node_iter = xml_node_iter->next){
    if(xml_node_iter->type != XML_ELEMENT_NODE)
      continue;
    auto node_name = ::xml::xmlChar_as_string(xml_node_iter->name);
    if(node_name.compare(node::Signals) == 0){
      for(auto xml_node_child_iter = xml_node_iter->children ; xml_node_child_iter != nullptr ; xml_node_child_iter = xml_node_child_iter->next){
        if(xml_node_child_iter->type != XML_ELEMENT_NODE)
          continue;
        node_name = ::xml::xmlChar_as_string(xml_node_child_iter->name);
        for(auto xml_attr = xml_node_iter->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
          auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
          if(attr_name.compare(attr::SamplingPeriodUs) == 0){
            ::xml::xmlAttr_get_value( xml_attr, _sampling_period_us);
          }else{
            THROW_PARSER_ERROR(xml_node_child_iter, "Unknown attribute [" << attr_name << "] in tag [" << node::VNAD_Stream << "]");
          }
        }
        if(node_name.compare(node::Signal) == 0){
          VNADSignal* signal = new VNADSignal();
          signal->load(xml_node_child_iter);
          _sample_max_size_bytes += (signal->get_sample_max_size_bytes() + sizeof(uint16_t)) * signal->_vnad_max_number;
          _signal_list.emplace_back(signal);
        }else{
          THROW_PARSER_ERROR(xml_node_child_iter, "Unknown node [" << node_name << "] in tag [" << node::Signals << "]");
        }
      }
      PRINT_DEBUG("VNAD Stream [" << _name << "] SampleMaxSizeBytes[" << _sample_max_size_bytes << "] SampleMaxNumber[" << _sample_max_number << "]");
    }else if(node_name.compare(node::DataTimestamp) == 0){
      _data_timestamp.load(xml_node_iter);
    }else{
      THROW_PARSER_ERROR(xml_node_iter, "Unknown node [" << node_name << "] in tag [" << node::VNAD_Stream << "]");
    }
  }
  // Sort according to VNAD position
  std::sort(_signal_list.begin(), _signal_list.end(),
            [](const std::unique_ptr<Signal>& a, const std::unique_ptr<Signal>& b) {
              return (a == nullptr || b == nullptr) ? false : (a->_vnad_position < b->_vnad_position);
            });

  // Update position attribute
  uint32_t p = 0;
  for(auto & s : _signal_list){
    s->_position = p++;
  }
}

//
// Header
//
ed247::xml::Header::Header() :
  _enable(ED247_YESNO_NO),
  _transport_timestamp(ED247_YESNO_NO)
{
}

void ed247::xml::Header::load(const xmlNodePtr xml_node)
{
  for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
    auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
    if(attr_name.compare(attr::Enable) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _enable);
    }else if(attr_name.compare(attr::TransportTimestamp) == 0){
      ::xml::xmlAttr_get_value(xml_attr, _transport_timestamp);
    }else{
      THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::Header << "]");
    }
  }
}

//
// Channel
//
ed247::xml::Channel::Channel() :
  _frame_standard_revision(ED247_STANDARD__INVALID)
{
}

void ed247::xml::Channel::load(const xmlNodePtr xml_node)
{
  ed247_direction_t streams_direction(ED247_DIRECTION__INVALID);

  for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
    auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
    if(attr_name.compare(attr::Name) == 0){
      ::xml::xmlAttr_get_value( xml_attr, _name);
    }else if(attr_name.compare(attr::Comment) == 0){
      ::xml::xmlAttr_get_value( xml_attr, _comment);
    }else{
      THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::MultiChannel << "] or [" << node::Channel << "]");
    }
  }

  for(auto xml_node_iter = xml_node->children ; xml_node_iter != nullptr ; xml_node_iter = xml_node_iter->next){
    if(xml_node_iter->type != XML_ELEMENT_NODE)
      continue;
    auto node_name = ::xml::xmlChar_as_string(xml_node_iter->name);
    if(node_name.compare(node::ComInterface) == 0){
      _com_interface.load(xml_node_iter);
    }else if(node_name.compare(node::Header) == 0){
      _header.load(xml_node_iter);
    }else if(node_name.compare(node::FrameFormat) == 0){
      for(auto xml_attr = xml_node_iter->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
        auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
        if(attr_name.compare(attr::StandardRevision) == 0){
          ::xml::xmlAttr_get_value( xml_attr, _frame_standard_revision);
        }else{
          THROW_PARSER_ERROR(xml_node_iter, "Unknown attribute [" << attr_name << "] in tag [" << node::FrameFormat <<"]");
        }
      }
    }
    else if((_simple == false && node_name.compare(node::Streams) == 0) ||
            (_simple == true  && node_name.compare(node::Stream) == 0))
    {
      for(auto xml_node_child_iter = xml_node_iter->children ; xml_node_child_iter != nullptr ; xml_node_child_iter = xml_node_child_iter->next){
        if(xml_node_child_iter->type != XML_ELEMENT_NODE)
          continue;
        node_name = ::xml::xmlChar_as_string(xml_node_child_iter->name);
        // A429
        if(node_name.compare(node::A429_Stream) == 0){
          A429Stream* stream = new A429Stream();
          stream->load(xml_node_child_iter);
          _stream_list.emplace_back(stream);
          // A664
        }else if(node_name.compare(node::A664_Stream) == 0){
          A664Stream* stream = new A664Stream();
          stream->load(xml_node_child_iter);
          _stream_list.emplace_back(stream);
          // A825
        }else if(node_name.compare(node::A825_Stream) == 0){
          A825Stream* stream = new A825Stream();
          stream->load(xml_node_child_iter);
          _stream_list.emplace_back(stream);
          // SERIAL
        }else if(node_name.compare(node::SERIAL_Stream) == 0){
          SERIALStream* stream = new SERIALStream();
          stream->load(xml_node_child_iter);
          _stream_list.emplace_back(stream);
          // DISCRETE
        }else if(node_name.compare(node::DIS_Stream) == 0){
          DISStream* stream = new DISStream();
          stream->load(xml_node_child_iter);
          _stream_list.emplace_back(stream);
          // ANALOG
        }else if(node_name.compare(node::ANA_Stream) == 0){
          ANAStream* stream = new ANAStream();
          stream->load(xml_node_child_iter);
          _stream_list.emplace_back(stream);
          // NAD
        }else if(node_name.compare(node::NAD_Stream) == 0){
          NADStream* stream = new NADStream();
          stream->load(xml_node_child_iter);
          _stream_list.emplace_back(stream);
          // VNAD
        }else if(node_name.compare(node::VNAD_Stream) == 0){
          VNADStream* stream = new VNADStream();
          stream->load(xml_node_child_iter);
          _stream_list.emplace_back(stream);
          // Otherwise
        }else{
          THROW_PARSER_ERROR(xml_node_child_iter, "Unknown node [" << node_name << "] in tag [" << node::Streams << "]");
        }
        streams_direction = (ed247_direction_t)(streams_direction | _stream_list.back()->_direction);
      }
    }else{
      THROW_PARSER_ERROR(xml_node_iter, "Unexpected node [" << node_name << "]");
    }
  }

  //
  // Consolidate direction
  //

  // Set a direction on UdpSockets without direction & look for overall ComInterface direction
  ed247_direction_t com_interface_direction(ED247_DIRECTION__INVALID);
  for(UdpSocket& udp_socket : _com_interface._udp_sockets) {
    if (udp_socket._direction == ED247_DIRECTION__INVALID) {
      if (streams_direction == ED247_DIRECTION__INVALID || streams_direction == ED247_DIRECTION_INOUT)
        THROW_PARSER_ERROR(xml_node, "Cannot decide UdpSocket " << udp_socket._dst_ip_address << ":" <<
                           udp_socket._dst_ip_port << " direction for channel " << _name);
      udp_socket._direction = streams_direction;
    }
    com_interface_direction = (ed247_direction_t)(com_interface_direction | udp_socket._direction);
  }

  // Set a direction on Steams without direction & look for overall streams direction
  // This prevent the creation of bidirectional streams when channel has only one-way UdpSockets
  for (std::unique_ptr<Stream>& stream : _stream_list) {
    if (stream->_direction == ED247_DIRECTION__INVALID) {
      stream->_direction = com_interface_direction;
    }
    streams_direction = (ed247_direction_t)(streams_direction | _stream_list.back()->_direction);
  }

  // Warn if some streams are not able to communicate
  if (((streams_direction       & ED247_DIRECTION_IN) != 0) &&
      ((com_interface_direction & ED247_DIRECTION_IN) == 0))
    PARSER_WARNING(xml_node, "Channel " << _name << " has input streams without input UdpSockets.");

  if (((streams_direction       & ED247_DIRECTION_OUT) != 0) &&
      ((com_interface_direction & ED247_DIRECTION_OUT) == 0))
    PARSER_WARNING(xml_node, "Channel " << _name << " has output streams without output UdpSockets.");
}

//
// Component
//
ed247::xml::Component::Component() :
  _identifier(0),
  _component_type(ED247_COMPONENT_TYPE_VIRTUAL),
  _standard_revision(ED247_STANDARD__INVALID)
{
}

void ed247::xml::Component::load(const xmlNodePtr xml_node)
{
  for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
    auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
    if(attr_name.compare("noNamespaceSchemaLocation") == 0 || attr_name.compare("xsi") == 0)
      continue;
    if(attr_name.compare(attr::Name) == 0){
      ::xml::xmlAttr_get_value( xml_attr, _name);
    }else if(attr_name.compare(attr::ComponentVersion) == 0){
      ::xml::xmlAttr_get_value( xml_attr, _version);
    }else if(attr_name.compare(attr::ComponentType) == 0){
      ::xml::xmlAttr_get_value( xml_attr, _component_type);
    }else if(attr_name.compare(attr::Comment) == 0){
      ::xml::xmlAttr_get_value( xml_attr, _comment);
    }else if(attr_name.compare(attr::StandardRevision) == 0){
      ::xml::xmlAttr_get_value( xml_attr, _standard_revision);
    }else if(attr_name.compare(attr::Identifier) == 0){
      ::xml::xmlAttr_get_value( xml_attr, _identifier);
    }else{
      THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::ED247ComponentInstanceConfiguration <<"]");
    }
  }
  if(_standard_revision != ED247_STANDARD_ED247A)
    THROW_PARSER_ERROR(xml_node, "This version do not support any other standard than [" << std::string(ed247_standard_string(ED247_STANDARD_ED247A)) << "]");

  for(auto xml_node_iter = xml_node->children ; xml_node_iter != nullptr ; xml_node_iter = xml_node_iter->next){
    if(xml_node_iter->type != XML_ELEMENT_NODE)
      continue;
    auto node_name = ::xml::xmlChar_as_string(xml_node_iter->name);
    if(node_name.compare(node::Channels) == 0){
      for(auto xml_node_channel = xml_node_iter->children ; xml_node_channel != nullptr ; xml_node_channel = xml_node_channel->next){
        if(xml_node_channel->type != XML_ELEMENT_NODE)
          continue;
        node_name = ::xml::xmlChar_as_string(xml_node_channel->name);
        if(node_name.compare(node::MultiChannel) == 0) {
          _channel_list.emplace_back();
          Channel& channel = _channel_list.back();
          channel._simple = false; // store if it is a simple channel (only one stream)
          channel.load(xml_node_channel);
          if(channel._frame_standard_revision != ED247_STANDARD_ED247A)
            THROW_PARSER_ERROR(xml_node_channel, "This version do not support any other standard than [" << std::string(ed247_standard_string(ED247_STANDARD_ED247A)) << "]");
        }else if(node_name.compare(node::Channel) == 0) {
          _channel_list.emplace_back();
          Channel& channel = _channel_list.back();
          channel._simple = true; // store if it is a simple channel (only one stream)
          channel.load(xml_node_channel);
          if(channel._frame_standard_revision != ED247_STANDARD_ED247A)
            THROW_PARSER_ERROR(xml_node_channel, "This version do not support any other standard than [" << std::string(ed247_standard_string(ED247_STANDARD_ED247A)) << "]");
        }else{
          THROW_PARSER_ERROR(xml_node_channel, "Unknown node [" << node_name << "] in tag [" << node::Channels << "]");
        }
      }
    }else if(node_name.compare(node::FileProducer) == 0){
      for(auto xml_attr = xml_node_iter->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
        auto attr_name = ::xml::xmlChar_as_string(xml_attr->name);
        if(attr_name.compare(attr::Identifier) == 0){
          ::xml::xmlAttr_get_value( xml_attr, _file_producer_identifier);
        }else if(attr_name.compare(attr::Comment) == 0){
          ::xml::xmlAttr_get_value( xml_attr, _file_producer_comment);
        }else{
          THROW_PARSER_ERROR(xml_node_iter, "Unknown attribute [" << attr_name << "] in tag [" << node::FileProducer <<"]");
        }
      }
    }else{
      THROW_PARSER_ERROR(xml_node_iter, "Unknown node [" << node_name << "] in tag [" << node::ED247ComponentInstanceConfiguration << "]");
    }
  }
}

//
// load
//
static std::unique_ptr<ed247::xml::Component> ed247_ecic_load(xmlParserCtxtPtr & p_xml_context, xmlDocPtr & p_xml_doc)
{
  xmlDocPtr              p_xsd_doc;
  xmlSchemaParserCtxtPtr p_xsd_schema_parser;
  xmlSchemaPtr           p_xsd_schema;
  xmlSchemaValidCtxtPtr  p_xsd_valid_context;

  // Parse XSD
  if((p_xsd_doc = xmlCtxtReadMemory(p_xml_context,xsd_schema,(int)strlen(xsd_schema),nullptr,nullptr,0)) == nullptr)
    THROW_PARSER_ERROR(nullptr, "Failed to load schema in memory");

  // Validate XSD
  if((p_xsd_schema_parser = xmlSchemaNewDocParserCtxt(p_xsd_doc)) == nullptr)
    THROW_PARSER_ERROR(nullptr, "Failed to create schema parser");
  if((p_xsd_schema = xmlSchemaParse(p_xsd_schema_parser)) == nullptr)
    THROW_PARSER_ERROR(nullptr, "Failed to create schema");
  if((p_xsd_valid_context = xmlSchemaNewValidCtxt(p_xsd_schema)) == nullptr)
    THROW_PARSER_ERROR(nullptr, "Failed to validate schema context");

  // Validate XML
  if(xmlSchemaValidateDoc(p_xsd_valid_context, p_xml_doc) != 0)
    THROW_PARSER_ERROR(nullptr, "Failed to validate XML document");

  // Load Nodes
  xmlNodePtr xmlRootNode(xmlDocGetRootElement(p_xml_doc));

  std::unique_ptr<ed247::xml::Component> root(new ed247::xml::Component());
  root->load(xmlRootNode);

  return root;
}

std::unique_ptr<ed247::xml::Component> ed247::xml::load_filepath(const std::string & filepath)
{
  xmlParserCtxtPtr    p_xml_context;
  xmlDocPtr           p_xml_doc;

  // Create context
  if((p_xml_context = xmlNewParserCtxt()) == nullptr)
    THROW_PARSER_ERROR(nullptr, "Failed to create XML context pointer");

  // Setup error handler
  xmlSetStructuredErrorFunc(nullptr,&libxml_structured_error);

  // Parse XML
  if((p_xml_doc = xmlCtxtReadFile(p_xml_context,filepath.c_str(),NULL,0)) == nullptr)
    THROW_PARSER_ERROR(nullptr, "Failed to read [" << filepath << "]");

  // Store filename for debugging purpose
  if (p_xml_doc->name == nullptr) {
    p_xml_doc->name = strdup(filepath.c_str());
  }

  std::unique_ptr<ed247::xml::Component> root = ed247_ecic_load(p_xml_context, p_xml_doc);

  if(p_xml_doc)
    xmlFreeDoc(p_xml_doc);
  if(p_xml_context)
    xmlFreeParserCtxt(p_xml_context);

  return root;
}

std::unique_ptr<ed247::xml::Component> ed247::xml::load_content(const std::string & content)
{
  xmlParserCtxtPtr    p_xml_context;
  xmlDocPtr           p_xml_doc;

  // Create context
  if((p_xml_context = xmlNewParserCtxt()) == nullptr)
    THROW_PARSER_ERROR(nullptr, "Failed to create XML context pointer");

  // Setup error handler
  xmlSetStructuredErrorFunc(nullptr,&libxml_structured_error);

  // Parse XML
  if((p_xml_doc = xmlCtxtReadMemory(p_xml_context,content.c_str(),(int)content.length(),nullptr,nullptr,0)) == nullptr)
    THROW_PARSER_ERROR(nullptr, "Failed to read XML file content");

  std::unique_ptr<ed247::xml::Component> root = ed247_ecic_load(p_xml_context, p_xml_doc);

  if(p_xml_doc)
    xmlFreeDoc(p_xml_doc);
  if(p_xml_context)
    xmlFreeParserCtxt(p_xml_context);

  return root;
}
