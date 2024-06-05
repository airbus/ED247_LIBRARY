#include "ed247.h"
#include "time_tools.h"
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <memory>


#define LOG_SHORTFILE       (strrchr("/" __FILE__, '/') + 1)
#define LOG_STREAM_FILELINE LOG_SHORTFILE << ":" << __LINE__ << " "

#define SAY(m) std::cout << m << std::endl;
#define ERR(m) std::cerr << LOG_STREAM_FILELINE << m << std::endl;
#define DIE(m) do { ERR(m); exit(1); } while (0)
#define ASSERT(t) do { if (!(t)) DIE(#t " FAILED"); } while (0)

// Options
namespace {
  std::string ecic_path;
  bool fill_depth = false;
  uint64_t period_ms = 50;
  uint64_t duration_ms = 0;
}



//
// Classes that push data over network
//
struct transceiver_t {
  transceiver_t(ed247_stream_t stream) : _stream(stream) {}
  virtual void push() = 0;
protected:
  ed247_stream_t _stream;
};

struct generic_transceiver_t : public transceiver_t {
  generic_transceiver_t(ed247_stream_t stream) :
    transceiver_t(stream),
    _payload(nullptr)
  {
    _sample_size = ed247_stream_get_sample_max_size_bytes(stream);
    _qdepth = ed247_stream_get_sample_max_number(stream);
    _id = ed247_stream_get_uid(stream);

    // Generate an ID for non-multichannel streams
    if (_id == 0) _id = next_id++;

    SAY("Initialize stream " << ed247_stream_get_name(stream) << " " <<
        ed247_stream_type_string(ed247_stream_get_type(stream)) << " "
        "ID: " << _id << " "
        "Sample size: " << _sample_size << " "
        "Depth: " << _qdepth << " "
        );
    _payload = new uint8_t[_sample_size];
    memset(_payload, 0, _sample_size);

    // If the payload size allows it, write the UID at the begining
    if (_sample_size > 4) {
      *(uint32_t*)_payload = _id;
     _payload_update = _payload + 4;
      _payload_update_size = _sample_size - 4;
    } else {
      _payload_update = _payload;
      _payload_update_size = _sample_size;
    }

  }
  void push() override {
    for (uint32_t depthid = 0;
         (fill_depth && depthid < _qdepth) || depthid == 0;
         depthid++)
    {
      memset(_payload_update,
             _payload_update[0] + 1,  // uint8 -> % 256
             _payload_update_size);
      ASSERT(ed247_stream_push_sample(_stream, _payload, _sample_size, nullptr, nullptr) == ED247_STATUS_SUCCESS);
    }
  }


private:
  uint32_t _id;
  uint8_t* _payload;
  uint32_t _sample_size;
  uint32_t _qdepth;

  uint8_t* _payload_update;
  uint32_t _payload_update_size;

  static uint32_t next_id;
};
uint32_t generic_transceiver_t::next_id = 100000;



void usage() {
  SAY("USAGE: chatbot [--fill_depth] [-p <period_ms>] [-d <duration_ms>] <ECIC>");
  SAY("");
  SAY("  --fill_depth     Push as mush sample as possible (according to sample_max_number value in ECIC)");
  SAY("  -p <period_ms>   number: Send all streams each <period_ms> miliseconds. Default: " << period_ms << " ms.");
  SAY("                   'match': Send samples according to period defined in ECIC and CMD files (NOT YET IMPLEMENTED)");
  SAY("  -d <duration_ms> number: Send all streams during <duration_ms> miliseconds. 0 means never stop. Default: " << duration_ms << " ms.");
  exit(1);
}

uint64_t parse_int_parameter(std::string arg, std::string value) {
  try {
    std::size_t last;
    uint64_t res = std::stoi(value, &last);
    if (last != value.size()) throw 42;
    return res;
  } catch (...) {
    DIE("Invalid " << arg << " parameter: '" << value << "'");
  }
}

int main(int argc, char** argv)
{
  //
  // Parse arguments
  //
  for (int arg_id = 1; arg_id < argc; arg_id++) {
    std::string arg = argv[arg_id];

    if (arg == "-h" || arg == "--help") {
      usage();
    }
    else if (arg == "--fill_depth") {
      fill_depth = true;
    }
    else if (arg == "-p") {
      if (++arg_id >= argc) DIE("-p option require an argument !");
      std::string value = argv[arg_id];
      if (arg == "match") DIE("period = 'match' not yet implemented !");
      period_ms = parse_int_parameter(arg, value);
    }
    else if (arg == "-d") {
      if (++arg_id >= argc) DIE("-d option require an argument !");
      std::string value = argv[arg_id];
      duration_ms = parse_int_parameter(arg, value);
    }
    else if (arg[0] == '-') {
      DIE("Unknown option '" << arg << "'.");
    }
    else {
      if (ecic_path.empty() == false) {
        DIE("Unexpected argument '" << arg << "'.");
      }
      ecic_path = arg;
    }
  }


  //
  // Parse ECIC and create transceivers
  //
  ed247_context_t context = nullptr;
  ed247_stream_list_t stream_list = nullptr;
  ASSERT(ed247_load_file(ecic_path.c_str(), &context) == ED247_STATUS_SUCCESS);
  ASSERT(ed247_get_stream_list(context, &stream_list) == ED247_STATUS_SUCCESS);

  uint32_t stream_count = 0;
  ASSERT(ed247_stream_list_size(stream_list, &stream_count) == ED247_STATUS_SUCCESS);
  SAY(stream_count << " streams in this ECIC.");

  using transceiver_list_t = std::vector<std::unique_ptr<transceiver_t>>;
  transceiver_list_t transceiver_list;
  ed247_stream_t stream = nullptr;
  do {
    ASSERT(ed247_stream_list_next(stream_list, &stream) == ED247_STATUS_SUCCESS);
    if (stream == nullptr) break;
    if (ed247_stream_get_direction(stream) & ED247_DIRECTION_OUT) {
      transceiver_list.emplace_back(new generic_transceiver_t(stream));
    }
  } while (stream != nullptr);

  //
  // Send data
  //
  if (duration_ms) {
    SAY("Begin sending at " << period_ms << " ms during " << duration_ms << " ms.");
  } else {
    SAY("Begin sending at " << period_ms << " ms.");
  }
  uint64_t send_start_time = time_tools::get_monotonic_time_us();
  do {
    uint64_t cycle_start_time = time_tools::get_monotonic_time_us();
    for (auto& itransceiver: transceiver_list) {
      itransceiver->push();
    }
    ed247_send_pushed_samples(context);
    uint64_t cycle_end_time = time_tools::get_monotonic_time_us();
    if (duration_ms && duration_ms * 1000 > cycle_end_time - send_start_time) break;
    time_tools::sleep_us(period_ms * 1000 + cycle_start_time - cycle_end_time);
  } while (true);

  ed247_stream_list_free(stream_list);
  ed247_unload(context);
  return 0;
}
