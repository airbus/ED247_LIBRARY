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

/************
 * Includes *
 ************/

#include <ed247_test.h>
#include <ed247_logs.h>

#include <ed247_context.h>
#include <ed247_channel.h>
#include <ed247_cominterface.h>

#include <memory>

/***********
 * Defines *
 ***********/

using namespace ed247;

class SignalContext : public ::testing::TestWithParam<std::string> {};

TEST_P(SignalContext, SinglePushPop)
{
    std::string filepath{CONFIG_PATH"/ut_signals/"};
    filepath += GetParam();

    Context * context = Context::Builder::create(filepath, libed247_configuration_t(LIBED247_CONFIGURATION_DEFAULT));
    Context::Builder::initialize(*context);

    // Retrieve the pool of signals
    auto pool_signals = context->getPoolSignals();
    if(GetParam() == "nad.xml")
        ASSERT_EQ(pool_signals->size(), (size_t)25);
    else
        ASSERT_EQ(pool_signals->size(), (size_t)12);

    // Check finder for find all
    auto signals = pool_signals->find(".*");
    if(GetParam() == "nad.xml")
        ASSERT_EQ(signals.size(), (size_t)25);
    else
        ASSERT_EQ(signals.size(), (size_t)12);

    // Check stream finder
    auto pool_streams = context->getPoolStreams();
    auto stream = pool_streams->find("Stream").front();
    ASSERT_EQ(stream->find_signals(".*").size(), (size_t)2);

    // Check signal sample allocation
    auto signal = pool_signals->find(".*").front();
    auto signal_sample = signal->allocate_sample();
    ASSERT_EQ(signal_sample->size(), (size_t)0);
    ASSERT_EQ(signal_sample->capacity(), BaseSignal::sample_max_size_bytes(signal->get_configuration()->info));

    // Check BaseStream::Assistant creation
    auto assistant = stream->get_assistant();
    ASSERT_NE(assistant, nullptr);

    // Check write & encode
    std::vector<std::unique_ptr<BaseSample>> samples;
    auto stream_sample = stream->allocate_sample();
    for(auto & signal : *assistant->get_stream()->signals()){
        auto sample = signal->allocate_sample();
        ASSERT_EQ(sample->size(), (size_t)0);
        ASSERT_EQ(sample->capacity(), BaseSignal::sample_max_size_bytes(signal->get_configuration()->info));
        std::ostringstream oss;
        oss.str("");
        oss << std::setw(sample->capacity()) << std::setfill('0') << 1;
        std::string msg = oss.str();
        sample->copy(msg.c_str(), sample->capacity());
        assistant->write(signal, sample->data(), sample->size());
        if(stream->get_configuration()->info.type == ED247_STREAM_TYPE_VNAD){
            *(uint16_t*)((char*)stream_sample->data()+(uint8_t)stream_sample->size()) = (uint16_t)htons((uint16_t)sample->size());
            stream_sample->set_size(stream_sample->size()+sizeof(uint16_t));
        }
        memcpy((char*)stream_sample->data()+stream_sample->size(), msg.c_str(), sample->size());
        stream_sample->set_size(stream_sample->size()+sample->size());
        samples.push_back(std::move(sample));
    }
    if(stream->get_configuration()->info.type == ED247_STREAM_TYPE_VNAD){   
        ASSERT_EQ(stream_sample->size(), stream_sample->capacity()/stream->get_configuration()->info.sample_max_number);
        assistant->encode();
        ASSERT_EQ(stream_sample->size(), assistant->buffer().size());
        ASSERT_EQ(memcmp(stream_sample->data(), assistant->buffer().data_const(), assistant->buffer().size()), 0);
    }else{ 
        ASSERT_EQ(stream_sample->size(), stream_sample->capacity());
        assistant->encode();
        ASSERT_EQ(stream_sample->size(), assistant->buffer().size());
        ASSERT_EQ(memcmp(stream_sample->data(), assistant->buffer().data_const(), stream_sample->size()), 0);
    }

    // Check push
    for(auto & signal : *assistant->get_stream()->signals()){
        auto sample = signal->allocate_sample();
        ASSERT_EQ(sample->size(), (size_t)0);
        ASSERT_EQ(sample->capacity(), BaseSignal::sample_max_size_bytes(signal->get_configuration()->info));
        std::ostringstream oss;
        oss.str("");
        oss << std::setw(sample->capacity()) << std::setfill('0') << 1;
        std::string msg = oss.str();
        sample->copy(msg.c_str(), sample->capacity());
        assistant->write(signal, sample->data(), sample->size());
        samples.push_back(std::move(sample));
    }
    assistant->push();
    ASSERT_EQ(assistant->get_stream()->send_stack().size(), (size_t)1);

    // Check decode & read
    stream = pool_streams->find("StreamInput").front();
    ASSERT_NE(stream, nullptr);
    assistant = stream->get_assistant();
    ASSERT_NE(assistant, nullptr);
    assistant->decode(stream_sample->data(), stream_sample->size());
    for(auto & signal : *assistant->get_stream()->signals()){
        auto sample = signal->allocate_sample();
        const void *data;
        size_t size;
        assistant->read(signal, &data, &size);
        ASSERT_EQ(size, sample->capacity());
        std::ostringstream oss;
        oss.str("");
        oss << std::setw(sample->capacity()) << std::setfill('0') << 1;
        std::string msg = oss.str();
        ASSERT_EQ(memcmp(data, msg.c_str(), size), 0);
    }

    // Check pop (need a send / recv)
}

std::vector<std::string> configuration_files = {
    std::string("dis.xml"),
    std::string("ana.xml"),
    std::string("nad.xml"),
    std::string("vnad.xml")
};

INSTANTIATE_TEST_CASE_P(SignalTests, SignalContext,
    ::testing::ValuesIn(configuration_files));

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}