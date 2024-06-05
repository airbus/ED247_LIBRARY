#include "a429_tools.h"
#include "string.h"

#define CHAR_0  '0'
a429_label_number_t a429_label_encode(const char* octal_number)
{
  // Get and check string length
  // Remove first 0 if string use C octal notation
  int len = strlen(octal_number);
  if (len == 4 && octal_number[0] == CHAR_0) {
    len = 3;
    octal_number = octal_number + 1;
  }
  if (len == 0 || len > 3) return 0;


  // Extract digits and check them
  int digits[3] = { 0, 0, 0 };
  for (int i = 0; i < len; i++) {
    if (octal_number[i] < CHAR_0 || octal_number[i] > CHAR_0 + 7) return 0;
    digits[len - i - 1] = octal_number[i] - CHAR_0;
  }

  // The major digit cannot be greater than 3 (377 is the max octal 8-bits value)
  if (digits[2] > 3) return 0;

  // Convert digits to their decimal representation according to A429 protocol
  uint8_t inverted_number = (digits[2] << 6) + (digits[1] << 3) + digits[0];
  return 0
    | ((inverted_number & 0x01) << 7)
    | ((inverted_number & 0x02) << 5)
    | ((inverted_number & 0x04) << 3)
    | ((inverted_number & 0x08) << 1)
    | ((inverted_number & 0x10) >> 1)
    | ((inverted_number & 0x20) >> 3)
    | ((inverted_number & 0x40) >> 5)
    | ((inverted_number & 0x80) >> 7);
}


static const char* number_2_octal_string[256] = {
      "000", "200", "100", "300", "040", "240", "140", "340", // labels 000 - 007
      "020", "220", "120", "320", "060", "260", "160", "360", // labels 010 - 017
      "010", "210", "110", "310", "050", "250", "150", "350", // labels 020 - 027
      "030", "230", "130", "330", "070", "270", "170", "370", // labels 030 - 037
      "004", "204", "104", "304", "044", "244", "144", "344", // labels 040 - 047
      "024", "224", "124", "324", "064", "264", "164", "364", // labels 050 - 057
      "014", "214", "114", "314", "054", "254", "154", "354", // labels 060 - 067
      "034", "234", "134", "334", "074", "274", "174", "374", // labels 070 - 077
      "002", "202", "102", "302", "042", "242", "142", "342", // labels 100 - 107
      "022", "222", "122", "322", "062", "262", "162", "362", // labels 110 - 117
      "012", "212", "112", "312", "052", "252", "152", "352", // labels 120 - 127
      "032", "232", "132", "332", "072", "272", "172", "372", // labels 130 - 137
      "006", "206", "106", "306", "046", "246", "146", "346", // labels 140 - 147
      "026", "226", "126", "326", "066", "266", "166", "366", // labels 150 - 157
      "016", "216", "116", "316", "056", "256", "156", "356", // labels 160 - 167
      "036", "236", "136", "336", "076", "276", "176", "376", // labels 170 - 177
      "001", "201", "101", "301", "041", "241", "141", "341", // labels 200 - 207
      "021", "221", "121", "321", "061", "261", "161", "361", // labels 210 - 217
      "011", "211", "111", "311", "051", "251", "151", "351", // labels 220 - 227
      "031", "231", "131", "331", "071", "271", "171", "371", // labels 230 - 237
      "005", "205", "105", "305", "045", "245", "145", "345", // labels 240 - 247
      "025", "225", "125", "325", "065", "265", "165", "365", // labels 250 - 257
      "015", "215", "115", "315", "055", "255", "155", "355", // labels 260 - 267
      "035", "235", "135", "335", "075", "275", "175", "375", // labels 270 - 277
      "003", "203", "103", "303", "043", "243", "143", "343", // labels 300 - 307
      "023", "223", "123", "323", "063", "263", "163", "363", // labels 310 - 317
      "013", "213", "113", "313", "053", "253", "153", "353", // labels 320 - 327
      "033", "233", "133", "333", "073", "273", "173", "373", // labels 330 - 337
      "007", "207", "107", "307", "047", "247", "147", "347", // labels 340 - 347
      "027", "227", "127", "327", "067", "267", "167", "367", // labels 350 - 357
      "017", "217", "117", "317", "057", "257", "157", "357", // labels 360 - 367
      "037", "237", "137", "337", "077", "277", "177", "377"  // labels 370 - 377
};

extern const char* a429_label_decode(a429_label_number_t label_number)
{
  return number_2_octal_string[label_number];
}


static const int a429_parity_table[256] =
{
# define P2(n) n, n^1, n^1, n
# define P4(n) P2(n), P2(n^1), P2(n^1), P2(n)
# define P6(n) P4(n), P4(n^1), P4(n^1), P4(n)
  P6((int)A429_PARITY_EVEN), P6((int)A429_PARITY_ODD), P6((int)A429_PARITY_ODD), P6((int)A429_PARITY_EVEN)
};


void a429_parity_update(a429_word_ptr_t word, a429_parity_kind_t parity_kind)
{
  word[0] &= 0x7F;
  if (a429_parity_table[word[0] ^ word[1] ^ word[2] ^ word[3]] == parity_kind) {
    word[0] |= 0x80;
  }
}

bool a429_parity_match(const a429_word_ptr_t word, a429_parity_kind_t parity_kind)
{
  bool shall_be_set = (a429_parity_table[(word[0]&0x7F) ^ word[1] ^ word[2] ^ word[3]] == parity_kind);
  return (bool)(word[0] & 0x80) == shall_be_set;
}

a429_sdi_t a429_sdi_from_string(const char* const str_sdi) {
  if (strncmp(str_sdi, "00", 2) == 0) return A429_SDI_00;
  if (strncmp(str_sdi, "01", 2) == 0) return A429_SDI_01;
  if (strncmp(str_sdi, "10", 2) == 0) return A429_SDI_10;
  if (strncmp(str_sdi, "11", 2) == 0) return A429_SDI_11;
  return A429_SDI_00; // Errors not handled
}
