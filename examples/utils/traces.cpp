#include "traces.h"
#include <iomanip>
#include <bitset>


std::ostream& operator<<(std::ostream &stream, const hex_stream& self)
{
  if (self._payload != nullptr) {
    std::ios::fmtflags old_flags = stream.flags();
    stream.setf(std::ios::hex, std::ios::basefield);
    stream.setf(std::ios::uppercase);
    char old_fill = stream.fill('0');

    for (int pos = 0; pos < self._len; pos++) {
      stream << std::setw(2) << (int)self._payload[pos] << " ";
    }

    stream.fill(old_fill);
    stream.flags(old_flags);
  }
  else {
    stream << "(null)";
  }

  return stream;
}

std::ostream& operator<<(std::ostream &stream, const bin_stream& self)
{
  if (self._payload != nullptr) {
    char old_fill = stream.fill('0');
    for (int pos = 0; pos < self._len; pos++) {
      stream << std::setw(8) << std::bitset<8>((int)self._payload[pos]) << " ";
    }
    stream.fill(old_fill);
  }
  else {
    stream << "(null)";
  }

  return stream;
}


std::ostream& operator<<(std::ostream& stream, const ed247_timestamp_t* ts) {
  if (ts == nullptr) {
    stream << "(nullptr)";
  } else {
    time_t time = ts->epoch_s;
    struct tm* time_m = gmtime(&time);
    char ftime[20];
    strftime(ftime, 20, "%H:%M:%S", time_m);
    stream << ftime;

    stream << '.';
    char oldfill = stream.fill('0');
    int oldwidth = stream.width(3);
    stream << (ts->offset_ns / 1000 / 1000);
    stream.fill(oldfill);
    stream.width(oldwidth);
  }
  return stream;
}
