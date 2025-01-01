#pragma once
#include <format>
#include <memory>
#include <stdexcept>
#include <rte_ethdev.h>

#include <iostream>

namespace dpdk::memory {
using PktBufPool = std::shared_ptr<rte_mempool>;
} // namespace dpdk::memory::pool
namespace dpdk::device {

class Ethernet {
  using PktBuf = memory::PktBufPool;
  using size_t = uint16_t;
 public:
  using id_t = uint16_t;

  Ethernet() = delete;
  Ethernet(const Ethernet&) = delete;
  Ethernet &operator=(const Ethernet&) = delete;
  inline Ethernet &operator=(Ethernet&&) = delete;
  Ethernet(id_t port_id, PktBuf mbuf_pool)
      : port_id_{port_id}
      , mbuf_pool_{mbuf_pool}

  {
    if (!rte_eth_dev_is_valid_port(port_id_)) throw std::invalid_argument {
      std::format("Invalid port: {}", port_id_)
    };
  }
  inline Ethernet(Ethernet&& rhs) {
    port_id_ = rhs.port_id_;
    mbuf_pool_ = std::move(rhs.mbuf_pool_);
  }

  enum class Direction { RX, TX };
  using enum Direction;
  template <Direction D>
  void configure(const size_t n_rxq, size_t nb_rxd, size_t n_txq, size_t nb_txd) {
    rte_eth_dev_info dev_info {};
    if (const auto r = rte_eth_dev_info_get(port_id_, &dev_info); r != 0) {
      throw std::runtime_error {std::format(
          "Error during getting device (port {}) info: {}",
          port_id_,
          rte_strerror(-r))
      };
    }

    rte_eth_conf port_conf {};
    port_conf.txmode.offloads |=
      dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE;

    if (const auto r = rte_eth_dev_configure( port_id_, n_rxq, n_txq, &port_conf); r != 0) {
      throw std::runtime_error {std::format(
          "Cannot configure port {}",
          port_id_)
      };
    }

    if constexpr (D == Direction::RX) {
      // XXX check modified nb_rxd, nb_txd returned
      if (const auto r = rte_eth_dev_adjust_nb_rx_tx_desc(
              port_id_, &nb_rxd, &nb_txd); r != 0) {
        throw std::runtime_error {std::format(
            "Cannot adjust number of RX/TX descriptors for port {}",
            port_id_)
        };
      }
      std::clog << "setting RX queue" << std::endl;
      for (uint16_t q = 0; q < n_rxq; q++) {
        if (const auto r = rte_eth_rx_queue_setup(
                port_id_, q, nb_rxd, rte_eth_dev_socket_id(port_id_), nullptr,
                mbuf_pool_.get()); r < 0) {
          throw std::runtime_error {std::format(
              "Cannot setup RX queue {} for port {}",
              q,
              port_id_)
          };
        }
      }
    } else if constexpr (D == Direction::TX) {
      // XXX check modified nb_rxd, nb_txd returned
      if (const auto r = rte_eth_dev_adjust_nb_rx_tx_desc(
              port_id_, &nb_rxd, &nb_txd); r != 0) {
        throw std::runtime_error {std::format(
            "Cannot adjust number of RX/TX descriptors for port {}",
            port_id_)
        };
      }
      auto txconf = dev_info.default_txconf;  // XXX
      txconf.offloads = port_conf.txmode.offloads;
      // allocate and set up 1 TX queue per Ethernet port.
      for (uint16_t q = 0; q < n_txq; q++) {
        if (const auto r = rte_eth_tx_queue_setup(
                port_id_, q, nb_txd, rte_eth_dev_socket_id(port_id_), &txconf);
            r < 0) {
          throw std::runtime_error {std::format(
              "Cannot setup TX queue {} for port {}",
              q,
              port_id_)
          };
        }
      }
    } else {
      static_assert(false, "Unexpected direction");
    }
  }

  inline bool start() { return rte_eth_dev_start(port_id_) == 0; }

  inline bool promiscuous() const { return rte_eth_promiscuous_get(port_id_); }
  inline bool promiscuous(const bool enable) {
    return enable ? rte_eth_promiscuous_enable(port_id_) == 0 :
                    rte_eth_promiscuous_disable(port_id_) == 0;
  }

  inline rte_ether_addr macaddr() const {
    rte_ether_addr addr;
    if (rte_eth_macaddr_get(port_id_, &addr) != 0) throw std::runtime_error {
      std::format("Cannot get MAC address for port {}", port_id_)
    };
    return addr;
  }

  ~Ethernet() noexcept {
    rte_eth_dev_stop(port_id_);
    //rte_eth_dev_close(port_id_);
  }

  inline auto id() const { return port_id_; }

 private:
  id_t port_id_;
  PktBuf mbuf_pool_;
};

} // namespace dpdk::device
