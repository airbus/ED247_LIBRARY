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
#ifndef _ED247_XML_H_
#define _ED247_XML_H_
#include "ed247.h"
#include <string>
#include <vector>
#include <memory>

// Prevent include of libxml2 header
typedef struct _xmlNode *xmlNodePtr;

namespace ed247 {
  namespace xml {
    struct Component;

    // Load ECIC and return an ed247::xml::Component node
    std::unique_ptr<Component> load_filepath(const std::string & filepath);
    std::unique_ptr<Component> load_content(const std::string & content);


    struct Node
    {
      virtual void load(const xmlNodePtr xml_node) = 0;
    };

    struct UdpSocket : public Node
    {
      std::string       _dst_ip_address;
      uint16_t          _dst_ip_port;
      std::string       _src_ip_address;
      uint16_t          _src_ip_port;
      std::string       _mc_ip_address;
      uint16_t          _mc_ttl;
      ed247_direction_t _direction;

      UdpSocket();
      virtual void load(const xmlNodePtr xml_node) override final;
    };

    struct ComInterface : public Node
    {
      std::vector<UdpSocket> _udp_sockets;
      virtual void load(const xmlNodePtr xml_node) override final;
    };

    struct DataTimestamp : public Node
    {
      ed247_yesno_t _enable;
      ed247_yesno_t _enable_sample_offset;

      DataTimestamp();
      virtual void load(const xmlNodePtr xml_node) override final;
    };

    struct Errors : public Node
    {
      ed247_yesno_t _enable;

      Errors();
      virtual void load(const xmlNodePtr xml_node) override final;
    };

    struct Header: public Node
    {
      ed247_yesno_t _enable;
      ed247_yesno_t _transport_timestamp;

      Header();
      virtual void load(const xmlNodePtr xml_node) override final;
    };

    //
    // Signals
    //
    struct Signal : public Node
    {
      std::string           _name;
      ed247_signal_type_t   _type;
      std::string           _comment;
      std::string           _icd;
      uint32_t              _byte_offset;                // All signals except VNAD
      std::string           _analogue_electrical_unit;
      ed247_nad_type_t      _nad_type;                   // UINT8 for discrete and FLOAT32 for analogue
      std::string           _nad_unit;                   // NAD and VNAD
      std::vector<uint32_t> _nad_dimensions;
      uint32_t              _vnad_position;
      uint32_t              _vnad_max_number;

      uint32_t get_sample_max_size_bytes() const;
      inline uint32_t get_nad_type_size() const { return get_nad_type_size(_nad_type); }
      static uint32_t get_nad_type_size(ed247_nad_type_t nad_type);

      Signal(ed247_signal_type_t type);
    };

    struct DISSignal : public Signal
    {
      DISSignal();
      virtual void load(const xmlNodePtr xml_node) override final;
    };

    struct ANASignal : public Signal
    {
      ANASignal();
      virtual void load(const xmlNodePtr xml_node) override final;
    };

    struct NADSignal : public Signal
    {
      NADSignal();
      virtual void load(const xmlNodePtr xml_node) override final;
    };

    struct VNADSignal : public Signal
    {
      VNADSignal();
      virtual void load(const xmlNodePtr xml_node) override final;
    };

    //
    // Streams
    //
    struct Stream : public Node
    {
      std::string         _name;
      ed247_direction_t   _direction;
      ed247_stream_type_t _type;
      std::string         _comment;
      std::string         _icd;
      ed247_uid_t         _uid;
      uint32_t            _sample_max_number;
      uint32_t            _sample_max_size_bytes;
      bool                _sample_size_fixed;
      DataTimestamp       _data_timestamp;

      virtual bool is_signal_based() const = 0;
      virtual void validate(const xmlNodePtr closest_node) = 0;
      Stream(ed247_stream_type_t type, uint32_t sample_max_size_bytes, bool sample_size_fixed);
    };

    struct StreamProtocoled : public Stream
    {
      Errors _errors;
      virtual bool is_signal_based() const override final { return false; }
      StreamProtocoled(ed247_stream_type_t type, uint32_t sample_max_size_bytes, bool sample_size_fixed = false);
    };

    struct StreamSignals : public Stream
    {
      std::vector<std::unique_ptr<Signal>> _signal_list;
      uint32_t                             _sampling_period_us;
      virtual bool is_signal_based() const override final { return true; }
      StreamSignals(ed247_stream_type_t type, uint32_t sample_max_size_bytes, bool sample_size_fixed = true);
    };

    struct A429Stream : public StreamProtocoled
    {
      A429Stream();
      virtual void load(const xmlNodePtr xml_node) override final;
      virtual void validate(const xmlNodePtr closest_node) override final;
    };

    struct A664Stream : public StreamProtocoled
    {
      ed247_yesno_t _enable_message_size;

      A664Stream();
      virtual void load(const xmlNodePtr xml_node) override final;
      virtual void validate(const xmlNodePtr closest_node) override final;
    };

    struct A825Stream : public StreamProtocoled
    {
      A825Stream();
      virtual void load(const xmlNodePtr xml_node) override final;
      virtual void validate(const xmlNodePtr closest_node) override final;
    };

    struct SERIALStream : public StreamProtocoled
    {
      SERIALStream();
      virtual void load(const xmlNodePtr xml_node) override final;
      virtual void validate(const xmlNodePtr closest_node) override final;
    };

    struct DISStream : public StreamSignals
    {
      DISStream();
      virtual void load(const xmlNodePtr xml_node) override final;
      virtual void validate(const xmlNodePtr closest_node) override final;
    };

    struct ANAStream : public StreamSignals
    {
      ANAStream();
      virtual void load(const xmlNodePtr xml_node) override final;
      virtual void validate(const xmlNodePtr closest_node) override final;
    };

    struct NADStream : public StreamSignals
    {
      NADStream();
      virtual void load(const xmlNodePtr xml_node) override final;
      virtual void validate(const xmlNodePtr closest_node) override final;
    };

    struct VNADStream : public StreamSignals
    {
      VNADStream();
      virtual void load(const xmlNodePtr xml_node) override final;
      virtual void validate(const xmlNodePtr closest_node) override final;
    };

    //
    // Channels
    //
    struct Channel : public Node
    {
      std::string                           _name;
      std::string                           _comment;
      ed247_standard_t                      _frame_standard_revision;
      ComInterface                          _com_interface;
      Header                                _header;
      std::vector<std::unique_ptr<Stream>>  _stream_list;
      bool                                  _simple;

      Channel();
      virtual void load(const xmlNodePtr xml_node) override final;
    };

    //
    // Component (root)
    //
    struct Component: public Node
    {
      ed247_uid_t            _identifier;
      std::string            _name;
      std::string            _version;
      ed247_component_type_t _component_type;
      ed247_standard_t       _standard_revision;
      std::string            _comment;
      std::string            _file_producer_identifier;
      std::string            _file_producer_comment;
      std::vector<Channel>   _channel_list;

      Component();
      virtual void load(const xmlNodePtr xml_node) override final;
    };
  }
}

std::ostream& operator<<(std::ostream& stream, const ed247::xml::UdpSocket& socket);

#endif
