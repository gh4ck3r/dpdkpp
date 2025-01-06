#include <iostream>
#include <eal.hh>
#include <cpu.hh>

#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>

#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

[[noreturn]] static void
lcore_main(dpdk::EAL &eal)
{
  const auto diffNUMA = [current = dpdk::cpu::socket_id()]
    (const auto &nic) {
      const auto sid = nic.socket_id();
      return sid != SOCKET_ID_ANY && static_cast<unsigned>(sid) != current;
    };
  for (const auto &nic: eal.ethdevs() | std::views::filter(diffNUMA)) {
    std::clog << "WARNING, port " << nic.id()
      << " is on remote NUMA node to polling thread.\n"
      << "\tPerformance will not be optimal."
      << std::endl;
  }

  std::clog << "\nCore " << dpdk::lcore::id()
    << " forwarding packets. [Ctrl+C to quit]"
    << std::endl;

  auto odd = eal.ethdevs() | std::views::filter(
      [idx=1] (const auto &) mutable { return idx++ % 2; });
  auto even = eal.ethdevs() | std::views::filter(
      [idx=0] (const auto &) mutable { return idx++ % 2; });

  const auto l2forward = [] (auto &from, auto &to) {
    struct rte_mbuf *bufs[BURST_SIZE];

    const auto nb_rx = rte_eth_rx_burst(from.id(), 0, bufs, BURST_SIZE);
    if (nb_rx == 0) [[unlikely]] return;

    const auto nb_tx = rte_eth_tx_burst(to.id(), 0, bufs, nb_rx);

    if (nb_tx < nb_rx) [[unlikely]] {
      for (auto buf = nb_tx; buf < nb_rx; buf++) {
        rte_pktmbuf_free(bufs[buf]);
      }
    }
  };

  for (;;) {
    auto oi = odd.begin();
    auto ei = even.begin();
    for (;oi != odd.end() && ei != even.end(); oi++, ei++) {
      l2forward(*oi, *ei);
      l2forward(*ei, *oi);
    }
  }
}

static inline std::ostream& operator<<(std::ostream &os, const rte_ether_addr &addr)
{
  return os << std::format("{:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}",
      RTE_ETHER_ADDR_BYTES(&addr));
}

int main(int argc, char *argv[])
{
  dpdk::EAL eal{argc, argv};

  if (!eal.workers().empty()) {
    std::clog << "\nWARNING: Too many lcores enabled("
      << eal.n_lcores()
      << "). Only 1 used.\n";
    // XXX close(?) workers if possible
  }

  /* Check that there is an even number of ports to send/receive on. */
  const auto nb_ports = eal.n_ethdevs();
  if (nb_ports < 2 || (nb_ports & 1))
    rte_exit(EXIT_FAILURE, "Error: number of ports must be even\n");

  auto mbuf_pool = eal.make_mempool("MBUF_POOL",
      NUM_MBUFS * nb_ports,
      MBUF_CACHE_SIZE,
      0,
      RTE_MBUF_DEFAULT_BUF_SIZE,
      rte_socket_id());

  for (auto &nic: eal.ethdevs()) try {
    nic.stop()
      .txq({TX_RING_SIZE})
      .rxq({RX_RING_SIZE})
      .promiscuous(true)
      .start();

    // Display the port MAC address.
    std::cout << "Port " << nic.id() << " MAC: " << nic.macaddr() << std::endl;

    if (!nic.promiscuous()) throw std::runtime_error {
      std::format("Cannot enable promiscuous mode for port {}", nic.id())
    };
  } catch (const std::exception &e) {
    rte_exit(EXIT_FAILURE, "Error initializing port %u: %s\n", nic.id(), e.what());
  }

  lcore_main(eal);

  return 0;
}
