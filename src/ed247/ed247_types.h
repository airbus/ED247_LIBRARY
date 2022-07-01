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
#ifndef _ED247_TYPES_
#define _ED247_TYPES_

#include <string>

namespace ed247
{

  typedef struct {
    uint32_t epoch;
    uint32_t nanoseconds;
  } timestamp_t;

  namespace defines{

    static const std::string Unknown {"Unknown"};

    namespace status {
      static const std::string SUCCESS{"SUCCESS"};
      static const std::string FAILURE{"FAILURE"};
      static const std::string TIMEOUT{"TIMEOUT"};
      static const std::string NODATA{"NODATA"};
    }

    namespace standard {
      static const std::string ED247 {"-"};
      static const std::string ED247A {"A"};
    }

    namespace direction {
      static const std::string INPUT {"In"};
      static const std::string OUTPUT {"Out"};
      static const std::string INOUT {"InOut"};
    }

    namespace yesno {
      static const std::string NO{"No"};
      static const std::string YES{"Yes"};
    }

    namespace component_type {
      static const std::string VIRTUAL{"Virtual"};
      static const std::string BRIDGE{"Bridge"};
    }

    namespace stream_type {
      static const std::string A664 {"A664"};
      static const std::string A429 {"A429"};
      static const std::string A825 {"A825"};
      static const std::string M1553 {"M1553"};
      static const std::string SERIAL {"SERIAL"};
      static const std::string AUDIO {"AUDIO"};
      static const std::string VIDEO {"VIDEO"};
      static const std::string ETHERNET {"ETHERNET"};
      static const std::string ANALOG {"ANALOG"};
      static const std::string DISCRETE {"DISCRETE"};
      static const std::string NAD {"NAD"};
      static const std::string VNAD {"VNAD"};
    }

    namespace signal_type {
      static const std::string ANALOG {"ANALOG"};
      static const std::string DISCRETE {"DISCRETE"};
      static const std::string NAD {"NAD"};
      static const std::string VNAD {"VNAD"};
    }

    namespace nad_type {
      static const std::string INT8 {"int8"};
      static const std::string INT16 {"int16"};
      static const std::string INT32 {"int32"};
      static const std::string INT64 {"int64"};
      static const std::string UINT8 {"uint8"};
      static const std::string UINT16 {"uint16"};
      static const std::string UINT32 {"uint32"};
      static const std::string UINT64 {"uint64"};
      static const std::string FLOAT32 {"float32"};
      static const std::string FLOAT64 {"float64"};
    }

  }

}

#endif
