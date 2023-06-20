/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsran/ofh/ofh_uplane_message_decoder.h"
#include "srsran/srslog/logger.h"

namespace srsran {
namespace ofh {
class iq_decompressor;
class network_order_binary_deserializer;

/// Open Fronthaul User-Plane message decoder implementation.
class uplane_message_decoder_impl : public uplane_message_decoder
{
public:
  uplane_message_decoder_impl(srslog::basic_logger& logger_,
                              subcarrier_spacing    scs_,
                              unsigned              nof_symbols_,
                              unsigned              ru_nof_prbs_,
                              iq_decompressor&      decompressor_) :
    logger(logger_), scs(scs_), nof_symbols(nof_symbols_), ru_nof_prbs(ru_nof_prbs_), decompressor(decompressor_)
  {
  }

  // See interface for documentation.
  bool decode(uplane_message_decoder_results& results, span<const uint8_t> message) override;

private:
  /// Decodes the User-Plane message header and returns true on success, otherwise false.
  bool decode_header(uplane_message_decoder_results& results, network_order_binary_deserializer& deserializer);

  /// Decodes the sections and returns true on success, otherwise false.
  bool decode_all_sections(uplane_message_decoder_results& results, network_order_binary_deserializer& deserializer);

  /// Decodes the section header and returns true on success, otherwise false.
  bool decode_section(uplane_message_decoder_results& results, network_order_binary_deserializer& deserializer);

  /// Decodes the section header and returns true on success, otherwise false.
  bool decode_section_header(uplane_section_params& results, network_order_binary_deserializer& deserializer);

  /// Decodes the compression header. Returns true on success, otherwise false.
  virtual bool decode_compression_header(uplane_section_params&             results,
                                         network_order_binary_deserializer& deserializer) = 0;

  /// Decodes the compression length field. Returns true on success, otherwise false.
  bool decode_compression_length(uplane_section_params&             results,
                                 network_order_binary_deserializer& deserializer,
                                 const ru_compression_params&       compression_params);

  /// Decodes the IQ data from the given deserializer. Returns true on success, otherwise false.
  bool decode_iq_data(uplane_section_params&             results,
                      network_order_binary_deserializer& deserializer,
                      const ru_compression_params&       compression_params);

protected:
  srslog::basic_logger&    logger;
  const subcarrier_spacing scs;
  const unsigned           nof_symbols;
  const unsigned           ru_nof_prbs;
  iq_decompressor&         decompressor;
};

} // namespace ofh
} // namespace srsran
