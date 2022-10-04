/**
 *
 * \section COPYRIGHT
 *
 * Copyright 2013-2021 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "rlc_stress_test_mac.h"

using namespace srsgnb;

void mac_dummy::run_tx_tti()
{
  // Pull a number of RLC PDUs
  for (uint32_t i = 0; i < args.nof_pdu_tti; i++) {
    // Get MAC opportunity size (maximum size of the RLC PDU)
    float factor = 1.0f;
    if (not args.const_opp) {
      factor = 0.5f + real_dist(rgen);
    }
    int opp_size = static_cast<int>(args.avg_opp_size * factor);

    // Request data to transmit
    if (bsr.load(std::memory_order_relaxed) > 0) {
      byte_buffer_slice_chain pdu = rlc_tx_lower->pull_pdu(opp_size);
      // Push PDU in the list
      if (pdu.length() > 0) {
        pdu_list.push_back(std::move(pdu));
      }
    }
  }
}

void mac_dummy::run_rx_tti()
{
  auto pdu_it      = pdu_list.begin(); // PDU iterator
  bool skip_action = false;            // Avoid discarding a duplicated or duplicating a discarded

  while (pdu_it != pdu_list.end()) {
    // Drop
    float rnd = real_dist(rgen);
    if (std::isnan(rnd) || ((rnd > args.pdu_drop_rate) || skip_action)) {
      // Cut/Trim
      byte_buffer_slice_chain::iterator pdu_end = pdu_it->end();
      if ((real_dist(rgen) < args.pdu_cut_rate)) {
        size_t pdu_len     = pdu_it->length();
        size_t trimmed_len = static_cast<size_t>(pdu_len * real_dist(rgen));
        if (trimmed_len < pdu_len) {
          pdu_end = pdu_it->begin() + trimmed_len;
        }
        logger.info("Cutting MAC PDU len ({} B -> {} B)", pdu_it->length(), pdu_len);
      }

      // Write PDU copy in RX
      rlc_rx_lower->handle_pdu(byte_buffer(pdu_it->begin(), pdu_end));

      // Write PCAP
      // TODO: write PCAP
    } else {
      logger.info(pdu_it->begin(), pdu_it->end(), "Dropping RLC PDU ({} B)", pdu_it->length());
      skip_action = true; // Avoid drop duplicating this PDU
    }

    // Duplicate
    if (real_dist(rgen) > args.pdu_duplicate_rate || skip_action) {
      pdu_it++;
      skip_action = false; // Allow action on the next PDU
    } else {
      logger.info(pdu_it->begin(), pdu_it->end(), "Duplicating RLC PDU ({} B)", pdu_it->length());
      skip_action = true; // Avoid drop of this PDU
    }
  }

  // clear pdu_list
  pdu_list.clear();
}