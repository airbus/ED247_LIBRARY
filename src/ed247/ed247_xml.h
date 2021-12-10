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

#ifndef _ED247_XML_H_
#define _ED247_XML_H_

#include "ed247_internals.h"

#include <libxml/tree.h>

#include <vector>

#define DECL_STREAM_OPERATOR_SIGNATURE(x,strx,xstr)             \
    std::ostream & operator << (std::ostream & os, const x & e);\
    std::istream & operator >> (std::istream & is, x & e);      \

DECL_STREAM_OPERATOR_SIGNATURE(
    ed247_component_type_t,
    ed247_component_type_from_string,
    ed247_component_type_string
)

DECL_STREAM_OPERATOR_SIGNATURE(
    ed247_standard_t,
    ed247_standard_from_string,
    ed247_standard_string
)

DECL_STREAM_OPERATOR_SIGNATURE(
    ed247_yesno_t,
    ed247_yesno_from_string,
    ed247_yesno_string
)

DECL_STREAM_OPERATOR_SIGNATURE(
    ed247_direction_t,
    ed247_direction_from_string,
    ed247_direction_string
)

DECL_STREAM_OPERATOR_SIGNATURE(
    ed247_nad_type_t,
    ed247_nad_type_from_string,
    ed247_nad_type_string
)

namespace ed247
{

namespace xml
{

namespace node
{
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

namespace attr
{
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
    static const std::string MaxLength { "MaxLength" };
    static const std::string Unit { "Unit" };
    static const std::string SampleDataTimestampOffset { "SampleDataTimestampOffset" };
    static const std::string ElectricalUnit { "ElectricalUnit" };
    static const std::string Dimensions { "Dimensions" };
}

std::string xmlChar2string(const xmlChar *str);

size_t nad_type_size(ed247_nad_type_t type);

template<typename T>
void set_value(T & variable, const xmlAttrPtr attribute);

class Exception : std::exception
{
public:
    inline Exception(std::string message) :
        _message(message)
    {}
    virtual ~Exception() {}

    virtual const char *what() const noexcept;

private:
    std::string _message;
    mutable std::string _what;
};

class Node
{
    public:
        void load(const xmlNodePtr xml_node);
        virtual void reset() {};
    
    protected:
        virtual void fill_attributes(const xmlNodePtr xml_node) {_UNUSED(xml_node);};
        virtual void create_children(const xmlNodePtr xml_node) {_UNUSED(xml_node);};
};

class DataTimestamp : public Node
{
    private:
        virtual void fill_attributes(const xmlNodePtr xml_node) final;
        virtual void create_children(const xmlNodePtr xml_node) final;
        
    public:
        virtual void reset() final;
    
    public:
        ed247_yesno_t enable;
        ed247_yesno_t enable_sample_offset;
};

class Errors : public Node
{
    private:
        virtual void fill_attributes(const xmlNodePtr xml_node) final;
        virtual void create_children(const xmlNodePtr xml_node) final;
        
    public:
        virtual void reset() final;
    
    public:
        ed247_yesno_t enable;
};

class UdpSocket : public Node
{
    public:
        explicit UdpSocket(std::string new_dst_ip_address = "",
            uint16_t new_dst_ip_port = 0,
            std::string new_src_ip_address = "",
            uint16_t new_src_ip_port = 0,
            std::string new_mc_ip_address = "",
            uint16_t new_mc_ttl = 1,
            ed247_direction_t new_direction = ED247_DIRECTION_INOUT):
            dst_ip_address(new_dst_ip_address),
            dst_ip_port(new_dst_ip_port),
            src_ip_address(new_src_ip_address),
            src_ip_port(new_src_ip_port),
            mc_ip_address(new_mc_ip_address),
            mc_ttl(new_mc_ttl),
            direction(new_direction){}
        virtual ~UdpSocket() {}

        std::string toString() const;
    private:
        virtual void fill_attributes(const xmlNodePtr xml_node) final;
        virtual void create_children(const xmlNodePtr xml_node) final;
        
    public:
        virtual void reset() final;

    public:
        std::string dst_ip_address;
        uint16_t dst_ip_port;
        std::string src_ip_address;
        uint16_t src_ip_port;
        std::string mc_ip_address;
        uint16_t mc_ttl;
        ed247_direction_t direction;
};

class ComInterface : public Node
{
    private:
        virtual void fill_attributes(const xmlNodePtr xml_node) final;
        virtual void create_children(const xmlNodePtr xml_node) final;
        
    public:
        virtual void reset() final;
    
    public:
        std::vector<std::shared_ptr<UdpSocket>> udp_sockets;
};

struct Stream
{
    ed247_stream_info_t info;
    DataTimestamp data_timestamp;
};

class A429Stream : public Node, public Stream
{
    private:
        virtual void fill_attributes(const xmlNodePtr xml_node) final;
        virtual void create_children(const xmlNodePtr xml_node) final;

    public:
        virtual void reset() final;
        Errors errors;
};

class A664Stream : public Node, public Stream
{
    private:
        virtual void fill_attributes(const xmlNodePtr xml_node) final;
        virtual void create_children(const xmlNodePtr xml_node) final;

    public:
        virtual void reset() final;
        Errors errors;
        ed247_yesno_t enable_message_size;
};

class A825Stream : public Node, public Stream
{
    private:
        virtual void fill_attributes(const xmlNodePtr xml_node) final;
        virtual void create_children(const xmlNodePtr xml_node) final;

    public:
        virtual void reset() final;
        Errors errors;
};

class SERIALStream : public Node, public Stream
{
    private:
        virtual void fill_attributes(const xmlNodePtr xml_node) final;
        virtual void create_children(const xmlNodePtr xml_node) final;

    public:
        virtual void reset() final;
        Errors errors;
};

struct Signal
{
    ed247_signal_info_t info;
    size_t position;
};

class DISSignal : public Node, public Signal
{
    private:
        virtual void fill_attributes(const xmlNodePtr xml_node) final;
        virtual void create_children(const xmlNodePtr xml_node) final;
    
    public:
        virtual void reset() final;
};

class ANASignal : public Node, public Signal
{
    private:
        virtual void fill_attributes(const xmlNodePtr xml_node) final;
        virtual void create_children(const xmlNodePtr xml_node) final;
    
    public:
        virtual void reset() final;
};

class NADSignal : public Node, public Signal
{
    private:
        virtual void fill_attributes(const xmlNodePtr xml_node) final;
        virtual void create_children(const xmlNodePtr xml_node) final;
    public:
        virtual ~NADSignal();
        virtual void reset() final;
};

class VNADSignal : public Node, public Signal
{
    private:
        virtual void fill_attributes(const xmlNodePtr xml_node) final;
        virtual void create_children(const xmlNodePtr xml_node) final;

    public:
        virtual void reset() final;
};

struct StreamSignals : public Stream
{
    std::vector<std::shared_ptr<Signal>> signals;
};

class DISStream : public Node, public StreamSignals
{
    private:
        virtual void fill_attributes(const xmlNodePtr xml_node) final;
        virtual void create_children(const xmlNodePtr xml_node) final;
        
    public:
        virtual void reset() final;
};

class ANAStream : public Node, public StreamSignals
{
    private:
        virtual void fill_attributes(const xmlNodePtr xml_node) final;
        virtual void create_children(const xmlNodePtr xml_node) final;
        
    public:
        virtual void reset() final;
};

class NADStream : public Node, public StreamSignals
{
    private:
        virtual void fill_attributes(const xmlNodePtr xml_node) final;
        virtual void create_children(const xmlNodePtr xml_node) final;
        
    public:
        virtual void reset() final;
};

class VNADStream : public Node, public StreamSignals
{
    private:
        virtual void fill_attributes(const xmlNodePtr xml_node) final;
        virtual void create_children(const xmlNodePtr xml_node) final;
        
    public:
        virtual void reset() final;
};

class Header: public Node
{
    private:
        virtual void fill_attributes(const xmlNodePtr xml_node) final;
        virtual void create_children(const xmlNodePtr xml_node) final;
        
    public:
        virtual void reset() final;

    public:
        ed247_yesno_t enable;
        ed247_yesno_t transport_timestamp;

        bool operator == (const Header & other) const
        {
            return enable == other.enable &&
                transport_timestamp == other.transport_timestamp;
        }

        bool operator != (const Header & other) const
        {
            return !operator==(other);
        }
};

class Channel: public Node, public std::enable_shared_from_this<Channel>
{
    private:
        virtual void fill_attributes(const xmlNodePtr xml_node) final;
        virtual void create_children(const xmlNodePtr xml_node) final;
        
    public:
        virtual void reset() final;

    public:
        ed247_channel_info_t info;
        ComInterface com_interface;
        Header header;
        std::vector<std::shared_ptr<Stream>> streams;
        bool simple;
};

class Root: public Node
{    
    private:
        virtual void fill_attributes(const xmlNodePtr xml_node) final;
        virtual void create_children(const xmlNodePtr xml_node) final;
        
    public:
        virtual void reset() final;

    public:
        ed247_component_info_t info;
        std::vector<std::shared_ptr<Channel>> channels;
        
};

std::shared_ptr<Node> load_filepath(const std::string & filepath);
std::shared_ptr<Node> load_content(const std::string & content);

}

}

#endif