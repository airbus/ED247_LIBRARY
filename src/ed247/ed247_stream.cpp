
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

#include "ed247_stream.h"
#include "ed247_channel.h"
#include "ed247_signal.h"
#include "ed247_context.h"

#include <regex>

namespace ed247
{

// StreamSample

void StreamSample::update_info(const FrameHeader & header)
{
    if(header._recv_headers_iter == header._recv_headers.end()){
        _info.component_identifier = 0;
        _info.sequence_number = 0;
        _info.transport_timestamp.epoch_s = 0;
        _info.transport_timestamp.offset_ns = 0;
    }else{
        _info.component_identifier = header._recv_headers_iter->component_identifier;
        _info.sequence_number = header._recv_headers_iter->sequence_number;
        _info.transport_timestamp = header._recv_headers_iter->transport_timestamp;
    }
}

// StreamBuilder<>
template<ed247_stream_type_t T, ed247_stream_type_t ... E>
std::shared_ptr<BaseStream> StreamBuilder<T, E...>::create(const ed247_stream_type_t & type, std::shared_ptr<xml::Stream> & configuration, std::shared_ptr<BaseSignal::Pool> & pool_signals)
{
    if(type == T){
        static typename Stream<T>::Builder builder;
        return builder.create(configuration, pool_signals);
    }else{
        return StreamBuilder<E...>::create(type, configuration, pool_signals);
    }
}

template<ed247_stream_type_t T>
std::shared_ptr<BaseStream> StreamBuilder<T>::create(const ed247_stream_type_t & type, std::shared_ptr<xml::Stream> & configuration, std::shared_ptr<BaseSignal::Pool> & pool_signals)
{
    if(type == T){
        static typename Stream<T>::Builder builder;
        return builder.create(configuration, pool_signals);
    }else{
        THROW_ED247_ERROR("Failed to create stream '" << configuration->info.name << "'");
    }
}

// BaseStream
bool BaseStream::push_sample(const void * sample_data, size_t sample_size, const ed247_timestamp_t * data_timestamp, bool * full)
{
  if(!(_configuration->info.direction & ED247_DIRECTION_OUT)) {
    PRINT_ERROR("Stream '" << get_name() << "': Cannot write sample on a stream which is not an output one");
    return false;
  }
  if(sample_size > _configuration->info.sample_max_size_bytes) {
    PRINT_ERROR("Stream '" << get_name() << "': Invalid sample size (" << sample_size << ")");
    return false;
  }
  auto && sample = _send_stack.next_write();
  sample->copy(sample_data, sample_size);
  if(data_timestamp) sample->set_data_timestamp(*data_timestamp);
  bool tmp_full = _send_stack.increment();
  if(full) *full = tmp_full;
  return true;
}

std::shared_ptr<StreamSample> BaseStream::pop_sample(bool *empty)
{
  if(!(_configuration->info.direction & ED247_DIRECTION_IN)) {
    PRINT_ERROR("Stream '" << get_name() << "': Cannot read sample on a stream which is not an input one");
    return nullptr;
  }
  return _recv_stack.pop_front(empty);
}

std::vector<std::shared_ptr<BaseSignal>> BaseStream::find_signals(std::string str_regex)
{
    std::regex reg(str_regex);
    std::vector<std::shared_ptr<BaseSignal>> founds;
    for(auto signal : *_signals){
        if(std::regex_match(signal->get_name(), reg)){
            founds.push_back(signal);
        }
    }
    return founds;
}

std::shared_ptr<BaseSignal> BaseStream::get_signal(std::string str_name)
{
    for(auto signal : *_signals){
        if(signal->get_name() == str_name) return signal;
    }
    return nullptr;
}

void BaseStream::register_channel(Channel & channel, ed247_direction_t direction)
{
    auto sp_channel = channel.shared_from_this();
    _channel = sp_channel;
    channel.add_stream(*this, direction);
}

// BaseStream::Pool

BaseStream::Pool::Pool():
    _streams(std::make_shared<SmartListStreams>())
{
    _streams->set_managed(true);
}

BaseStream::Pool::Pool(std::shared_ptr<BaseSignal::Pool> & pool_signals):
    _streams(std::make_shared<SmartListStreams>()),
    _pool_signals(pool_signals)
{
    _streams->set_managed(true);
}

std::shared_ptr<BaseStream> BaseStream::Pool::get(std::shared_ptr<xml::Stream> & configuration)
{
    std::shared_ptr<BaseStream> sp_base_stream;
    std::string name{configuration->info.name};

    auto iter = std::find_if(_streams->begin(), _streams->end(),
        [&name](const std::shared_ptr<BaseStream> & s){ return s->get_name() == name; });
    if(iter == _streams->end()){
        auto sp_stream = _builder.create(configuration->info.type,configuration, _pool_signals);
        sp_base_stream = std::static_pointer_cast<BaseStream>(sp_stream);
        _streams->push_back(sp_base_stream);
    }else{
        THROW_ED247_ERROR("Stream [" << name << "] already exists");
    }

    return sp_base_stream;
}

std::vector<std::shared_ptr<BaseStream>> BaseStream::Pool::find(std::string strregex)
{
    std::regex reg(strregex);
    std::vector<std::shared_ptr<BaseStream>> founds;
    for(auto stream : *_streams){
        if(std::regex_match(stream->get_name(), reg)){
            founds.push_back(stream);
        }
    }
    return founds;
}

std::shared_ptr<BaseStream> BaseStream::Pool::get(std::string str_name)
{
    for(auto stream : *_streams){
        if(stream->get_name() == str_name) return stream;
    }
    return nullptr;
}

std::shared_ptr<SmartListStreams> BaseStream::Pool::streams()
{
    return _streams;
}

size_t BaseStream::Pool::size() const
{
    return _streams->size();
}

// BaseStream::Builder

void BaseStream::Builder::build(std::shared_ptr<Pool> & pool, std::shared_ptr<xml::Stream> & configuration, Channel & channel) const
{
    auto sp_stream = pool->get(configuration);
    sp_stream->register_channel(channel, configuration->info.direction);
}

template<ed247_stream_type_t E>
void Stream<E>::allocate_stacks()
{
    PRINT_DEBUG("Allocate internal RECV buffer of type [" << ed247_stream_type_string(E) << "] with SampleMaxSizeBytes[" << _configuration->info.sample_max_size_bytes << "] SampleMaxNumber[" << _configuration->info.sample_max_number << "]");
    _recv_stack.allocate(_configuration->info.sample_max_size_bytes, _configuration->info.sample_max_number);
    _recv_working_sample = std::make_shared<StreamSample>();
    _recv_working_sample->allocate(_configuration->info.sample_max_size_bytes);
    PRINT_DEBUG("Allocate internal SEND buffer of type [" << ed247_stream_type_string(E) << "] with SampleMaxSizeBytes[" << _configuration->info.sample_max_size_bytes << "] SampleMaxNumber[" << _configuration->info.sample_max_number << "]");
    _send_stack.allocate(_configuration->info.sample_max_size_bytes, _configuration->info.sample_max_number);
    _send_working_sample = std::make_shared<StreamSample>();
    _send_working_sample->allocate(_configuration->info.sample_max_size_bytes);
    if(_configuration->info.direction <= ED247_DIRECTION__INVALID ||
        _configuration->info.direction > ED247_DIRECTION_INOUT)
        THROW_ED247_ERROR("Stream '" << _configuration->info.name << "' direction is neigher input nor ouput nor bidirectional");
}

template<ed247_stream_type_t E>
void Stream<E>::allocate_working_sample()
{
    _working_sample.allocate(_configuration->info.sample_max_size_bytes);
}

template<ed247_stream_type_t E>
template<ed247_stream_type_t T>
typename std::enable_if<!StreamSignalTypeChecker<T>::value,std::unique_ptr<StreamSample>>::type
Stream<E>::allocate_sample_impl() const
{
    auto sample = std::make_unique<StreamSample>();
    sample->allocate(_configuration->info.sample_max_size_bytes);
    return sample;
}

template<ed247_stream_type_t E>
template<ed247_stream_type_t T>
typename std::enable_if<StreamSignalTypeChecker<T>::value,std::unique_ptr<StreamSample>>::type
Stream<E>::allocate_sample_impl() const
{
    auto sample = std::make_unique<StreamSample>();
    sample->allocate(_configuration->info.sample_max_size_bytes);
    return sample;
}

// Stream<A429>

template<>
size_t Stream<ED247_STREAM_TYPE_A429>::encode(char * frame, size_t frame_size)
{
    if(_send_stack.size() == 0) return 0;
    size_t frame_index = 0;
    bool empty;
    do{
        const auto & sample = _send_stack.pop_front(&empty);

        // Write Data Timestamp and Precise Data Timestamp
        encode_data_timestamp(sample, frame, frame_size, frame_index);

        // Write sample data
        if((frame_index + sample->size()) > frame_size) {
            THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
        }
        memcpy(frame + frame_index, sample->data(), sample->size());
        frame_index += sample->size();
    }while(!empty);
    return frame_index;
}

template<>
bool Stream<ED247_STREAM_TYPE_A429>::decode(const char * frame, size_t frame_size, const FrameHeader * header)
{
    size_t frame_index = 0;
    static ed247_timestamp_t data_timestamp;
    static ed247_timestamp_t timestamp;
    while(frame_index < frame_size){
        // Read Data Timestamp if necessary
        if (decode_data_timestamp(frame, frame_size, frame_index, data_timestamp, timestamp) == false) return false;
        // Read sample data
        if((frame_size-frame_index) < _configuration->info.sample_max_size_bytes) {
          PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
          return false;
        }
        auto && sample = _recv_stack.next_write();
        sample->copy(frame+frame_index, _configuration->info.sample_max_size_bytes);
        frame_index += sample->size();
        // Update data timestamp
        if(_configuration->data_timestamp.enable == ED247_YESNO_YES){
            sample->set_data_timestamp(data_timestamp);
        }
        // Update simulation time
        sample->update_timestamp();
        // Attach header
        if(header)
            sample->update_info(*header);
        _recv_stack.increment();
    }
    // Callbacks
    return run_callbacks();
}

template<>
ed247_status_t Stream<ED247_STREAM_TYPE_A429>::check_sample_size(size_t sample_size) const
{
    return sample_size == _configuration->info.sample_max_size_bytes ? ED247_STATUS_SUCCESS : ED247_STATUS_FAILURE;
}

template<>
void Stream<ED247_STREAM_TYPE_A429>::allocate_buffer()
{
    auto size = _configuration->info.sample_max_number * _configuration->info.sample_max_size_bytes;
    if(_configuration->data_timestamp.enable == ED247_YESNO_YES){
        size += sizeof(ed247_timestamp_t);
        size += (_configuration->info.sample_max_number > 1) ? (sizeof(uint32_t) * (_configuration->info.sample_max_number - 1)) : 0;
    }
    _buffer.allocate(size);
}

// Stream<A664>

template<>
size_t Stream<ED247_STREAM_TYPE_A664>::encode(char * frame, size_t frame_size)
{
    auto enable_message_size = std::static_pointer_cast<xml::A664Stream>(_configuration)->enable_message_size == ED247_YESNO_YES;
    size_t frame_index = 0;
    while (_send_stack.size() > 0) {
        const auto & sample = _send_stack.pop_front();

        // Write Data Timestamp and Precise Data Timestamp
        encode_data_timestamp(sample, frame, frame_size, frame_index);

        // Write sample size
        if(enable_message_size){
            if((frame_index + sizeof(uint16_t)) > frame_size) {
                THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
            }
            if(sample->size() != (uint16_t)sample->size())
                THROW_ED247_ERROR("Stream '" << get_name() << "': Stream data size it too big for a 16 bits number ! (" << sample->size() << ")");
            uint16_t size = htons((uint16_t)sample->size());
            memcpy(frame + frame_index, &size, sizeof(uint16_t));
            frame_index += sizeof(uint16_t);
        }

        // Write sample data
        if((frame_index + sizeof(uint16_t)) > frame_size) {
            THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
        }
        memcpy(frame + frame_index, sample->data(), sample->size());
        frame_index += sample->size();

        // If message size is disabled, we cannot append more sample.
        // Remaining ones will be available for next encode() call.
        if (enable_message_size == false) break;
    };
    return frame_index;
}

template<>
bool Stream<ED247_STREAM_TYPE_A664>::decode(const char * frame, size_t frame_size, const FrameHeader * header)
{
    auto enable_message_size = std::static_pointer_cast<xml::A664Stream>(_configuration)->enable_message_size == ED247_YESNO_YES;
    size_t frame_index = 0;
    size_t sample_size = 0;
    static ed247_timestamp_t data_timestamp;
    static ed247_timestamp_t timestamp;
    while(frame_index < frame_size){
        // Read Data Timestamp if necessary
        if (decode_data_timestamp(frame, frame_size, frame_index, data_timestamp, timestamp) == false) return false;
        // Sample size
        if(enable_message_size){
            // Read sample size
            if((frame_size-frame_index) < sizeof(uint16_t)) {
              PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
              return false;
            }
            sample_size = ntohs(*(uint16_t*)(frame+frame_index));
            frame_index += sizeof(uint16_t);
        }else{
            // Assume remaining data is the message
            sample_size = (frame_size-frame_index);
        }
        // Read sample data
        if((frame_size-frame_index) < sample_size) {
          PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
          return false;
        }

        PRINT_CRAZY("AFDX stream '" << get_name() << "' decoded. Size: " << sample_size);
        auto & sample = _recv_stack.next_write();
        if (sample->copy(frame+frame_index, sample_size) == false) {
          PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
          return false;
        }
        frame_index += sample->size();
        // Update data timestamp
        if(_configuration->data_timestamp.enable == ED247_YESNO_YES){
            sample->set_data_timestamp(data_timestamp);
        }
        // Update simulation time
        sample->update_timestamp();
        // Attach header
        if(header)
            sample->update_info(*header);
        _recv_stack.increment();
    }
    // Callbacks
    return run_callbacks();
}

template<>
ed247_status_t Stream<ED247_STREAM_TYPE_A664>::check_sample_size(size_t sample_size) const
{
    return sample_size == _configuration->info.sample_max_size_bytes ? ED247_STATUS_SUCCESS : ED247_STATUS_FAILURE;
}

template<>
void Stream<ED247_STREAM_TYPE_A664>::allocate_buffer()
{
    auto enable_message_size = std::static_pointer_cast<xml::A664Stream>(_configuration)->enable_message_size == ED247_YESNO_YES;
    auto size = _configuration->info.sample_max_number * (_configuration->info.sample_max_size_bytes + (enable_message_size ? sizeof(uint16_t) : 0));
    if(_configuration->data_timestamp.enable == ED247_YESNO_YES){
        // Data Timestamp
        size += sizeof(ed247_timestamp_t);
        // Data Timestamp Offsets
        size += (_configuration->info.sample_max_number > 1) ? (sizeof(uint32_t) * (_configuration->info.sample_max_number - 1)) : 0;
    }
    _buffer.allocate(size);
}

// Stream<A825>

template<>
size_t Stream<ED247_STREAM_TYPE_A825>::encode(char * frame, size_t frame_size)
{
    if(_send_stack.size() == 0) return 0;
    size_t frame_index = 0;
    bool empty;
    do{
        const auto & sample = _send_stack.pop_front(&empty);

        // Write Data Timestamp and Precise Data Timestamp
        encode_data_timestamp(sample, frame, frame_size, frame_index);

        // Write sample size
        if((frame_index + sizeof(uint8_t)) > frame_size) {
            THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
        }
        if(sample->size() != (uint8_t)sample->size())
            THROW_ED247_ERROR("Stream '" << get_name() << "': Stream data size it too big for a 8 bits number ! (" << sample->size() << ")");
        uint8_t size = (uint8_t)sample->size();
        memcpy(frame + frame_index, &size, sizeof(uint8_t));
        frame_index += sizeof(uint8_t);

        // Write sample data
       if((frame_index + sizeof(uint16_t)) > frame_size) {
         THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
        }
        memcpy(frame + frame_index, sample->data(), sample->size());
        frame_index += sample->size();
    }while(!empty);
    return frame_index;
}

template<>
bool Stream<ED247_STREAM_TYPE_A825>::decode(const char * frame, size_t frame_size, const FrameHeader * header)
{
    size_t frame_index = 0;
    size_t sample_size = 0;
    static ed247_timestamp_t data_timestamp;
    static ed247_timestamp_t timestamp;
    while(frame_index < frame_size){
        // Read Data Timestamp if necessary
        if (decode_data_timestamp(frame, frame_size, frame_index, data_timestamp, timestamp) == false) return false;
        // Read sample size
        if((frame_size-frame_index) < sizeof(uint8_t)) {
          PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
          return false;
        }
        sample_size = *(uint8_t*)(frame+frame_index);
        frame_index += sizeof(uint8_t);
        // Read sample data
        if((frame_size-frame_index) < sample_size) {
          PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
          return false;
        }
        auto & sample = _recv_stack.next_write();
        if (sample->copy(frame+frame_index, sample_size) == false) {
          PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
          return false;
        }
        frame_index += sample->size();
        // Update data timestamp
        if(_configuration->data_timestamp.enable == ED247_YESNO_YES){
            sample->set_data_timestamp(data_timestamp);
        }
        // Update simulation time
        sample->update_timestamp();
        // Attach header
        if(header)
            sample->update_info(*header);
        _recv_stack.increment();
    }
    // Callbacks
    return run_callbacks();
}

template<>
ed247_status_t Stream<ED247_STREAM_TYPE_A825>::check_sample_size(size_t sample_size) const
{
    return sample_size == _configuration->info.sample_max_size_bytes ? ED247_STATUS_SUCCESS : ED247_STATUS_FAILURE;
}

template<>
void Stream<ED247_STREAM_TYPE_A825>::allocate_buffer()
{
    auto size = _configuration->info.sample_max_number * (_configuration->info.sample_max_size_bytes + sizeof(uint8_t));
    if(_configuration->data_timestamp.enable == ED247_YESNO_YES){
        // Data Timestamp
        size += sizeof(ed247_timestamp_t);
        // Data Timestamp Offsets
        size += (_configuration->info.sample_max_number > 1) ? (sizeof(uint32_t) * (_configuration->info.sample_max_number - 1)) : 0;
    }
    _buffer.allocate(size);
}

// Stream<SERIAL>

template<>
size_t Stream<ED247_STREAM_TYPE_SERIAL>::encode(char * frame, size_t frame_size)
{
    if(_send_stack.size() == 0) return 0;
    size_t frame_index = 0;
    bool empty;
    do{
        const auto & sample = _send_stack.pop_front(&empty);

        // Write Data Timestamp and Precise Data Timestamp
        encode_data_timestamp(sample, frame, frame_size, frame_index);

        // Write sample size
        if((frame_index + sizeof(uint16_t)) > frame_size) {
          THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
        }
        if(sample->size() != (uint16_t)sample->size())
            THROW_ED247_ERROR("Stream '" << get_name() << "': Stream data size it too big for a 16 bits number ! (" << sample->size() << ")");
        uint16_t size = (uint16_t)sample->size();
        memcpy(frame + frame_index, &size, sizeof(uint16_t));
        frame_index += sizeof(uint16_t);

        // Write sample data
        if((frame_index + sizeof(uint16_t)) > frame_size) {
          THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
        }
        memcpy(frame + frame_index, sample->data(), sample->size());
        frame_index += sample->size();
    }while(!empty);
    return frame_index;
}

template<>
bool Stream<ED247_STREAM_TYPE_SERIAL>::decode(const char * frame, size_t frame_size, const FrameHeader * header)
{
    size_t frame_index = 0;
    size_t sample_size = 0;
    static ed247_timestamp_t data_timestamp;
    static ed247_timestamp_t timestamp;
    while(frame_index < frame_size){
        // Read Data Timestamp if necessary
        if (decode_data_timestamp(frame, frame_size, frame_index, data_timestamp, timestamp) == false) return false;
        // Read sample size
        if((frame_size-frame_index) < sizeof(uint16_t)) {
          PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
          return false;
        }
        sample_size = *(uint16_t*)(frame+frame_index);
        frame_index += sizeof(uint16_t);
        // Read sample data
        if((frame_size-frame_index) < sample_size) {
          PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
          return false;
        }
        auto & sample = _recv_stack.next_write();
        if (sample->copy(frame+frame_index, sample_size) == false) {
          PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
          return false;
        }
        frame_index += sample->size();
        // Update data timestamp
        if(_configuration->data_timestamp.enable == ED247_YESNO_YES){
            sample->set_data_timestamp(data_timestamp);
        }
        // Update simulation time
        sample->update_timestamp();
        // Attach header
        if(header)
            sample->update_info(*header);
        _recv_stack.increment();
    }
    // Callbacks
    return run_callbacks();
}

template<>
ed247_status_t Stream<ED247_STREAM_TYPE_SERIAL>::check_sample_size(size_t sample_size) const
{
    return sample_size == _configuration->info.sample_max_size_bytes ? ED247_STATUS_SUCCESS : ED247_STATUS_FAILURE;
}

template<>
void Stream<ED247_STREAM_TYPE_SERIAL>::allocate_buffer()
{
    auto size = _configuration->info.sample_max_number * (_configuration->info.sample_max_size_bytes + sizeof(uint16_t));
    if(_configuration->data_timestamp.enable == ED247_YESNO_YES){
        // Data Timestamp
        size += sizeof(ed247_timestamp_t);
        // Data Timestamp Offsets
        size += (_configuration->info.sample_max_number > 1) ? (sizeof(uint32_t) * (_configuration->info.sample_max_number - 1)) : 0;
    }
    _buffer.allocate(size);
}

// Stream<AUDIO>

template<>
size_t Stream<ED247_STREAM_TYPE_AUDIO>::encode(char * frame, size_t frame_size)
{
    if(_send_stack.size() == 0) return 0;
    size_t frame_index = 0;
    bool empty;
    do{
        const auto & sample = _send_stack.pop_front(&empty);

        // Write Data Timestamp and Precise Data Timestamp
        encode_data_timestamp(sample, frame, frame_size, frame_index);

        // Write sample size
        if((frame_index + sizeof(uint8_t)) > frame_size) {
            THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
        }
        if(sample->size() != (uint8_t)sample->size())
            THROW_ED247_ERROR("Stream '" << get_name() << "': Stream data size it too big for a 8 bits number ! (" << sample->size() << ")");
        uint8_t size = (uint8_t)sample->size();
        memcpy(frame + frame_index, &size, sizeof(uint8_t));
        frame_index += sizeof(uint8_t);

        // Write sample data
        if((frame_index + sizeof(uint16_t)) > frame_size) {
            THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
        }
        memcpy(frame + frame_index, sample->data(), sample->size());
        frame_index += sample->size();
    }while(!empty);
    return frame_index;
}

template<>
bool Stream<ED247_STREAM_TYPE_AUDIO>::decode(const char * frame, size_t frame_size, const FrameHeader * header)
{
    size_t frame_index = 0;
    size_t sample_size = 0;
    static ed247_timestamp_t data_timestamp;
    static ed247_timestamp_t timestamp;
    while(frame_index < frame_size){
        // Read Data Timestamp if necessary
        if (decode_data_timestamp(frame, frame_size, frame_index, data_timestamp, timestamp) == false) return false;
        // Read sample size
        if((frame_size-frame_index) < sizeof(uint8_t)) {
          PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
          return false;
        }
        sample_size = *(uint8_t*)(frame+frame_index);
        frame_index += sizeof(uint8_t);
        // Read sample data
        if((frame_size-frame_index) < sample_size) {
          PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
          return false;
        }
        auto & sample = _recv_stack.next_write();
        if (sample->copy(frame+frame_index, sample_size) == false) {
          PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
          return false;
        }
        frame_index += sample->size();
        // Update data timestamp
        if(_configuration->data_timestamp.enable == ED247_YESNO_YES){
            sample->set_data_timestamp(data_timestamp);
        }
        // Update simulation time
        sample->update_timestamp();
        // Attach header
        if(header)
            sample->update_info(*header);
        _recv_stack.increment();
    }
    // Callbacks
    return run_callbacks();
}

template<>
ed247_status_t Stream<ED247_STREAM_TYPE_AUDIO>::check_sample_size(size_t sample_size) const
{
    return sample_size == _configuration->info.sample_max_size_bytes ? ED247_STATUS_SUCCESS : ED247_STATUS_FAILURE;
}

template<>
void Stream<ED247_STREAM_TYPE_AUDIO>::allocate_buffer()
{
    auto size = _configuration->info.sample_max_number * (_configuration->info.sample_max_size_bytes + sizeof(uint8_t));
    if(_configuration->data_timestamp.enable == ED247_YESNO_YES){
        // Data Timestamp
        size += sizeof(ed247_timestamp_t);
        // Data Timestamp Offsets
        size += (_configuration->info.sample_max_number > 1) ? (sizeof(uint32_t) * (_configuration->info.sample_max_number - 1)) : 0;
    }
    _buffer.allocate(size);
}

// Stream<DISCRETE>

template<>
size_t Stream<ED247_STREAM_TYPE_DISCRETE>::encode(char * frame, size_t frame_size)
{
    if(_send_stack.size() == 0) return 0;
    size_t frame_index = 0;
    bool empty;
    do{
        const auto & sample = _send_stack.pop_front(&empty);

        // Write Data Timestamp and Precise Data Timestamp
        encode_data_timestamp(sample, frame, frame_size, frame_index);

        // Write sample data
        memcpy(frame + frame_index, sample->data(), sample->size());
        frame_index += sample->size();
    }while(!empty);
    return frame_index;
}

template<>
bool Stream<ED247_STREAM_TYPE_DISCRETE>::decode(const char * frame, size_t frame_size, const FrameHeader * header)
{
    size_t frame_index = 0;
    static ed247_timestamp_t data_timestamp;
    static ed247_timestamp_t timestamp;
    while(frame_index < frame_size){
        // Read Data Timestamp if necessary
        if (decode_data_timestamp(frame, frame_size, frame_index, data_timestamp, timestamp) == false) return false;
        // Read sample data
        if((frame_size-frame_index) < _configuration->info.sample_max_size_bytes) {
          PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
          return false;
        }
        auto && sample = _recv_stack.next_write();
        sample->copy(frame+frame_index, _configuration->info.sample_max_size_bytes);
        frame_index += sample->size();
        // Update data timestamp
        if(_configuration->data_timestamp.enable == ED247_YESNO_YES){
            sample->set_data_timestamp(data_timestamp);
        }
        // Update simulation time
        sample->update_timestamp();
        // Attach header
        if(header)
            sample->update_info(*header);
        _recv_stack.increment();
    }
    // Callbacks
    return run_callbacks();
}

template<>
ed247_status_t Stream<ED247_STREAM_TYPE_DISCRETE>::check_sample_size(size_t sample_size) const
{
    return sample_size <= _configuration->info.sample_max_size_bytes ? ED247_STATUS_SUCCESS : ED247_STATUS_FAILURE;
}

template<>
void Stream<ED247_STREAM_TYPE_DISCRETE>::allocate_buffer()
{
    auto size = _configuration->info.sample_max_number * _configuration->info.sample_max_size_bytes;
    if(_configuration->data_timestamp.enable == ED247_YESNO_YES){
        size += sizeof(ed247_timestamp_t);
        size += (_configuration->info.sample_max_number > 1) ? (sizeof(uint32_t) * (_configuration->info.sample_max_number - 1)) : 0;
    }
    _buffer.allocate(size);
}

// Stream<ANALOG>

template<>
size_t Stream<ED247_STREAM_TYPE_ANALOG>::encode(char * frame, size_t frame_size)
{
    if(_send_stack.size() == 0) return 0;
    size_t frame_index = 0;
    bool empty;
    do{
        const auto & sample = _send_stack.pop_front(&empty);

        // Write Data Timestamp and Precise Data Timestamp
        encode_data_timestamp(sample, frame, frame_size, frame_index);

        PRINT_CRAZY("SWAP stream [" << _configuration->info.name << "] ...");
        // SWAP
        for(auto signal : *_signals){
            *(uint32_t*)(sample->data_rw()+signal->get_configuration()->info.info.ana.byte_offset) = bswap_32(*(uint32_t*)(sample->data_rw()+signal->get_configuration()->info.info.ana.byte_offset));
        }
        PRINT_CRAZY("SWAP stream [" << _configuration->info.name << "] ... OK");

        // Write sample data
        memcpy(frame + frame_index, sample->data(), sample->size());
        frame_index += sample->size();
    }while(!empty);
    return frame_index;
}

template<>
bool Stream<ED247_STREAM_TYPE_ANALOG>::decode(const char * frame, size_t frame_size, const FrameHeader * header)
{
    size_t frame_index = 0;
    static ed247_timestamp_t data_timestamp;
    static ed247_timestamp_t timestamp;
    while(frame_index < frame_size){
        // Read Data Timestamp if necessary
        if (decode_data_timestamp(frame, frame_size, frame_index, data_timestamp, timestamp) == false) return false;
        // Read sample data
        if((frame_size-frame_index) < _configuration->info.sample_max_size_bytes) {
          PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
          return false;
        }
        auto & sample = _recv_stack.next_write();
        sample->copy(frame+frame_index, _configuration->info.sample_max_size_bytes);
        frame_index += sample->size();

        PRINT_CRAZY("SWAP stream [" << _configuration->info.name << "] ...");
        // SWAP
        for(auto signal : *_signals){
            *(uint32_t*)(sample->data_rw()+signal->get_configuration()->info.info.ana.byte_offset) = bswap_32(*(uint32_t*)(sample->data_rw()+signal->get_configuration()->info.info.ana.byte_offset));
        }
        PRINT_CRAZY("SWAP stream [" << _configuration->info.name << "] ... OK");

        // Update data timestamp
        if(_configuration->data_timestamp.enable == ED247_YESNO_YES){
            sample->set_data_timestamp(data_timestamp);
        }
        // Update simulation time
        sample->update_timestamp();
        // Attach header
        if(header)
            sample->update_info(*header);
        _recv_stack.increment();
    }
    // Callbacks
    return run_callbacks();
}

template<>
ed247_status_t Stream<ED247_STREAM_TYPE_ANALOG>::check_sample_size(size_t sample_size) const
{
    return sample_size <= _configuration->info.sample_max_size_bytes ? ED247_STATUS_SUCCESS : ED247_STATUS_FAILURE;
}

template<>
void Stream<ED247_STREAM_TYPE_ANALOG>::allocate_buffer()
{
    auto size = _configuration->info.sample_max_number * _configuration->info.sample_max_size_bytes;
    if(_configuration->data_timestamp.enable == ED247_YESNO_YES){
        size += sizeof(ed247_timestamp_t);
        size += (_configuration->info.sample_max_number > 1) ? (sizeof(uint32_t) * (_configuration->info.sample_max_number - 1)) : 0;
    }
    _buffer.allocate(size);
}

// Stream<NAD>

void swap_nad(void *sample_data, const ed247_nad_type_t & nad_type, const size_t & sample_element_length)
{
    // SWAP
        switch((uint8_t)nad_type){
            case ED247_NAD_TYPE_INT16:
                {
                    for(size_t i = 0 ; i < sample_element_length ; i++){
                        *((uint16_t*)sample_data+i) = bswap_16(*((uint16_t*)sample_data+i));
                    }
                } break;
            case ED247_NAD_TYPE_INT32:
                {
                    for(size_t i = 0 ; i < sample_element_length ; i++){
                        *((uint32_t*)sample_data+i) = bswap_32(*((uint32_t*)sample_data+i));
                    }
                } break;
            case ED247_NAD_TYPE_INT64:
                {
                    for(size_t i = 0 ; i < sample_element_length ; i++){
                        *((uint64_t*)sample_data+i) = bswap_64(*((uint64_t*)sample_data+i));
                    }
                } break;
            case ED247_NAD_TYPE_UINT16:
                {
                    for(size_t i = 0 ; i < sample_element_length ; i++){
                        *((uint16_t*)sample_data+i) = bswap_16(*((uint16_t*)sample_data+i));
                    }
                } break;
            case ED247_NAD_TYPE_UINT32:
                {
                    for(size_t i = 0 ; i < sample_element_length ; i++){
                        *((uint32_t*)sample_data+i) = bswap_32(*((uint32_t*)sample_data+i));
                    }
                } break;
            case ED247_NAD_TYPE_UINT64:
                {
                    for(size_t i = 0 ; i < sample_element_length ; i++){
                        *((uint64_t*)sample_data+i) = bswap_64(*((uint64_t*)sample_data+i));
                    }
                } break;
            case ED247_NAD_TYPE_FLOAT32:
                {
                    for(size_t i = 0 ; i < sample_element_length ; i++){
                        *((uint32_t*)sample_data+i) = bswap_32(*((uint32_t*)sample_data+i));
                    }
                } break;
            case ED247_NAD_TYPE_FLOAT64:
                {
                    for(size_t i = 0 ; i < sample_element_length ; i++){
                        *((uint64_t*)sample_data+i) = bswap_64(*((uint64_t*)sample_data+i));
                    }
                } break;
            default:
                break;
        }
}

template<>
size_t Stream<ED247_STREAM_TYPE_NAD>::encode(char * frame, size_t frame_size)
{
    if(_send_stack.size() == 0) return 0;
    size_t frame_index = 0;
    bool empty;
    do{
        const auto & sample = _send_stack.pop_front(&empty);

        // Write Data Timestamp and Precise Data Timestamp
        encode_data_timestamp(sample, frame, frame_size, frame_index);

        PRINT_CRAZY("SWAP stream [" << _configuration->info.name << "] ...");
        // SWAP
        for(auto signal : *_signals){
            void *sample_data = (void*)(sample->data_rw()+signal->get_configuration()->info.info.nad.byte_offset);
            size_t sample_element_length = BaseSignal::sample_max_size_bytes(signal->get_configuration()->info) / xml::nad_type_size(signal->get_configuration()->info.info.nad.nad_type);
            swap_nad(sample_data, signal->get_configuration()->info.info.nad.nad_type, sample_element_length);
        }
        PRINT_CRAZY("SWAP stream [" << _configuration->info.name << "] ... OK");

        // Write sample data
        memcpy(frame + frame_index, sample->data(), sample->size());
        frame_index += sample->size();
    }while(!empty);
    return frame_index;
}

template<>
bool Stream<ED247_STREAM_TYPE_NAD>::decode(const char * frame, size_t frame_size, const FrameHeader * header)
{
    size_t frame_index = 0;
    static ed247_timestamp_t data_timestamp;
    static ed247_timestamp_t timestamp;
    while(frame_index < frame_size){
        // Read Data Timestamp if necessary
        if (decode_data_timestamp(frame, frame_size, frame_index, data_timestamp, timestamp) == false) return false;
        // Read sample data
        if((frame_size-frame_index) < _configuration->info.sample_max_size_bytes) {
          PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
          return false;
        }
        auto & sample = _recv_stack.next_write();
        sample->copy(frame+frame_index, _configuration->info.sample_max_size_bytes);
        frame_index += sample->size();

        PRINT_CRAZY("SWAP stream [" << _configuration->info.name << "] ...");
        // SWAP
        for(auto signal : *_signals){
            void *sample_data = (void*)(sample->data_rw()+signal->get_configuration()->info.info.nad.byte_offset);
            size_t sample_element_length = BaseSignal::sample_max_size_bytes(signal->get_configuration()->info) / xml::nad_type_size(signal->get_configuration()->info.info.nad.nad_type);
            swap_nad(sample_data, signal->get_configuration()->info.info.nad.nad_type, sample_element_length);
        }
        PRINT_CRAZY("SWAP stream [" << _configuration->info.name << "] ... OK");

        // Update data timestamp
        if(_configuration->data_timestamp.enable == ED247_YESNO_YES){
            sample->set_data_timestamp(data_timestamp);
        }
        // Update simulation time
        sample->update_timestamp();
        // Attach header
        if(header)
            sample->update_info(*header);
        _recv_stack.increment();
    }
    // Callbacks
    return run_callbacks();
}

template<>
ed247_status_t Stream<ED247_STREAM_TYPE_NAD>::check_sample_size(size_t sample_size) const
{
    return sample_size <= _configuration->info.sample_max_size_bytes ? ED247_STATUS_SUCCESS : ED247_STATUS_FAILURE;
}

template<>
void Stream<ED247_STREAM_TYPE_NAD>::allocate_buffer()
{
    auto size = _configuration->info.sample_max_number * _configuration->info.sample_max_size_bytes;
    if(_configuration->data_timestamp.enable == ED247_YESNO_YES){
        size += sizeof(ed247_timestamp_t);
        size += (_configuration->info.sample_max_number > 1) ? (sizeof(uint32_t) * (_configuration->info.sample_max_number - 1)) : 0;
    }
    _buffer.allocate(size);
}

// Stream<VNAD>

template<>
size_t Stream<ED247_STREAM_TYPE_VNAD>::encode(char * frame, size_t frame_size)
{
    if(_send_stack.size() == 0) return 0;
    size_t frame_index = 0;
    bool empty;
    do{
        const auto & sample = _send_stack.pop_front(&empty);

        // Write Data Timestamp and Precise Data Timestamp
        encode_data_timestamp(sample, frame, frame_size, frame_index);

        PRINT_CRAZY("SWAP stream [" << _configuration->info.name << "] ...");
        // SWAP
        size_t cursor = 0;
        size_t cursor_step = 0;
        uint32_t isignal = 0;
        uint16_t sample_size_bytes = 0;
        while(cursor < sample->size()){
            sample_size_bytes = ntohs(*(uint16_t*)(sample->data()+cursor));
            cursor += sizeof(uint16_t);
            cursor_step = xml::nad_type_size((*_signals)[isignal]->get_configuration()->info.info.vnad.nad_type);
            swap_nad((void*)(sample->data_rw()+cursor), (*_signals)[isignal]->get_configuration()->info.info.vnad.nad_type, sample_size_bytes/cursor_step);
            cursor += sample_size_bytes;
            isignal++;
        }
        PRINT_CRAZY("SWAP stream [" << _configuration->info.name << "] ... OK");

        // Write sample size
        if((frame_index + sizeof(uint16_t) + sample->size()) > frame_size) {
            THROW_ED247_ERROR("Stream '" << get_name() << "': Stream buffer is too small to encode a new frame. Size: " << frame_size);
        }
        if(sample->size() != (uint16_t)sample->size())
            THROW_ED247_ERROR("Stream '" << get_name() << "': Stream data size it too big for a 16 bits number ! (" << sample->size() << ")");
        uint16_t size = htons((uint16_t)sample->size());
        memcpy(frame + frame_index, &size, sizeof(uint16_t));
        frame_index += sizeof(uint16_t);
        // Write sample data
        memcpy(frame + frame_index, sample->data(), sample->size());
        frame_index += sample->size();
    }while(!empty);
    return frame_index;
}

template<>
bool Stream<ED247_STREAM_TYPE_VNAD>::decode(const char * frame, size_t frame_size, const FrameHeader * header)
{
    size_t frame_index = 0;
    size_t sample_size = 0;
    static ed247_timestamp_t data_timestamp;
    static ed247_timestamp_t timestamp;
    while(frame_index < frame_size){
        // Read Data Timestamp if necessary
        if (decode_data_timestamp(frame, frame_size, frame_index, data_timestamp, timestamp) == false) return false;
        // Read sample size
        if((frame_size-frame_index) < sizeof(uint16_t)) {
          PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
          return false;
        }
        sample_size = ntohs(*(uint16_t*)(frame+frame_index));
        frame_index += sizeof(uint16_t);
        // Read sample data
        if((frame_size-frame_index) < sample_size) {
          PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
          return false;
        }
        auto & sample = _recv_stack.next_write();
        if (sample->copy(frame+frame_index, sample_size) == false) {
          PRINT_ERROR("Stream '" << get_name() << "': Received frame size is invalid: " << frame_size);
          return false;
        }
        frame_index += sample->size();

        PRINT_CRAZY("SWAP stream [" << _configuration->info.name << "] ...");
        // SWAP
        size_t cursor = 0;
        size_t cursor_step = 0;
        uint32_t isignal = 0;
        uint16_t sample_size_bytes = 0;
        while(cursor < sample->size()){
            sample_size_bytes = ntohs(*(uint16_t*)(sample->data()+cursor));
            cursor += sizeof(uint16_t);
            cursor_step = xml::nad_type_size((*_signals)[isignal]->get_configuration()->info.info.vnad.nad_type);
            swap_nad((void*)(sample->data_rw()+cursor), (*_signals)[isignal]->get_configuration()->info.info.vnad.nad_type, sample_size_bytes/cursor_step);
            cursor += sample_size_bytes;
            isignal++;
        }
        PRINT_CRAZY("SWAP stream [" << _configuration->info.name << "] ... OK");

        // Update data timestamp
        if(_configuration->data_timestamp.enable == ED247_YESNO_YES){
            sample->set_data_timestamp(data_timestamp);
        }
        // Update simulation time
        sample->update_timestamp();
        // Attach header
        if(header)
            sample->update_info(*header);
        _recv_stack.increment();
    }
    // Callbacks
    return run_callbacks();
}

template<>
ed247_status_t Stream<ED247_STREAM_TYPE_VNAD>::check_sample_size(size_t sample_size) const
{
    return sample_size <= _configuration->info.sample_max_size_bytes ? ED247_STATUS_SUCCESS : ED247_STATUS_FAILURE;
}

template<>
void Stream<ED247_STREAM_TYPE_VNAD>::allocate_buffer()
{
    auto size = _configuration->info.sample_max_number * (_configuration->info.sample_max_size_bytes+sizeof(uint32_t));
    if(_configuration->data_timestamp.enable == ED247_YESNO_YES){
        size += sizeof(ed247_timestamp_t);
        size += (_configuration->info.sample_max_number > 1) ? (sizeof(uint32_t) * (_configuration->info.sample_max_number - 1)) : 0;
    }
    _buffer.allocate(size);
}

// Stream<>::Builder

template<ed247_stream_type_t E>
template<ed247_stream_type_t T>
typename std::enable_if<!StreamSignalTypeChecker<T>::value, std::shared_ptr<Stream<E>>>::type
Stream<E>::Builder::create(std::shared_ptr<xml::Stream> & configuration,
    std::shared_ptr<BaseSignal::Pool> & pool_signals) const
{
    _UNUSED(pool_signals);
    PRINT_DEBUG("Create stream [" << configuration->info.name << "] ...");
    auto sp_stream = std::make_shared<Stream<E>>(configuration);
    sp_stream->_assistant = nullptr;
    sp_stream->allocate_stacks();
    sp_stream->allocate_buffer();
    sp_stream->allocate_working_sample();
    PRINT_DEBUG("Create stream [" << configuration->info.name << "] OK");
    return sp_stream;
}

template<ed247_stream_type_t E>
template<ed247_stream_type_t T>
typename std::enable_if<StreamSignalTypeChecker<T>::value, std::shared_ptr<Stream<E>>>::type
Stream<E>::Builder::create(std::shared_ptr<xml::Stream> & configuration,
    std::shared_ptr<BaseSignal::Pool> & pool_signals) const
{
    static BaseSignal::Builder builder;

    PRINT_DEBUG("Create signal based stream [" << configuration->info.name << "] ...");
    auto sp_stream = std::make_shared<Stream<E>>(configuration);
    auto sconfiguration = std::static_pointer_cast<xml::StreamSignals>(configuration);
    if(!sconfiguration)
        THROW_ED247_ERROR("Stream '" << configuration->info.name << "': Wrong stream type, not a signal based one");
    for(auto signal_configuration : sconfiguration->signals){
        sp_stream->_signals->push_back(builder.build(pool_signals, signal_configuration, *sp_stream));
    }
    sp_stream->_signals->reset();
    sp_stream->_assistant = std::make_shared<Assistant>(sp_stream);
    sp_stream->allocate_stacks();
    sp_stream->allocate_buffer();
    sp_stream->allocate_working_sample();
    PRINT_DEBUG("Create stream [" << configuration->info.name << "] OK");
    return sp_stream;
}

}
