/* -*- mode: c++; c-basic-offset: 4 -*-  */
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

#include <libxml/xmlschemas.h>

#include "ed247_xsd.h"
#include "ed247_signal.h"

#include <sstream>
#include <istream>
#include <memory>

#define DECL_STREAM_OPERATOR(x,strx,xstr)                       \
    std::istream & operator >> (std::istream & is, x & e)       \
    {                                                           \
        std::string str;                                        \
        is >> str;                                              \
        e = strx(str.c_str());                                  \
        return is;                                              \
    }

std::string node_2_fileline(xmlNode* node) {
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

#define THROW_PARSER_ERROR(closest_node, msg)                                            \
  do {                                                                                   \
    std::string message = strize() << msg;                                               \
    if (libxml_error.file != nullptr) {                                                  \
      message += strize() << ". From " << libxml_error.file << ':' << libxml_error.line; \
    } else if (closest_node != nullptr) {                                                \
      message += std::string(". Near ") + node_2_fileline(closest_node);                 \
    }                                                                                    \
    if (libxml_error.message != nullptr) {                                               \
      message += strize() << " " << libxml_error.message;                                \
    }                                                                                    \
    PRINT_ERROR(message);                                                                \
    throw xml::exception(message);                                                       \
  } while (0)


DECL_STREAM_OPERATOR(
    ed247_component_type_t,
    ed247_component_type_from_string,
    ed247_component_type_string
)

DECL_STREAM_OPERATOR(
    ed247_standard_t,
    ed247_standard_from_string,
    ed247_standard_string
)

DECL_STREAM_OPERATOR(
    ed247_yesno_t,
    ed247_yesno_from_string,
    ed247_yesno_string
)

DECL_STREAM_OPERATOR(
    ed247_direction_t,
    ed247_direction_from_string,
    ed247_direction_string
)

DECL_STREAM_OPERATOR(
    ed247_nad_type_t,
    ed247_nad_type_from_string,
    ed247_nad_type_string
)

static xmlError libxml_error;
static void libxml_structured_error(void * user_data, xmlErrorPtr error)
{
    _UNUSED(user_data);
    xmlResetError(&libxml_error);
    xmlCopyError(error, &libxml_error);
}

namespace ed247
{

namespace xml
{

std::string xmlChar2string(const xmlChar *str)
{
    if (!str) return std::string();
    return std::string((const char *)str);
}

size_t nad_type_size(ed247_nad_type_t type){
    switch(type){
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

template<typename T>
void set_value(T & variable, const xmlAttrPtr attribute)
{
    std::string value = xmlChar2string(xmlGetProp(attribute->parent,attribute->name));
    std::stringstream(value) >> variable;
}

template<>
void set_value(const char * & variable, const xmlAttrPtr attribute)
{
    xmlChar * value = xmlGetProp(attribute->parent,attribute->name);
    variable = reinterpret_cast<const char *>(value);
}

template<>
void set_value(std::string& variable, const xmlAttrPtr attribute)
{
    variable = xmlChar2string(xmlGetProp(attribute->parent,attribute->name));
}


// Node

void Node::load(const xmlNodePtr xml_node)
{
    reset();
    fill_attributes(xml_node);
    create_children(xml_node);
}

// DataTimestamp

void DataTimestamp::reset()
{
    enable = ED247_YESNO_NO;
    enable_sample_offset = ED247_YESNO_NO;
}

void DataTimestamp::fill_attributes(const xmlNodePtr xml_node)
{
    for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
        auto attr_name = xmlChar2string(xml_attr->name);
        if(attr_name.compare(attr::Enable) == 0){
            set_value(enable,xml_attr);
        }else if(attr_name.compare(attr::SampleDataTimestampOffset) == 0){
            set_value(enable_sample_offset,xml_attr);
        }else{
          THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::DataTimestamp << "]");
        }
    }
}

void DataTimestamp::create_children(const xmlNodePtr xml_node)
{
    _UNUSED(xml_node);
}

// Errors

void Errors::reset()
{
    enable = ED247_YESNO_NO;
}

void Errors::fill_attributes(const xmlNodePtr xml_node)
{
    for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
        auto attr_name = xmlChar2string(xml_attr->name);
        if(attr_name.compare(attr::Enable) == 0){
            set_value(enable,xml_attr);
        }else{
          THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::Errors << "]");
        }
    }
}

void Errors::create_children(const xmlNodePtr xml_node)
{
    _UNUSED(xml_node);
}

// UdpSocket

std::string UdpSocket::toString() const
{
  return strize() << "UdpSocket - DstIP[" << dst_ip_address << "] DstPort[" << dst_ip_port<< "] SrcIP[" << src_ip_address << "] SrcPort[" << src_ip_port << "] MulticastInterfaceIP[" << mc_ip_address << "] MulticastTTL[" << mc_ttl << "]";
}

void UdpSocket::reset()
{
    dst_ip_address.clear();
    dst_ip_port = 0;
    src_ip_address.clear();
    src_ip_port = 0;
    mc_ip_address.clear();
    mc_ttl = 1;
    direction = ED247_DIRECTION_INOUT;
}

void UdpSocket::fill_attributes(const xmlNodePtr xml_node)
{
    for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
        auto attr_name = xmlChar2string(xml_attr->name);
        if(attr_name.compare(attr::DstIP) == 0){
            set_value(dst_ip_address,xml_attr);
        }else if(attr_name.compare(attr::DstPort) == 0){
            set_value(dst_ip_port,xml_attr);
        }else if(attr_name.compare(attr::SrcIP) == 0){
            set_value(src_ip_address,xml_attr);
        }else if(attr_name.compare(attr::SrcPort) == 0){
            set_value(src_ip_port,xml_attr);
        }else if(attr_name.compare(attr::MulticastInterfaceIP) == 0){
            set_value(mc_ip_address,xml_attr);
        }else if(attr_name.compare(attr::MulticastTTL) == 0){
            set_value(mc_ttl,xml_attr);
        }else if(attr_name.compare(attr::Direction) == 0){
            set_value(direction,xml_attr);
        }else{
            THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::UdpSocket << "]");
        }
    }
}

void UdpSocket::create_children(const xmlNodePtr xml_node)
{
    _UNUSED(xml_node);
}

// ComInterface

void ComInterface::reset()
{
    udp_sockets.clear();
}

void ComInterface::fill_attributes(const xmlNodePtr xml_node)
{
    _UNUSED(xml_node);
}

void ComInterface::create_children(const xmlNodePtr xml_node)
{
    for(auto xml_node_iter = xml_node->children ; xml_node_iter != nullptr ; xml_node_iter = xml_node_iter->next){
        if(xml_node_iter->type != XML_ELEMENT_NODE)
            continue;
        auto node_name = xmlChar2string(xml_node_iter->name);
        if(node_name.compare(node::UdpSockets) == 0){
            for(auto xml_node_child_iter = xml_node_iter->children ; xml_node_child_iter != nullptr ; xml_node_child_iter = xml_node_child_iter->next){
                if(xml_node_child_iter->type != XML_ELEMENT_NODE)
                    continue;
                node_name = xmlChar2string(xml_node_child_iter->name);
                if(node_name.compare(node::UdpSocket) == 0){
                    std::unique_ptr<UdpSocket> udp_socket = std::make_unique<UdpSocket>();
                    udp_socket->load(xml_node_child_iter);
                    udp_sockets.push_back(std::move(udp_socket));
                }else{
                    THROW_PARSER_ERROR(xml_node_child_iter, "Unknown node [" << node_name << "] in tag [" << node::UdpSockets << "]");
                }
            }
        }else{
            THROW_PARSER_ERROR(xml_node_iter, "Unknown node [" << node_name << "] in tag [" << node::ComInterface << "]");
        }
    }
}

// A429Stream

void A429Stream::reset()
{
    info = LIBED247_STREAM_INFO_DEFAULT;
    info.type = ED247_STREAM_TYPE_A429;
    info.sample_max_size_bytes = 4;
}

void A429Stream::fill_attributes(const xmlNodePtr xml_node)
{
    for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
        auto attr_name = xmlChar2string(xml_attr->name);
        if(attr_name.compare(attr::Name) == 0){
            set_value(info.name,xml_attr);
        }else if(attr_name.compare(attr::Direction) == 0){
            set_value(info.direction,xml_attr);
        }else if(attr_name.compare(attr::Comment) == 0){
            set_value(info.comment,xml_attr);
        }else if(attr_name.compare(attr::ICD) == 0){
            set_value(info.icd,xml_attr);
        }else if(attr_name.compare(attr::SampleMaxNumber) == 0){
            set_value(info.sample_max_number,xml_attr);
        }else if(attr_name.compare(attr::UID) == 0){
            set_value(info.uid,xml_attr);
        }else{
            THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::A429_Stream << "]");
        }
    }
}

void A429Stream::create_children(const xmlNodePtr xml_node)
{
    for(auto xml_node_iter = xml_node->children ; xml_node_iter != nullptr ; xml_node_iter = xml_node_iter->next){
        if(xml_node_iter->type != XML_ELEMENT_NODE)
            continue;
        auto node_name = xmlChar2string(xml_node_iter->name);
        if(node_name.compare(node::DataTimestamp) == 0){
            data_timestamp.load(xml_node_iter);
        }else if(node_name.compare(node::Errors) == 0){
            errors.load(xml_node_iter);
        }else{
            THROW_PARSER_ERROR(xml_node_iter, "Unknown node [" << node_name << "] in tag [" << node::A429_Stream << "]");
        }
    }
}

// A664Stream

void A664Stream::reset()
{
    info = LIBED247_STREAM_INFO_DEFAULT;
    info.type = ED247_STREAM_TYPE_A664;
    info.sample_max_size_bytes = 1471;
    enable_message_size = ED247_YESNO_YES;
}

void A664Stream::fill_attributes(const xmlNodePtr xml_node)
{
    for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
        auto attr_name = xmlChar2string(xml_attr->name);
        if(attr_name.compare(attr::Name) == 0){
            set_value(info.name,xml_attr);
        }else if(attr_name.compare(attr::Direction) == 0){
            set_value(info.direction,xml_attr);
        }else if(attr_name.compare(attr::Comment) == 0){
            set_value(info.comment,xml_attr);
        }else if(attr_name.compare(attr::ICD) == 0){
            set_value(info.icd,xml_attr);
        }else if(attr_name.compare(attr::SampleMaxNumber) == 0){
            set_value(info.sample_max_number,xml_attr);
        }else if(attr_name.compare(attr::SampleMaxSizeBytes) == 0){
            set_value(info.sample_max_size_bytes,xml_attr);
        }else if(attr_name.compare(attr::UID) == 0){
            set_value(info.uid,xml_attr);
        }else{
            THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::A664_Stream << "]");
        }
    }
}

void A664Stream::create_children(const xmlNodePtr xml_node)
{
    for(auto xml_node_iter = xml_node->children ; xml_node_iter != nullptr ; xml_node_iter = xml_node_iter->next){
        if(xml_node_iter->type != XML_ELEMENT_NODE)
            continue;
        auto node_name = xmlChar2string(xml_node_iter->name);
        if(node_name.compare(node::DataTimestamp) == 0){
            data_timestamp.load(xml_node_iter);
        }else if(node_name.compare(node::Errors) == 0){
            errors.load(xml_node_iter);
        }else if(node_name.compare(node::MessageSize) == 0){
            for(auto xml_attr = xml_node_iter->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
                auto attr_name = xmlChar2string(xml_attr->name);
                if(attr_name.compare(attr::Enable) == 0){
                    set_value(enable_message_size,xml_attr);
                }else{
                    THROW_PARSER_ERROR(xml_node_iter, "Unknown attribute [" << attr_name << "] in tag [" << node::MessageSize << "]");
                }
            }
        }else{
            THROW_PARSER_ERROR(xml_node_iter, "Unknown node [" << node_name << "] in tag [" << node::A664_Stream << "]");
        }
    }
    // Check message enable size & sample_max_number
    if(enable_message_size == ED247_YESNO_NO && info.sample_max_number > 1) {
      THROW_PARSER_ERROR(xml_node, "A664 Stream cannot have a SampleMaxNumber of [" << info.sample_max_number << "] and not enabling Message Size encoding in frame");
    }
}

// A825Stream

void A825Stream::reset()
{
    info = LIBED247_STREAM_INFO_DEFAULT;
    info.type = ED247_STREAM_TYPE_A825;
    info.direction = ED247_DIRECTION_INOUT;
    info.sample_max_size_bytes = 69;
}

void A825Stream::fill_attributes(const xmlNodePtr xml_node)
{
    for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
        auto attr_name = xmlChar2string(xml_attr->name);
        if(attr_name.compare(attr::Name) == 0){
            set_value(info.name,xml_attr);
        }else if(attr_name.compare(attr::Direction) == 0){
            set_value(info.direction,xml_attr);
        }else if(attr_name.compare(attr::Comment) == 0){
            set_value(info.comment,xml_attr);
        }else if(attr_name.compare(attr::ICD) == 0){
            set_value(info.icd,xml_attr);
        }else if(attr_name.compare(attr::SampleMaxNumber) == 0){
            set_value(info.sample_max_number,xml_attr);
        }else if(attr_name.compare(attr::UID) == 0){
            set_value(info.uid,xml_attr);
        }else{
            THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::A825_Stream << "]");
        }
    }
}

void A825Stream::create_children(const xmlNodePtr xml_node)
{
    for(auto xml_node_iter = xml_node->children ; xml_node_iter != nullptr ; xml_node_iter = xml_node_iter->next){
        if(xml_node_iter->type != XML_ELEMENT_NODE)
            continue;
        auto node_name = xmlChar2string(xml_node_iter->name);
        if(node_name.compare(node::DataTimestamp) == 0){
            data_timestamp.load(xml_node_iter);
        }else if(node_name.compare(node::Errors) == 0){
            errors.load(xml_node_iter);
        }else{
            THROW_PARSER_ERROR(xml_node_iter, "Unknown node [" << node_name << "] in tag [" << node::A825_Stream << "]");
        }
    }
}

// SERIALStream

void SERIALStream::reset()
{
    info = LIBED247_STREAM_INFO_DEFAULT;
    info.type = ED247_STREAM_TYPE_SERIAL;
    info.direction = ED247_DIRECTION_INOUT;
}

void SERIALStream::fill_attributes(const xmlNodePtr xml_node)
{
    for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
        auto attr_name = xmlChar2string(xml_attr->name);
        if(attr_name.compare(attr::Name) == 0){
            set_value(info.name,xml_attr);
        }else if(attr_name.compare(attr::Direction) == 0){
            set_value(info.direction,xml_attr);
        }else if(attr_name.compare(attr::Comment) == 0){
            set_value(info.comment,xml_attr);
        }else if(attr_name.compare(attr::ICD) == 0){
            set_value(info.icd,xml_attr);
        }else if(attr_name.compare(attr::SampleMaxNumber) == 0){
            set_value(info.sample_max_number,xml_attr);
        }else if(attr_name.compare(attr::SampleMaxSizeBytes) == 0){
            set_value(info.sample_max_size_bytes,xml_attr);
        }else if(attr_name.compare(attr::UID) == 0){
            set_value(info.uid,xml_attr);
        }else{
            THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::SERIAL_Stream << "]");
        }
    }
}

void SERIALStream::create_children(const xmlNodePtr xml_node)
{
    for(auto xml_node_iter = xml_node->children ; xml_node_iter != nullptr ; xml_node_iter = xml_node_iter->next){
        if(xml_node_iter->type != XML_ELEMENT_NODE)
            continue;
        auto node_name = xmlChar2string(xml_node_iter->name);
        if(node_name.compare(node::DataTimestamp) == 0){
            data_timestamp.load(xml_node_iter);
        }else if(node_name.compare(node::Errors) == 0){
            errors.load(xml_node_iter);
        }else{
            THROW_PARSER_ERROR(xml_node_iter, "Unknown node [" << node_name << "] in tag [" << node::SERIAL_Stream << "]");
        }
    }
}

// DISSignal

void DISSignal::reset()
{
    info.name = nullptr;
    info.type = ED247_SIGNAL_TYPE_DISCRETE;
    info.comment = "";
    info.icd = "";
    info.info.dis = LIBED247_SIGNAL_INFO_DIS_DEFAULT;
}

void DISSignal::fill_attributes(const xmlNodePtr xml_node)
{
    for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
        auto attr_name = xmlChar2string(xml_attr->name);
        if(attr_name.compare(attr::Name) == 0){
            set_value(info.name,xml_attr);
        }else if(attr_name.compare(attr::Comment) == 0){
            set_value(info.comment,xml_attr);
        }else if(attr_name.compare(attr::ICD) == 0){
            set_value(info.icd,xml_attr);
        }else if(attr_name.compare(attr::ByteOffset) == 0){
            set_value(info.info.dis.byte_offset,xml_attr);
        }else{
            THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::Signal << "]");
        }
    }
}

void DISSignal::create_children(const xmlNodePtr xml_node)
{
    _UNUSED(xml_node);
}

// ANASignal

void ANASignal::reset()
{
    info.name = nullptr;
    info.type = ED247_SIGNAL_TYPE_ANALOG;
    info.comment = "";
    info.icd = "";
    info.info.ana = LIBED247_SIGNAL_INFO_ANA_DEFAULT;
}

void ANASignal::fill_attributes(const xmlNodePtr xml_node)
{
    for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
        auto attr_name = xmlChar2string(xml_attr->name);
        if(attr_name.compare(attr::Name) == 0){
            set_value(info.name,xml_attr);
        }else if(attr_name.compare(attr::Comment) == 0){
            set_value(info.comment,xml_attr);
        }else if(attr_name.compare(attr::ICD) == 0){
            set_value(info.icd,xml_attr);
        }else if(attr_name.compare(attr::ElectricalUnit) == 0){
            set_value(info.info.ana.electrical_unit,xml_attr);
        }else if(attr_name.compare(attr::ByteOffset) == 0){
            set_value(info.info.ana.byte_offset,xml_attr);
        }else{
            THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::Signal << "]");
        }
    }
}

void ANASignal::create_children(const xmlNodePtr xml_node)
{
    _UNUSED(xml_node);
}

// NADSignal

NADSignal::~NADSignal()
{
    if (info.info.nad.dimensions != NULL)
    {
        free(info.info.nad.dimensions);
        info.info.nad.dimensions = NULL;
    }
}

void NADSignal::reset()
{
    info.name = nullptr;
    info.type = ED247_SIGNAL_TYPE_NAD;
    info.comment = "";
    info.icd = "";
    info.info.nad = LIBED247_SIGNAL_INFO_NAD_DEFAULT;
    info.info.nad.dimensions_count = 1;
    info.info.nad.dimensions = (uint32_t*)calloc(info.info.nad.dimensions_count, sizeof(uint32_t));
    for (uint32_t i = 0; i < info.info.nad.dimensions_count; i++)
    {
        info.info.nad.dimensions[i] = 1;
    }
}

void NADSignal::fill_attributes(const xmlNodePtr xml_node)
{
    for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
        auto attr_name = xmlChar2string(xml_attr->name);
        if(attr_name.compare(attr::Name) == 0){
            set_value(info.name,xml_attr);
        }else if(attr_name.compare(attr::Comment) == 0){
            set_value(info.comment,xml_attr);
        }else if(attr_name.compare(attr::ICD) == 0){
            set_value(info.icd,xml_attr);
        }else if(attr_name.compare(attr::Dimensions) == 0){
            std::string testee(xmlChar2string(xmlGetProp(xml_node,xml_attr->name)));
            size_t pos = 0;
            uint32_t dimension;
            std::vector<uint32_t> dimensions;
            std::istringstream iss;
            // This bloc capture each dimension followed by a separator
            while((pos = testee.find_first_of("xX")) != std::string::npos){
                iss.str(testee.substr(0, pos));
                iss >> dimension;
                iss.clear(); // Reset the flags or iss will not be usable anymore
                dimensions.push_back(dimension);
                testee.erase(0,pos+1);
            }
            // Capture the last dimension separatedly because it is not followed by a separator
            iss.str(testee.substr(0, pos));
            iss >> dimension;
            dimensions.push_back(dimension);

            info.info.nad.dimensions_count = (uint32_t)dimensions.size();
            info.info.nad.dimensions = (uint32_t*)realloc(info.info.nad.dimensions,
                    sizeof(uint32_t)*info.info.nad.dimensions_count);
            pos = 0;
            for(auto dim : dimensions){
                info.info.nad.dimensions[pos++] = dim;
            }
        }else if(attr_name.compare(attr::Type) == 0){
            set_value(info.info.nad.nad_type,xml_attr);
        }else if(attr_name.compare(attr::Unit) == 0){
            set_value(info.info.nad.unit,xml_attr);
        }else if(attr_name.compare(attr::ByteOffset) == 0){
            set_value(info.info.nad.byte_offset,xml_attr);
        }else{
            THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::Signal << "]");
        }
    }
}

void NADSignal::create_children(const xmlNodePtr xml_node)
{
    _UNUSED(xml_node);
}

// VNADSignal

void VNADSignal::reset()
{
    info.name = nullptr;
    info.type = ED247_SIGNAL_TYPE_VNAD;
    info.comment = "";
    info.icd = "";
    info.info.vnad = LIBED247_SIGNAL_INFO_VNAD_DEFAULT;
}

void VNADSignal::fill_attributes(const xmlNodePtr xml_node)
{
    for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
        auto attr_name = xmlChar2string(xml_attr->name);
        if(attr_name.compare(attr::Name) == 0){
            set_value(info.name,xml_attr);
        }else if(attr_name.compare(attr::Comment) == 0){
            set_value(info.comment,xml_attr);
        }else if(attr_name.compare(attr::ICD) == 0){
            set_value(info.icd,xml_attr);
        }else if(attr_name.compare(attr::Type) == 0){
            set_value(info.info.vnad.nad_type,xml_attr);
        }else if(attr_name.compare(attr::Unit) == 0){
            set_value(info.info.vnad.unit,xml_attr);
        }else if(attr_name.compare(attr::MaxLength) == 0){
            set_value(info.info.vnad.max_length,xml_attr);
        }else if(attr_name.compare(attr::Position) == 0){
            set_value(info.info.vnad.position,xml_attr);
        }else{
            THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::Signal << "]");
        }
    }
}

void VNADSignal::create_children(const xmlNodePtr xml_node)
{
    _UNUSED(xml_node);
}

// DISStream

void DISStream::reset()
{
    info = LIBED247_STREAM_INFO_DEFAULT;
    info.type = ED247_STREAM_TYPE_DISCRETE;
    info.sample_max_size_bytes = 0;
    info.info.dis = LIBED247_STREAM_INFO_DIS_DEFAULT;
    signals.clear();
}

void DISStream::fill_attributes(const xmlNodePtr xml_node)
{
    for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
        auto attr_name = xmlChar2string(xml_attr->name);
        if(attr_name.compare(attr::Name) == 0){
            set_value(info.name,xml_attr);
        }else if(attr_name.compare(attr::Direction) == 0){
            set_value(info.direction,xml_attr);
        }else if(attr_name.compare(attr::Comment) == 0){
            set_value(info.comment,xml_attr);
        }else if(attr_name.compare(attr::ICD) == 0){
            set_value(info.icd,xml_attr);
        }else if(attr_name.compare(attr::SampleMaxNumber) == 0){
            set_value(info.sample_max_number,xml_attr);
        }else if(attr_name.compare(attr::SampleMaxSizeBytes) == 0){
            set_value(info.sample_max_size_bytes,xml_attr);
        }else if(attr_name.compare(attr::UID) == 0){
            set_value(info.uid,xml_attr);
        }else{
            THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::DIS_Stream << "]");
        }
    }
}

void DISStream::create_children(const xmlNodePtr xml_node)
{
    for(auto xml_node_iter = xml_node->children ; xml_node_iter != nullptr ; xml_node_iter = xml_node_iter->next){
        if(xml_node_iter->type != XML_ELEMENT_NODE)
            continue;
        auto node_name = xmlChar2string(xml_node_iter->name);
        if(node_name.compare(node::Signals) == 0){
            for(auto xml_node_child_iter = xml_node_iter->children ; xml_node_child_iter != nullptr ; xml_node_child_iter = xml_node_child_iter->next){
                if(xml_node_child_iter->type != XML_ELEMENT_NODE)
                    continue;
                node_name = xmlChar2string(xml_node_child_iter->name);
                for(auto xml_attr = xml_node_iter->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
                    auto attr_name = xmlChar2string(xml_attr->name);
                    if(attr_name.compare(attr::SamplingPeriodUs) == 0){
                        set_value(info.info.dis.sampling_period_us,xml_attr);
                    }else{
                        THROW_PARSER_ERROR(xml_node_child_iter, "Unknown attribute [" << attr_name << "] in tag [" << node::DIS_Stream << "]");
                    }
                }
                if(node_name.compare(node::Signal) == 0){
                    std::unique_ptr<DISSignal> signal = std::make_unique<DISSignal>();
                    signal->load(xml_node_child_iter);
                    if(signal->info.info.dis.byte_offset + BaseSignal::sample_max_size_bytes(signal->info) > info.sample_max_size_bytes)
                        THROW_PARSER_ERROR(xml_node_child_iter, "Stream [" << info.name << "] Signal [" << signal->info.name << "]: ByteOffset [" << signal->info.info.dis.byte_offset << "] + [" << BaseSignal::sample_max_size_bytes(signal->info) << "] > Stream SampleMaxSizeBytes [" << info.sample_max_size_bytes << "]");
                    signals.push_back(std::move(signal));
                }else{
                    THROW_PARSER_ERROR(xml_node_child_iter, "Unknown node [" << node_name << "] in tag [" << node::Signals << "]");
                }
            }
        }else if(node_name.compare(node::DataTimestamp) == 0){
            data_timestamp.load(xml_node_iter);
        }else{
            THROW_PARSER_ERROR(xml_node_iter, "Unknown node [" << node_name << "] in tag [" << node::DIS_Stream << "]");
        }
    }
    // Sort according to ByteOffset
    std::sort(signals.begin(), signals.end(), [](std::shared_ptr<Signal> a, std::shared_ptr<Signal>b){
        return (a == nullptr || b == nullptr) ? false : (a->info.info.dis.byte_offset < b->info.info.dis.byte_offset);
    });
    // Update position attribute
    size_t p = 0;
    for(auto & s : signals){
        s->position = p++;
    }
}

// ANAStream

void ANAStream::reset()
{
    info = LIBED247_STREAM_INFO_DEFAULT;
    info.type = ED247_STREAM_TYPE_ANALOG;
    info.sample_max_size_bytes = 0;
    info.info.ana = LIBED247_STREAM_INFO_ANA_DEFAULT;
    signals.clear();
}

void ANAStream::fill_attributes(const xmlNodePtr xml_node)
{
    for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
        auto attr_name = xmlChar2string(xml_attr->name);
        if(attr_name.compare(attr::Name) == 0){
            set_value(info.name,xml_attr);
        }else if(attr_name.compare(attr::Direction) == 0){
            set_value(info.direction,xml_attr);
        }else if(attr_name.compare(attr::Comment) == 0){
            set_value(info.comment,xml_attr);
        }else if(attr_name.compare(attr::ICD) == 0){
            set_value(info.icd,xml_attr);
        }else if(attr_name.compare(attr::SampleMaxNumber) == 0){
            set_value(info.sample_max_number,xml_attr);
        }else if(attr_name.compare(attr::SampleMaxSizeBytes) == 0){
            set_value(info.sample_max_size_bytes,xml_attr);
        }else if(attr_name.compare(attr::UID) == 0){
            set_value(info.uid,xml_attr);
        }else{
            THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::ANA_Stream << "]");
        }
    }
}

void ANAStream::create_children(const xmlNodePtr xml_node)
{
    for(auto xml_node_iter = xml_node->children ; xml_node_iter != nullptr ; xml_node_iter = xml_node_iter->next){
        if(xml_node_iter->type != XML_ELEMENT_NODE)
            continue;
        auto node_name = xmlChar2string(xml_node_iter->name);
        if(node_name.compare(node::Signals) == 0){
            for(auto xml_node_child_iter = xml_node_iter->children ; xml_node_child_iter != nullptr ; xml_node_child_iter = xml_node_child_iter->next){
                if(xml_node_child_iter->type != XML_ELEMENT_NODE)
                    continue;
                node_name = xmlChar2string(xml_node_child_iter->name);
                for(auto xml_attr = xml_node_iter->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
                    auto attr_name = xmlChar2string(xml_attr->name);
                    if(attr_name.compare(attr::SamplingPeriodUs) == 0){
                        set_value(info.info.ana.sampling_period_us,xml_attr);
                    }else{
                        THROW_PARSER_ERROR(xml_node_child_iter, "Unknown attribute [" << attr_name << "] in tag [" << node::ANA_Stream << "]");
                    }
                }
                if(node_name.compare(node::Signal) == 0){
                    std::unique_ptr<ANASignal> signal = std::make_unique<ANASignal>();
                    signal->load(xml_node_child_iter);
                    if(signal->info.info.ana.byte_offset + BaseSignal::sample_max_size_bytes(signal->info) > info.sample_max_size_bytes)
                        THROW_PARSER_ERROR(xml_node_child_iter, "Stream [" << info.name << "] Signal [" << signal->info.name << "]: ByteOffset [" << signal->info.info.ana.byte_offset << "] + [" << BaseSignal::sample_max_size_bytes(signal->info) << "] > Stream SampleMaxSizeBytes [" << info.sample_max_size_bytes << "]");
                    signals.push_back(std::move(signal));
                }else{
                    THROW_PARSER_ERROR(xml_node_child_iter, "Unknown node [" << node_name << "] in tag [" << node::Signals << "]");
                }
            }
        }else if(node_name.compare(node::DataTimestamp) == 0){
            data_timestamp.load(xml_node_iter);
        }else{
            THROW_PARSER_ERROR(xml_node_iter, "Unknown node [" << node_name << "] in tag [" << node::ANA_Stream << "]");
        }
    }
    // Sort according to ByteOffset
    std::sort(signals.begin(), signals.end(), [](std::shared_ptr<Signal> a, std::shared_ptr<Signal>b){
        return (a == nullptr || b == nullptr) ? false : (a->info.info.ana.byte_offset < b->info.info.ana.byte_offset);
    });
    // Update position attribute
    size_t p = 0;
    for(auto & s : signals){
        s->position = p++;
    }
}

// NADStream

void NADStream::reset()
{
    info = LIBED247_STREAM_INFO_DEFAULT;
    info.type = ED247_STREAM_TYPE_NAD;
    info.sample_max_size_bytes = 0;
    info.info.nad = LIBED247_STREAM_INFO_NAD_DEFAULT;
    signals.clear();
}

void NADStream::fill_attributes(const xmlNodePtr xml_node)
{
    for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
        auto attr_name = xmlChar2string(xml_attr->name);
        if(attr_name.compare(attr::Name) == 0){
            set_value(info.name,xml_attr);
        }else if(attr_name.compare(attr::Direction) == 0){
            set_value(info.direction,xml_attr);
        }else if(attr_name.compare(attr::Comment) == 0){
            set_value(info.comment,xml_attr);
        }else if(attr_name.compare(attr::ICD) == 0){
            set_value(info.icd,xml_attr);
        }else if(attr_name.compare(attr::SampleMaxNumber) == 0){
            set_value(info.sample_max_number,xml_attr);
        }else if(attr_name.compare(attr::SampleMaxSizeBytes) == 0){
            set_value(info.sample_max_size_bytes,xml_attr);
        }else if(attr_name.compare(attr::UID) == 0){
            set_value(info.uid,xml_attr);
        }else{
            THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::NAD_Stream << "]");
        }
    }
}

void NADStream::create_children(const xmlNodePtr xml_node)
{
    for(auto xml_node_iter = xml_node->children ; xml_node_iter != nullptr ; xml_node_iter = xml_node_iter->next){
        if(xml_node_iter->type != XML_ELEMENT_NODE)
            continue;
        auto node_name = xmlChar2string(xml_node_iter->name);
        if(node_name.compare(node::Signals) == 0){
            for(auto xml_node_child_iter = xml_node_iter->children ; xml_node_child_iter != nullptr ; xml_node_child_iter = xml_node_child_iter->next){
                if(xml_node_child_iter->type != XML_ELEMENT_NODE)
                    continue;
                node_name = xmlChar2string(xml_node_child_iter->name);
                for(auto xml_attr = xml_node_iter->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
                    auto attr_name = xmlChar2string(xml_attr->name);
                    if(attr_name.compare(attr::SamplingPeriodUs) == 0){
                        set_value(info.info.nad.sampling_period_us,xml_attr);
                    }else{
                        THROW_PARSER_ERROR(xml_node_child_iter, "Unknown attribute [" << attr_name << "] in tag [" << node::NAD_Stream << "]");
                    }
                }
                if(node_name.compare(node::Signal) == 0){
                    std::unique_ptr<NADSignal> signal = std::make_unique<NADSignal>();
                    signal->load(xml_node_child_iter);
                    if(signal->info.info.nad.byte_offset + BaseSignal::sample_max_size_bytes(signal->info) > info.sample_max_size_bytes)
                        THROW_PARSER_ERROR(xml_node_child_iter, "Stream [" << info.name << "] Signal [" << signal->info.name << "]: ByteOffset [" << signal->info.info.nad.byte_offset << "] + [" << BaseSignal::sample_max_size_bytes(signal->info) << "] > Stream SampleMaxSizeBytes [" << info.sample_max_size_bytes << "]");
                    signals.push_back(std::move(signal));
                }else{
                    THROW_PARSER_ERROR(xml_node_child_iter, "Unknown node [" << node_name << "] in tag [" << node::Signals << "]");
                }
            }
        }else if(node_name.compare(node::DataTimestamp) == 0){
            data_timestamp.load(xml_node_iter);
        }else{
            THROW_PARSER_ERROR(xml_node_iter, "Unknown node [" << node_name << "] in tag [" << node::NAD_Stream << "]");
        }
    }
    // Sort according to ByteOffset
    std::sort(signals.begin(), signals.end(), [](std::shared_ptr<Signal> a, std::shared_ptr<Signal>b){
        return (a == nullptr || b == nullptr) ? false : (a->info.info.nad.byte_offset < b->info.info.nad.byte_offset);
    });
    // Update position attribute
    size_t p = 0;
    for(auto & s : signals){
        s->position = p++;
    }
}

// VNADStream

void VNADStream::reset()
{
    info = LIBED247_STREAM_INFO_DEFAULT;
    info.type = ED247_STREAM_TYPE_VNAD;
    info.sample_max_size_bytes = 0;
    info.info.vnad = LIBED247_STREAM_INFO_VNAD_DEFAULT;
    signals.clear();
}

void VNADStream::fill_attributes(const xmlNodePtr xml_node)
{
    for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
        auto attr_name = xmlChar2string(xml_attr->name);
        if(attr_name.compare(attr::Name) == 0){
            set_value(info.name,xml_attr);
        }else if(attr_name.compare(attr::Direction) == 0){
            set_value(info.direction,xml_attr);
        }else if(attr_name.compare(attr::Comment) == 0){
            set_value(info.comment,xml_attr);
        }else if(attr_name.compare(attr::ICD) == 0){
            set_value(info.icd,xml_attr);
        }else if(attr_name.compare(attr::SampleMaxNumber) == 0){
            set_value(info.sample_max_number,xml_attr);
        }else if(attr_name.compare(attr::UID) == 0){
            set_value(info.uid,xml_attr);
        }else{
            THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::VNAD_Stream << "]");
        }
    }
}

void VNADStream::create_children(const xmlNodePtr xml_node)
{
    for(auto xml_node_iter = xml_node->children ; xml_node_iter != nullptr ; xml_node_iter = xml_node_iter->next){
        if(xml_node_iter->type != XML_ELEMENT_NODE)
            continue;
        auto node_name = xmlChar2string(xml_node_iter->name);
        if(node_name.compare(node::Signals) == 0){
            for(auto xml_node_child_iter = xml_node_iter->children ; xml_node_child_iter != nullptr ; xml_node_child_iter = xml_node_child_iter->next){
                if(xml_node_child_iter->type != XML_ELEMENT_NODE)
                    continue;
                node_name = xmlChar2string(xml_node_child_iter->name);
                for(auto xml_attr = xml_node_iter->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
                    auto attr_name = xmlChar2string(xml_attr->name);
                    if(attr_name.compare(attr::SamplingPeriodUs) == 0){
                        set_value(info.info.vnad.sampling_period_us,xml_attr);
                    }else{
                        THROW_PARSER_ERROR(xml_node_child_iter, "Unknown attribute [" << attr_name << "] in tag [" << node::VNAD_Stream << "]");
                    }
                }
                if(node_name.compare(node::Signal) == 0){
                    std::unique_ptr<VNADSignal> signal = std::make_unique<VNADSignal>();
                    signal->load(xml_node_child_iter);
                    info.sample_max_size_bytes += (BaseSignal::sample_max_size_bytes(signal->info) + sizeof(uint16_t)) * signal->info.info.vnad.max_length;
                    signals.push_back(std::move(signal));
                }else{
                    THROW_PARSER_ERROR(xml_node_child_iter, "Unknown node [" << node_name << "] in tag [" << node::Signals << "]");
                }
            }
            PRINT_DEBUG("VNAD Stream [" << info.name << "] SampleMaxSizeBytes[" << info.sample_max_size_bytes << "] SampleMaxNumber[" << info.sample_max_number << "]");
        }else if(node_name.compare(node::DataTimestamp) == 0){
            data_timestamp.load(xml_node_iter);
        }else{
            THROW_PARSER_ERROR(xml_node_iter, "Unknown node [" << node_name << "] in tag [" << node::VNAD_Stream << "]");
        }
    }
    // Sort according to ByteOffset
    std::sort(signals.begin(), signals.end(), [](std::shared_ptr<Signal> a, std::shared_ptr<Signal>b){
        return (a == nullptr || b == nullptr) ? false : (a->info.info.vnad.position < b->info.info.vnad.position);
    });
    // Update position attribute
    size_t p = 0;
    for(auto & s : signals){
        s->position = p++;
    }
}

// Header

void Header::reset()
{
    enable = ED247_YESNO_NO;
    transport_timestamp = ED247_YESNO_NO;
}

void Header::fill_attributes(const xmlNodePtr xml_node)
{
    for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
        auto attr_name = xmlChar2string(xml_attr->name);
        if(attr_name.compare(attr::Enable) == 0){
            set_value(enable,xml_attr);
        }else if(attr_name.compare(attr::TransportTimestamp) == 0){
            set_value(transport_timestamp,xml_attr);
        }else{
            THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::Header << "]");
        }
    }
}

void Header::create_children(const xmlNodePtr xml_node)
{
    _UNUSED(xml_node);
}

// Channel

void Channel::reset()
{
    _name = std::string();
    _comment = std::string();
    _frame_standard_revision = ED247_STANDARD__INVALID;
}

void Channel::fill_attributes(const xmlNodePtr xml_node)
{
    for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
        auto attr_name = xmlChar2string(xml_attr->name);
        if(attr_name.compare(attr::Name) == 0){
            set_value(_name, xml_attr);
        }else if(attr_name.compare(attr::Comment) == 0){
            set_value(_comment, xml_attr);
        }else{
            THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::MultiChannel << "] or [" << node::Channel << "]");
        }
    }
}

void Channel::create_children(const xmlNodePtr xml_node)
{
    for(auto xml_node_iter = xml_node->children ; xml_node_iter != nullptr ; xml_node_iter = xml_node_iter->next){
        if(xml_node_iter->type != XML_ELEMENT_NODE)
            continue;
        auto node_name = xmlChar2string(xml_node_iter->name);
        if(node_name.compare(node::ComInterface) == 0){
            com_interface.load(xml_node_iter);
        }else if(node_name.compare(node::Header) == 0){
            header.load(xml_node_iter);
        }else if(node_name.compare(node::FrameFormat) == 0){
            for(auto xml_attr = xml_node_iter->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
                auto attr_name = xmlChar2string(xml_attr->name);
                if(attr_name.compare(attr::StandardRevision) == 0){
                    set_value(_frame_standard_revision, xml_attr);
                }else{
                    THROW_PARSER_ERROR(xml_node_iter, "Unknown attribute [" << attr_name << "] in tag [" << node::FrameFormat <<"]");
                }
            }
        }else if(!simple && node_name.compare(node::Streams) == 0){
            for(auto xml_node_child_iter = xml_node_iter->children ; xml_node_child_iter != nullptr ; xml_node_child_iter = xml_node_child_iter->next){
                if(xml_node_child_iter->type != XML_ELEMENT_NODE)
                    continue;
                node_name = xmlChar2string(xml_node_child_iter->name);
                // A429
                if(node_name.compare(node::A429_Stream) == 0){
                    std::unique_ptr<A429Stream> stream = std::make_unique<A429Stream>();
                    stream->load(xml_node_child_iter);
                    streams.push_back(std::move(stream));
                // A664
                }else if(node_name.compare(node::A664_Stream) == 0){
                    std::unique_ptr<A664Stream> stream = std::make_unique<A664Stream>();
                    stream->load(xml_node_child_iter);
                    streams.push_back(std::move(stream));
                // A825
                }else if(node_name.compare(node::A825_Stream) == 0){
                    std::unique_ptr<A825Stream> stream = std::make_unique<A825Stream>();
                    stream->load(xml_node_child_iter);
                    streams.push_back(std::move(stream));
                // SERIAL
                }else if(node_name.compare(node::SERIAL_Stream) == 0){
                    std::unique_ptr<SERIALStream> stream = std::make_unique<SERIALStream>();
                    stream->load(xml_node_child_iter);
                    streams.push_back(std::move(stream));
                // DISCRETE
                }else if(node_name.compare(node::DIS_Stream) == 0){
                    std::unique_ptr<DISStream> stream = std::make_unique<DISStream>();
                    stream->load(xml_node_child_iter);
                    streams.push_back(std::move(stream));
                // ANALOG
                }else if(node_name.compare(node::ANA_Stream) == 0){
                    std::unique_ptr<ANAStream> stream = std::make_unique<ANAStream>();
                    stream->load(xml_node_child_iter);
                    streams.push_back(std::move(stream));
                // NAD
                }else if(node_name.compare(node::NAD_Stream) == 0){
                    std::unique_ptr<NADStream> stream = std::make_unique<NADStream>();
                    stream->load(xml_node_child_iter);
                    streams.push_back(std::move(stream));
                // VNAD
                }else if(node_name.compare(node::VNAD_Stream) == 0){
                    std::unique_ptr<VNADStream> stream = std::make_unique<VNADStream>();
                    stream->load(xml_node_child_iter);
                    streams.push_back(std::move(stream));
                // Otherwise
                }else{
                    THROW_PARSER_ERROR(xml_node_child_iter, "Unknown node [" << node_name << "] in tag [" << node::Streams << "]");
                }
            }
        }else if(simple && node_name.compare(node::Stream) == 0){
            for(auto xml_node_child_iter = xml_node_iter->children ; xml_node_child_iter != nullptr ; xml_node_child_iter = xml_node_child_iter->next){
                if(xml_node_child_iter->type != XML_ELEMENT_NODE)
                    continue;
                node_name = xmlChar2string(xml_node_child_iter->name);
                // A429
                if(node_name.compare(node::A429_Stream) == 0){
                    std::unique_ptr<A429Stream> stream = std::make_unique<A429Stream>();
                    stream->load(xml_node_child_iter);
                    streams.push_back(std::move(stream));
                // A664
                }else if(node_name.compare(node::A664_Stream) == 0){
                    std::unique_ptr<A664Stream> stream = std::make_unique<A664Stream>();
                    stream->load(xml_node_child_iter);
                    streams.push_back(std::move(stream));
                // A825
                }else if(node_name.compare(node::A825_Stream) == 0){
                    std::unique_ptr<A825Stream> stream = std::make_unique<A825Stream>();
                    stream->load(xml_node_child_iter);
                    streams.push_back(std::move(stream));
                // SERIAL
                }else if(node_name.compare(node::SERIAL_Stream) == 0){
                    std::unique_ptr<SERIALStream> stream = std::make_unique<SERIALStream>();
                    stream->load(xml_node_child_iter);
                    streams.push_back(std::move(stream));
                // DISCRETE
                }else if(node_name.compare(node::DIS_Stream) == 0){
                    std::unique_ptr<DISStream> stream = std::make_unique<DISStream>();
                    stream->load(xml_node_child_iter);
                    streams.push_back(std::move(stream));
                // ANALOG
                }else if(node_name.compare(node::ANA_Stream) == 0){
                    std::unique_ptr<ANAStream> stream = std::make_unique<ANAStream>();
                    stream->load(xml_node_child_iter);
                    streams.push_back(std::move(stream));
                // NAD
                }else if(node_name.compare(node::NAD_Stream) == 0){
                    std::unique_ptr<NADStream> stream = std::make_unique<NADStream>();
                    stream->load(xml_node_child_iter);
                    streams.push_back(std::move(stream));
                // VNAD
                }else if(node_name.compare(node::VNAD_Stream) == 0){
                    std::unique_ptr<VNADStream> stream = std::make_unique<VNADStream>();
                    stream->load(xml_node_child_iter);
                    streams.push_back(std::move(stream));
                // Otherwise
                }else{
                    THROW_PARSER_ERROR(xml_node_child_iter, "Unknown node [" << node_name << "] in tag [" << node::Channel << "]");
                }
            }
        }else{
            THROW_PARSER_ERROR(xml_node_iter, "Unknown node [" << node_name << "] in tag [" << node::MultiChannel << "] or [" << node::Channel << "]");
        }
    }
}

// Component

void Component::reset()
{
  _identifier = 0;
  _name = std::string();
  _version = std::string();
  _component_type = ED247_COMPONENT_TYPE_VIRTUAL;
  _standard_revision = ED247_STANDARD__INVALID;
  _comment = std::string();
  _file_producer_identifier = std::string();
  _file_producer_comment = std::string();

  channels.clear();
}

void Component::fill_attributes(const xmlNodePtr xml_node)
{
    for(auto xml_attr = xml_node->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
        auto attr_name = xmlChar2string(xml_attr->name);
        if(attr_name.compare("noNamespaceSchemaLocation") == 0 || attr_name.compare("xsi") == 0)
            continue;
        if(attr_name.compare(attr::Name) == 0){
            set_value(_name, xml_attr);
        }else if(attr_name.compare(attr::ComponentVersion) == 0){
            set_value(_version, xml_attr);
        }else if(attr_name.compare(attr::ComponentType) == 0){
            set_value(_component_type, xml_attr);
        }else if(attr_name.compare(attr::Comment) == 0){
            set_value(_comment, xml_attr);
        }else if(attr_name.compare(attr::StandardRevision) == 0){
            set_value(_standard_revision, xml_attr);
        }else if(attr_name.compare(attr::Identifier) == 0){
            set_value(_identifier, xml_attr);
        }else{
            THROW_PARSER_ERROR(xml_node, "Unknown attribute [" << attr_name << "] in tag [" << node::ED247ComponentInstanceConfiguration <<"]");
        }
    }
    if(_standard_revision != ED247_STANDARD_ED247A)
        THROW_PARSER_ERROR(xml_node, "This version do not support any other standard than [" << std::string(ed247_standard_string(ED247_STANDARD_ED247A)) << "]");
}

void Component::create_children(const xmlNodePtr xml_node)
{
    for(auto xml_node_iter = xml_node->children ; xml_node_iter != nullptr ; xml_node_iter = xml_node_iter->next){
        if(xml_node_iter->type != XML_ELEMENT_NODE)
            continue;
        auto node_name = xmlChar2string(xml_node_iter->name);
        if(node_name.compare(node::Channels) == 0){
            for(auto xml_node_channel = xml_node_iter->children ; xml_node_channel != nullptr ; xml_node_channel = xml_node_channel->next){
                if(xml_node_channel->type != XML_ELEMENT_NODE)
                    continue;
                node_name = xmlChar2string(xml_node_channel->name);
                if(node_name.compare(node::MultiChannel) == 0){
                    std::unique_ptr<Channel> channel = std::make_unique<Channel>();
                    channel->simple = false; // store if it is a simple channel (only one stream)
                    channel->load(xml_node_channel);
                    if(channel->_frame_standard_revision != ED247_STANDARD_ED247A)
                        THROW_PARSER_ERROR(xml_node_channel, "This version do not support any other standard than [" << std::string(ed247_standard_string(ED247_STANDARD_ED247A)) << "]");
                    channels.push_back(std::move(channel));
                }else if(node_name.compare(node::Channel) == 0){
                    std::unique_ptr<Channel> channel = std::make_unique<Channel>();
                    channel->simple = true; // store if it is a simple channel (only one stream)
                    channel->load(xml_node_channel);
                    if(channel->_frame_standard_revision != ED247_STANDARD_ED247A)
                        THROW_PARSER_ERROR(xml_node_channel, "This version do not support any other standard than [" << std::string(ed247_standard_string(ED247_STANDARD_ED247A)) << "]");
                    channels.push_back(std::move(channel));
                }else{
                    THROW_PARSER_ERROR(xml_node_channel, "Unknown node [" << node_name << "] in tag [" << node::Channels << "]");
                }
            }
        }else if(node_name.compare(node::FileProducer) == 0){
            for(auto xml_attr = xml_node_iter->properties ; xml_attr != nullptr ; xml_attr = xml_attr->next){
                auto attr_name = xmlChar2string(xml_attr->name);
                if(attr_name.compare(attr::Identifier) == 0){
                    set_value(_file_producer_identifier, xml_attr);
                }else if(attr_name.compare(attr::Comment) == 0){
                    set_value(_file_producer_comment, xml_attr);
                }else{
                    THROW_PARSER_ERROR(xml_node_iter, "Unknown attribute [" << attr_name << "] in tag [" << node::FileProducer <<"]");
                }
            }
        }else{
            THROW_PARSER_ERROR(xml_node_iter, "Unknown node [" << node_name << "] in tag [" << node::ED247ComponentInstanceConfiguration << "]");
        }
    }
}

// load

std::shared_ptr<Node> load_xml(xmlParserCtxtPtr & p_xml_context, xmlDocPtr & p_xml_doc)
{
    xmlDocPtr           p_xsd_doc;
    xmlSchemaParserCtxtPtr p_xsd_schema_parser;
    xmlSchemaPtr p_xsd_schema;
    xmlSchemaValidCtxtPtr p_xsd_valid_context;

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

    auto root = std::make_shared<Component>();

    root->load(xmlRootNode);

    return std::dynamic_pointer_cast<Node>(root);
}

std::shared_ptr<Node> load_filepath(const std::string & filepath)
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

    auto root = load_xml(p_xml_context, p_xml_doc);

    if(p_xml_doc)
        xmlFreeDoc(p_xml_doc);
    if(p_xml_context)
        xmlFreeParserCtxt(p_xml_context);

    return root;
}

std::shared_ptr<Node> load_content(const std::string & content)
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

    auto root = load_xml(p_xml_context, p_xml_doc);

    if(p_xml_doc)
        xmlFreeDoc(p_xml_doc);
    if(p_xml_context)
        xmlFreeParserCtxt(p_xml_context);

    return root;
}

}

}
