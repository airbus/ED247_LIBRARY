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

// Set ED247_FRIEND_TEST to have access to class private members
#define TEST_CLASS_NAME(test_case_name, test_name) test_case_name##_##test_name##_Test
class TEST_CLASS_NAME(SignalContext, SinglePushPop);
#define ED247_FRIEND_TEST() friend TEST_CLASS_NAME(SignalContext, SinglePushPop)

#include "unitary_test.h"
#include "ed247_context.h"
#include "ed247_stream_assistant.h"
#include "ed247_bswap.h"

class SignalContext : public ::testing::TestWithParam<std::string> {};

void swap_copy(const char *source_data, char* dest_data, const ed247_nad_type_t& nad_type)
{
  switch(nad_type) {
  case ED247_NAD_TYPE_INT8:
  case ED247_NAD_TYPE_UINT8:
    memcpy(dest_data, source_data, 1);
    break;
  case ED247_NAD_TYPE_INT16:
    *((uint16_t*)dest_data) = bswap_16(*((uint16_t*)source_data));
    break;
  case ED247_NAD_TYPE_INT32:
    *((uint32_t*)dest_data) = bswap_32(*((uint32_t*)source_data));
    break;
  case ED247_NAD_TYPE_INT64:
    *((uint64_t*)dest_data) = bswap_64(*((uint64_t*)source_data));
    break;
  case ED247_NAD_TYPE_UINT16:
    *((uint16_t*)dest_data) = bswap_16(*((uint16_t*)source_data));
    break;
  case ED247_NAD_TYPE_UINT32:
    *((uint32_t*)dest_data) = bswap_32(*((uint32_t*)source_data));
    break;
  case ED247_NAD_TYPE_UINT64:
    *((uint64_t*)dest_data) = bswap_64(*((uint64_t*)source_data));
    break;
  case ED247_NAD_TYPE_FLOAT32:
    *((uint32_t*)dest_data) = bswap_32(*((uint32_t*)source_data));
    break;
  case ED247_NAD_TYPE_FLOAT64:
    *((uint64_t*)dest_data) = bswap_64(*((uint64_t*)source_data));
    break;
  default:
    THROW_ED247_ERROR("Unexpected NAD type: " << nad_type);
  }
}

void swap_payload(const char *source_data, char* dest_data, uint32_t payload_size, const ed247_nad_type_t& nad_type)
{
  uint32_t pos = 0;
  while (pos <= payload_size) {
    swap_copy(source_data + pos, dest_data + pos, nad_type);
    pos += ed247::xml::Signal::get_nad_type_size(nad_type);
  }

}


TEST_P(SignalContext, SinglePushPop)
{
  std::string filepath = GetParam();

  ed247::Context* context = ed247::Context::create_from_filepath(filepath);

  // Retrieve the set of signals
  auto signal_set = context->get_signal_set();
  if(std::string(GetParam()).find("_nad.xml") != std::string::npos)
    ASSERT_EQ(signal_set._signals.size(), (uint32_t)25);
  else
    ASSERT_EQ(signal_set._signals.size(), (uint32_t)12);

  // Check finder for find all
  auto signals = signal_set.find(".*");
  if(std::string(GetParam()).find("_nad.xml") != std::string::npos)
    ASSERT_EQ(signals.size(), (uint32_t)25);
  else
    ASSERT_EQ(signals.size(), (uint32_t)12);

  // Check stream finder
  auto stream_set = context->get_stream_set();
  auto stream = stream_set.find("Stream1").front();
  ASSERT_EQ(stream->find_signals(".*").size(), (uint32_t)2);

  // Check signal sample allocation
  auto signal = signal_set.find(".*").front();
  std::unique_ptr<ed247::Sample> signal_sample(new ed247::Sample(signal->get_sample_max_size_bytes()));
  ASSERT_EQ(signal_sample->size(), (uint32_t)0);
  ASSERT_EQ(signal_sample->capacity(), signal->get_sample_max_size_bytes());

  // Check BaseStream::Assistant creation
  ed247_stream_assistant_t api_assistant = stream->get_api_assistant();
  ASSERT_NE(api_assistant, nullptr);
  ed247::StreamAssistant* assistant = static_cast<ed247::StreamAssistant*>(api_assistant);

  // Check write & push
  std::vector<std::unique_ptr<ed247::Sample>> samples;
  ed247::StreamSample stream_sample(stream->get_sample_max_size_bytes());
  for(auto & signal : stream->get_signals()){
    std::unique_ptr<ed247::Sample> sample(new ed247::Sample(signal->get_sample_max_size_bytes()));
    ASSERT_EQ(sample->size(), (uint32_t)0);
    ASSERT_EQ(sample->capacity(), signal->get_sample_max_size_bytes());
    std::string msg = strize() << std::setw(sample->capacity()) << std::setfill('0') << 1;
    sample->copy(msg.c_str(), sample->capacity());
    assistant->write(*signal, sample->data(), sample->size());
    if(stream->get_type() == ED247_STREAM_TYPE_VNAD){
      *(uint16_t*)(stream_sample.data_rw()+(uint8_t)stream_sample.size()) = (uint16_t)htons((uint16_t)sample->size());
      stream_sample.set_size(stream_sample.size()+sizeof(uint16_t));
    }
    memcpy(stream_sample.data_rw()+stream_sample.size(), msg.c_str(), sample->size());
    stream_sample.set_size(stream_sample.size()+sample->size());
    samples.push_back(std::move(sample));
  }
  ASSERT_EQ(stream_sample.size(), stream_sample.capacity());
  assistant->push(nullptr, nullptr);
  ASSERT_EQ(stream->get_outgoing_sample_number(), (uint32_t)1);
  ASSERT_EQ(stream_sample.size(), assistant->_buffer.size());

  swap_payload(stream_sample.data(), stream_sample.data_rw(), stream_sample.size(), signal->get_nad_type());
  ASSERT_EQ(memcmp(stream_sample.data(), assistant->_buffer.data(), stream_sample.size()), 0);

  // Check pop & read
  stream = stream_set.find("StreamInput").front();
  ASSERT_NE(stream, nullptr);
  stream->_recv_stack.push_back().copy(stream_sample.data(), stream_sample.size());

  api_assistant = stream->get_api_assistant();
  ASSERT_NE(assistant, nullptr);
  assistant = static_cast<ed247::StreamAssistant*>(api_assistant);
  assistant->pop(nullptr, nullptr, nullptr, nullptr);
  for(auto & signal : stream->get_signals()){
    std::unique_ptr<ed247::Sample> sample(new ed247::Sample(signal->get_sample_max_size_bytes()));
    const void *data;
    uint32_t size;
    assistant->read(*signal, &data, &size);
    ASSERT_EQ(size, sample->capacity());
    std::string msg = strize() << std::setw(sample->capacity()) << std::setfill('0') << 1;
    SAY(size);
    SAY("data: " << hex_stream(data, size));
    SAY("msg: " << hex_stream(msg.c_str(), size));
    ASSERT_EQ(memcmp(data, msg.c_str(), size), 0);
  }

  delete context;
}

std::vector<std::string> configuration_files;

INSTANTIATE_TEST_CASE_P(SignalTests, SignalContext,
                        ::testing::ValuesIn(configuration_files));


/*************
 * Functions *
 *************/

int main(int argc, char **argv)
{
  std::string config_path = "../config";
  if(argc >=1) config_path = argv[1];
  SAY("Configuration path: " << config_path);

  configuration_files.push_back(config_path+"/ecic_unit_signals_dis.xml");
  configuration_files.push_back(config_path+"/ecic_unit_signals_ana.xml");
  configuration_files.push_back(config_path+"/ecic_unit_signals_nad.xml");
  configuration_files.push_back(config_path+"/ecic_unit_signals_vnad.xml");

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
