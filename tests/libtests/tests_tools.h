#ifndef _TESTS_TOOLS_H_
#define _TESTS_TOOLS_H_
#include "ed247.h"
#include "memhooks.h"
#include "gtest/gtest.h"
#include "ed247_logs.h"

#ifdef __linux__
#include <arpa/inet.h>
#endif

#define ED247_ONE_MILI   1000
#define ED247_100_MILI   (100 * ED247_ONE_MILI)
#define ED247_ONE_SECOND (1000 * ED247_ONE_MILI)

namespace tests_tools {
  /**
   * @brief The function counts the number of line where regex trace_to_find is found in the file pointed by filename.
   * @param[in] filename designates the file to parse.
   * @param[in] trace_to_find designates the regex to look for in each line of the file.
   * @return NULL if the file could not be openned, or a pointer that store the number of hit lines.
   * @note If an ill-formated regex is provided, the corresponding error will be thrown.
   * @note Regex not correctly implemented on gcc4.8.x and earlier (used for linux build)
   * @note Providing ".*" allows to count the number of lines in the provided file
   **/
  const uint32_t* count_matching_lines_in_file(const char* filename, const char* trace_to_find);

  // Display ED247 library information
  void display_ed247_lib_infos();

  // GTEST predicate to compare two payload
  inline ::testing::AssertionResult AssertPayloadEq(const char* exprPayload1,
                                                    const char* exprPayload2,
                                                    const char* exprSize,
                                                    const char* payload1,
                                                    const char* payload2,
                                                    int size)
  {
    if (memcmp(payload1, payload2, size) == 0) return testing::AssertionSuccess();
    return testing::AssertionFailure() << "Expect equality of payloads (size " << size << "): " << exprPayload1 << ", " <<  exprPayload2 << std::endl
                                       << "  " << hex_stream(payload1, size) << std::endl
                                       << "  " << hex_stream(payload2, size);
  }

}

#define ASSERT_PAYLOAD_EQ(payload1, payload2, size) ASSERT_PRED_FORMAT3(::tests_tools::AssertPayloadEq, payload1, payload2, size)

#define ASSERT_POP_EQ(stream, size, expected_data)                                                                                     \
  do {                                                                                                                                 \
    const char* payload = nullptr;                                                                                                     \
    uint32_t payload_size;                                                                                                             \
    char expected[size];                                                                                                               \
    memset(expected, expected_data, size);                                                                                             \
    ASSERT_EQ(ed247_stream_pop_sample((stream), (const void**)&payload, &payload_size, NULL, NULL, NULL, NULL), ED247_STATUS_SUCCESS); \
    ASSERT_EQ(payload_size, size);                                                                                                     \
    ASSERT_PAYLOAD_EQ(payload, expected, size);                                                                                        \
} while (0)


#define ASSERT_POP_NODATA(stream)                                                                                                     \
  do {                                                                                                                                \
    const char* payload = nullptr;                                                                                                    \
    uint32_t payload_size;                                                                                                            \
    ASSERT_EQ(ed247_stream_pop_sample((stream), (const void**)&payload, &payload_size, NULL, NULL, NULL, NULL), ED247_STATUS_NODATA)  \
      << " Received payload (size " << payload_size << ") is: " << std::endl << "  " << hex_stream(payload, payload_size);            \
    } while (0)


#endif
