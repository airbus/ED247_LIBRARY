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


#ifdef __linux__
#include <dlfcn.h>
static std::string ed247_lib_path() {
  Dl_info info;
  if (dladdr((void*)&ed247_load_file, &info) != 0 && info.dli_fname) {
    return info.dli_fname;
  } else {
    return "<Unknown library path>";
  }
}
#else
static std::string ed247_lib_path() {
  return std::string();
}
#endif

void tests_tools::display_ed247_lib_infos()
{
  SAY("ED247 Library " << ed247_get_implementation_version() << " - " << ed247_lib_path());
}
