#ifndef __A429_TOOLS_H__
#define __A429_TOOLS_H__
#include <cstdint>


//
// ** WARNING **
// The API below store the A429 words in natural order.
//
// This is not the case for most software. The A429 words is often stored
// into an 'int'. On little-endian computers (x86, x64), the byte order of
// the payload may be inverted.
//
// This API is not compatible with software that use an 'int' to store
// A429 words.
//


// A429 word
typedef uint8_t        a429_word_t[4];
typedef uint8_t*       a429_word_ptr_t;
typedef uint8_t const* a429_word_const_ptr_t;


// A429 label number (inverted octal representation)
typedef uint8_t a429_label_number_t;

extern a429_label_number_t a429_label_encode(const char* octal_number);  // return 0 on failure
extern const char* a429_label_decode(a429_label_number_t label_number);
inline void a429_label_set(a429_word_ptr_t word, a429_label_number_t label_number) {
  word[3] = label_number;
}
inline a429_label_number_t a429_label_get(a429_word_const_ptr_t word) {
  return word[3];
}
inline const char* a429_label_get_decode(a429_word_const_ptr_t word) {
  return a429_label_decode(a429_label_get(word));
}

// A429 SDI
typedef enum {
  A429_SDI_00 = 0b00,
  A429_SDI_01 = 0b01,
  A429_SDI_10 = 0b10,
  A429_SDI_11 = 0b11,
} a429_sdi_t;

inline void a429_sdi_set(a429_word_ptr_t word, a429_sdi_t sdi) {
  word[2] = (word[2] & 0xFC) + sdi;
}
inline a429_sdi_t a429_sdi_get(a429_word_const_ptr_t word) {
  return (a429_sdi_t)(word[2] & 0x03);
}


// A429 SSM
// Only the two lower bytes are used
typedef uint8_t a429_ssm_t;

inline void a429_ssm_set(a429_word_ptr_t word, a429_ssm_t ssm) {
  word[0] = (word[0] & 0b10011111) + ((ssm & 0b11) << 5);
}
inline a429_ssm_t a429_ssm_get(a429_word_const_ptr_t word) {
  return (a429_ssm_t)((word[0] & 0b01100000) >> 5);
}


// A429 parity
typedef enum {
  A429_PARITY_EVEN = 0,
  A429_PARITY_ODD  = 1,
} a429_parity_kind_t;

extern void a429_parity_update(a429_word_ptr_t word, a429_parity_kind_t parity_kind = A429_PARITY_ODD);
extern bool a429_parity_match(a429_word_const_ptr_t word, a429_parity_kind_t parity_kind = A429_PARITY_ODD);


// A429 BNR value
// -262144 <= value <= 262143
inline void a429_bnr_set(a429_word_ptr_t word, int32_t value) {
  if (value < 0) word[0] |= 0x10;
  word[2] = ((value & 0x00003F) << 2) + (word[2] & 0x03);
  word[1] =  (value & 0x003FC0) >> 6;
  word[0] = ((value & 0x07C000) >> 14) + (word[0] & 0xE0);
}

inline int32_t a429_bnr_get(a429_word_const_ptr_t word) {
  if (word[0] & 0x10) {
    return -1 -
      (((word[0] & 0x1F) ^ 0x1F) << 14) -
      ((word[1] ^ 0xFF) << 6) -
      ((word[2] ^ 0xFF) >> 2);
  } else {
    return
      ((word[0] & 0x1F) << 14) +
      (word[1] << 6) +
      (word[2] >> 2) +
      0;
  }
}

// Helper function
inline void a429_set(a429_word_ptr_t word, const char* octal_number, a429_sdi_t sdi, uint32_t value) {
  word[0] = 0;
  a429_label_set(word, a429_label_encode(octal_number));
  a429_sdi_set(word, sdi);
  a429_bnr_set(word, value);
}

#endif
