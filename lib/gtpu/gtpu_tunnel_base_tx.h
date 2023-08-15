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

#include "gtpu_tunnel_logger.h"
#include "srsran/gtpu/gtpu_tunnel_tx.h"
#include "srsran/pcap/pcap.h"
#include <arpa/inet.h>
#include <cstdint>
#include <netinet/in.h>

namespace srsran {

/// Class used for transmitting GTP-U bearers.
class gtpu_tunnel_base_tx
{
public:
  gtpu_tunnel_base_tx(gtpu_tunnel_log_prefix               log_prefix,
                      dlt_pcap&                            gtpu_pcap_,
                      gtpu_tunnel_tx_upper_layer_notifier& upper_dn_) :
    logger("GTPU", log_prefix), gtpu_pcap(gtpu_pcap_), upper_dn(upper_dn_)
  {
  }

  bool to_sockaddr(sockaddr_storage& out_sockaddr, const char* addr, uint16_t port)
  {
    // Validate configuration
    if (inet_pton(AF_INET, addr, &((::sockaddr_in*)&out_sockaddr)->sin_addr) == 1) {
      ((::sockaddr_in*)&out_sockaddr)->sin_family = AF_INET;
      ((::sockaddr_in*)&out_sockaddr)->sin_port   = htons(port);
    } else if (inet_pton(AF_INET6, addr, &((::sockaddr_in6*)&out_sockaddr)->sin6_addr) == 1) {
      ((::sockaddr_in6*)&out_sockaddr)->sin6_family = AF_INET6;
      ((::sockaddr_in6*)&out_sockaddr)->sin6_port   = htons(port);
    } else {
      logger.log_error("Invalid address or port. addr={} port={} errno={}", addr, port, strerror(errno));
      return false;
    }
    return true;
  }

protected:
  gtpu_tunnel_logger logger;

  void send_pdu(byte_buffer buf, const ::sockaddr_storage& peer_sockaddr)
  {
    if (gtpu_pcap.is_write_enabled()) {
      gtpu_pcap.push_pdu(buf.deep_copy());
    }

    upper_dn.on_new_pdu(std::move(buf), peer_sockaddr);
  }

private:
  dlt_pcap&                            gtpu_pcap;
  gtpu_tunnel_tx_upper_layer_notifier& upper_dn;
};
} // namespace srsran
