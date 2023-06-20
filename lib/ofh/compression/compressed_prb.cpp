/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "srsran/ofh/compression/compressed_prb.h"

using namespace srsran;
using namespace ofh;

/// Number of bits in one byte.
static constexpr unsigned BITS_IN_BYTE = 8U;

void compressed_prb::pack_compressed_data(span<const int16_t> compressed_iq,
                                          unsigned            iq_width,
                                          uint8_t             compression_param)
{
  srsran_assert(compressed_iq.size() == NOF_SUBCARRIERS_PER_RB * 2, "Compressed data must contain 12 IQ samples");
  srsran_assert(iq_width <= MAX_WIDTH, "Requested width can not exceed 16");

  comp_param = compression_param;

  unsigned start_pos = 0;
  for (int16_t sample : compressed_iq) {
    unsigned left_to_pack = iq_width;

    while (left_to_pack != 0) {
      unsigned nbits  = std::min(BITS_IN_BYTE, left_to_pack);
      uint8_t  masked = sample;
      if (left_to_pack > BITS_IN_BYTE) {
        masked = sample >> (left_to_pack - BITS_IN_BYTE);
        sample &= mask_lsb_ones<uint16_t>(left_to_pack - BITS_IN_BYTE);
      }
      data.insert(masked, start_pos, nbits);
      start_pos += nbits;
      left_to_pack -= nbits;
    }
  }
  // Clear tail bits of the last used byte
  if (start_pos % BITS_IN_BYTE) {
    unsigned nbits = BITS_IN_BYTE - start_pos % BITS_IN_BYTE;
    data.insert((uint8_t)0, start_pos, nbits);
  }
  bytes_used =
      (compressed_iq.size() * iq_width / BITS_IN_BYTE) + ((compressed_iq.size() * iq_width % BITS_IN_BYTE) ? 1U : 0);
}

int16_t compressed_prb::extract_bits(unsigned pos, unsigned length) const
{
  int16_t  value        = 0;
  unsigned left_to_read = length;
  while (left_to_read) {
    if (left_to_read > BITS_IN_BYTE) {
      value = data.extract(pos, BITS_IN_BYTE);
      value <<= (left_to_read - BITS_IN_BYTE);
      left_to_read -= BITS_IN_BYTE;
      pos += BITS_IN_BYTE;
    } else {
      value |= data.extract(pos, left_to_read);
      left_to_read = 0;
    }
  }
  return value;
}
