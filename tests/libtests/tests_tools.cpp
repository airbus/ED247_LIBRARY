#include "tests_tools.h"
#include <fstream>
#include <regex>

const uint32_t* tests_tools::count_matching_lines_in_file(const char* filename, const char* trace_to_find)
{
  std::ifstream file;
  std::regex to_look_for(trace_to_find);
  static uint32_t nb_match;
  if(!filename) return NULL;
  file.open (filename);
  if (file.is_open())
  {
    nb_match = 0;
    for(std::string line; getline(file, line);)
    {
      if (regex_match(line.c_str(), to_look_for))
      {
        nb_match++;
      }
    }
    file.close();
  } else {
    return NULL;
  }
  return &nb_match;
}
