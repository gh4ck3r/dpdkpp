#pragma once
#include <algorithm>
#include <format>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>
#include <cpu.hh>
#include <rte_ethdev.h>

namespace dpdk::memory {
using PktBufPool = std::shared_ptr<rte_mempool>;
} // namespace dpdk::memory::pool
namespace dpdk::device {

class Ethernet {
  class Config;
  using PktBuf = memory::PktBufPool;
  using size_t = uint16_t;
  using id_t = uint16_t;

 public:
  inline Ethernet(const id_t port_id, PktBuf mbuf_pool);
  inline Ethernet(Ethernet&& rhs);

  Ethernet() = delete;
  Ethernet(const Ethernet&) = delete;
  Ethernet &operator=(const Ethernet&) = delete;
  Ethernet &operator=(Ethernet&&) = delete;

  inline bool start() { return rte_eth_dev_start(port_id_) == 0; }
  [[nodiscard]] inline Config stop();

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
  inline auto socket_id() const {
    return rte_eth_dev_socket_id(id()); // FIXME what if negative(SOCKET_ID_ANY)?
  }

 private:
  inline auto devinfo() {
    rte_eth_dev_info dev_info;
    if (const auto r = rte_eth_dev_info_get(id(), &dev_info); r != 0) {
      throw std::runtime_error {std::format(
          "Error during getting device (port {}) info: {}", id(), rte_strerror(-r))
      };
    }
    return dev_info;
  }

  inline rte_eth_conf port_conf() {
    rte_eth_conf port_conf {};
    port_conf.txmode.offloads |=
      devinfo().tx_offload_capa & RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE;
    return port_conf;
  }

 private:
  id_t port_id_;
  PktBuf mbuf_pool_;
};

Ethernet::Ethernet(const id_t port_id, PktBuf mbuf_pool)
  : port_id_{port_id}, mbuf_pool_{mbuf_pool}
{
  if (!rte_eth_dev_is_valid_port(id())) throw std::invalid_argument {
    std::format("Invalid port: {}", id())
  };

  constexpr size_t n_rxq = 0, n_txq = 0;  // using default
  const rte_eth_conf conf = port_conf();
  if (const auto err = rte_eth_dev_configure(id(), n_rxq, n_txq, &conf); err) {
    throw std::runtime_error { std::format("Cannot configure port {}", id())};
  }

  constexpr id_t qid = 0;       // first queue id
  constexpr size_t n_desc = 0;  // using default
  if (const auto err = rte_eth_tx_queue_setup(
        id(), qid, n_desc, socket_id(), nullptr); err) {
    throw std::runtime_error { std::format(
        "failed to setup tx queue of port {}: {}", id(), err)};
  }

  if (const auto err = rte_eth_rx_queue_setup(
        id(), qid, n_desc, socket_id(), nullptr, mbuf_pool_.get()); err)
  {
    throw std::runtime_error { std::format(
        "failed to setup rx queue of port {}: {}", id(), err) };
  }
}

Ethernet::Ethernet(Ethernet&& rhs) {
  port_id_ = std::exchange(rhs.port_id_, 0);
  mbuf_pool_ = std::exchange(rhs.mbuf_pool_, {});
}

class Ethernet::Config {
  friend class Ethernet;
  Ethernet &owner_;
  Config() = delete;
  Config(Ethernet &owner) : owner_(owner) {}

 public:
  inline Config &&rxq(const std::vector<size_t> &conf) &&;
  inline Config &&txq(const std::vector<size_t> &conf) &&;

  template<typename...ARGS>
  inline Config &&promiscuous(ARGS&&...args) && {
    if (!owner_.promiscuous(std::forward<ARGS>(args)...)) [[unlikely]] {
      throw std::runtime_error {
        std::format("failed to set promiscuous mode at {}", owner_.id())};
    }
    return std::move(*this);
  }

  inline decltype(auto) start() && { return (owner_.start()); }
};

Ethernet::Config Ethernet::stop() {
  const auto err = rte_eth_dev_stop(port_id_);
  if (!err) [[likely]] return *this;

  if (err == -EBUSY) throw std::logic_error {
    std::format("failed to stop port {}: EBUSY", port_id_)};

  if (err < 0) [[likely]] {
    throw std::runtime_error{rte_strerror(-err)};
  } else {
    throw std::system_error{-err, std::system_category(),
      std::format("unkonwn error while stopping port {}", port_id_)};
  }
}


Ethernet::Config &&Ethernet::Config::rxq(const std::vector<size_t> &conf) &&
{
  if (std::numeric_limits<uint16_t>::max()  < conf.size()) {
    throw std::invalid_argument {"Ethernet queue size overflow"};
  }

  auto nb_rxd = std::ranges::max(conf);
  // XXX check modified nb_rxd, nb_txd returned
  if (const auto r = rte_eth_dev_adjust_nb_rx_tx_desc(
          owner_.id(), &nb_rxd, nullptr); r != 0) {
    throw std::runtime_error {std::format(
        "Cannot adjust number of RX/TX descriptors for port {}",
        owner_.id())
    };
  }

  // TODO keep mbuf_pool
  for (auto q = 0u; q < conf.size(); q++) {
    if (const auto r = rte_eth_rx_queue_setup(
            owner_.id(), q, conf[q], owner_.socket_id(), nullptr,
            owner_.mbuf_pool_.get()); r < 0) {
      throw std::runtime_error {std::format(
          "Cannot setup RX queue {} for port {}", q, owner_.id())
      };
    }
  }
  return std::move(*this);
}

Ethernet::Config &&Ethernet::Config::txq(const std::vector<size_t> &conf) &&
{
  if (std::numeric_limits<uint16_t>::max()  < conf.size()) {
    throw std::invalid_argument {"Ethernet queue size overflow"};
  }

  auto nb_txd = std::ranges::max(conf);
  // XXX check modified nb_rxd, nb_txd returned
  if (const auto r = rte_eth_dev_adjust_nb_rx_tx_desc(
        owner_.id(), nullptr, &nb_txd); r != 0) {
    throw std::runtime_error {std::format(
        "Cannot adjust number of RX/TX descriptors for port {}",
        owner_.id())
    };
  }

  const auto &devinfo = owner_.devinfo();
  auto txconf = devinfo.default_txconf;  // XXX
  txconf.offloads = devinfo.tx_offload_capa & RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE;
  // allocate and set up 1 TX queue per Ethernet port.
  for (auto q = 0u; q < conf.size(); q++) {
    if (const auto r = rte_eth_tx_queue_setup(
          owner_.id(), q, conf[q], owner_.socket_id(), &txconf);
        r < 0) {
      throw std::runtime_error {std::format(
          "Cannot setup TX queue {} for port {}",
          q,
          owner_.id())
      };
    }
  }
  return std::move(*this);
}

} // namespace dpdk::device
