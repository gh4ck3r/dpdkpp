#pragma once
#include <memory>
#include <ranges>
#include <rte_eal.h>
#include <rte_debug.h>
#include <lcore.hh>
#include <device/ethernet.hh>

extern int main(int argc, char **argv);

namespace dpdk {
namespace eal {

class Context {
  friend class EAL;
  Context(int &argc, char **&argv);
  ~Context() noexcept;
};

class EAL : Context {
  friend int ::main(int argc, char **argv);
  EAL(int &argc, char **&argv);
  EAL() = delete;
  EAL(const EAL &) = delete;
  ~EAL() noexcept;

 public:
  inline auto n_lcores() { return rte_lcore_count(); }
  inline auto lcore_main() { return lcores_.main(); }
  inline auto workers() {
    return lcores_.workers() | std::views::filter([](auto &lcore) {
      return lcore.is_enabled();
    });
  }

  inline size_t n_ethdevs() const { return rte_eth_dev_count_avail(); }
  std::vector<device::Ethernet> &ethdevs();
  [[nodiscard]]
  auto make_mempool(
      const char *name,
      unsigned n,
      unsigned cache_size,
      unsigned priv_size,
      unsigned data_room_size,
      int socket_id)
  {
    if (!pktbuf_) {
      pktbuf_.reset(
        rte_pktmbuf_pool_create(name, n, cache_size, priv_size, data_room_size, socket_id),
        rte_mempool_free
      );

      if (!pktbuf_) throw std::runtime_error {"Cannot create mbuf pool"};
    }

    return pktbuf_;
  }

 private:
  lcore::Manager lcores_;
  memory::PktBufPool pktbuf_;
  std::vector<device::Ethernet> ethdevs_;
};

} // namespace eal
using eal::EAL;
} // namespace dpdk
