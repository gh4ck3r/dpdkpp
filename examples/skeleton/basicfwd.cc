#include <iostream>
#include <eal.hh>

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

/* basicfwd.c: Basic DPDK skeleton forwarding example. */

/*
 * Initializes a given port using global settings and with the RX buffers
 * coming from the mbuf_pool passed as a parameter.
 */

/*
 * The lcore main. This is the main thread that does the work, reading from
 * an input port and writing to an output port.
 */

 /* Basic forwarding application lcore. 8< */
[[noreturn]] static void
lcore_main(void)
{
	uint16_t port;

	/*
	 * Check that the port is on the same NUMA node as the polling thread
	 * for best performance.
	 */
	RTE_ETH_FOREACH_DEV(port)
		if (rte_eth_dev_socket_id(port) >= 0 &&
				rte_eth_dev_socket_id(port) !=
						(int)rte_socket_id())
			printf("WARNING, port %u is on remote NUMA node to "
					"polling thread.\n\tPerformance will "
					"not be optimal.\n", port);

	printf("\nCore %u forwarding packets. [Ctrl+C to quit]\n",
			rte_lcore_id());

	/* Main work of application loop. 8< */
	for (;;) {
		/*
		 * Receive packets on a port and forward them on the paired
		 * port. The mapping is 0 -> 1, 1 -> 0, 2 -> 3, 3 -> 2, etc.
		 */
		RTE_ETH_FOREACH_DEV(port) {

			/* Get burst of RX packets, from first port of pair. */
			struct rte_mbuf *bufs[BURST_SIZE];
			const uint16_t nb_rx = rte_eth_rx_burst(port, 0,
					bufs, BURST_SIZE);

			if (unlikely(nb_rx == 0))
				continue;

			/* Send burst of TX packets, to second port of pair. */
			const uint16_t nb_tx = rte_eth_tx_burst(port ^ 1, 0,
					bufs, nb_rx);

			/* Free any unsent packets. */
			if (unlikely(nb_tx < nb_rx)) {
				uint16_t buf;
				for (buf = nb_tx; buf < nb_rx; buf++)
					rte_pktmbuf_free(bufs[buf]);
			}
		}
	}
	/* >8 End of loop. */
}
/* >8 End Basic forwarding application lcore. */

int main(int argc, char *argv[])
{
	dpdk::EAL eal{argc, argv};

	if (!std::ranges::empty(eal.workers())) {
		std::clog << "\nWARNING: Too many lcores enabled("
			<< eal.n_lcores()
			<< "). Only 1 used.\n";
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

	for (auto &nic : eal.ethdevs()) try {
		using enum dpdk::device::Ethernet::Direction;
		nic.configure<RX>(1, RX_RING_SIZE, 1, TX_RING_SIZE);

    // starting ethernet port
		if (!nic.start()) throw std::runtime_error {
			std::format("Cannot start port {}", nic.id())
		};

    // Display the port MAC address.
    const auto addr = nic.macaddr();
		std::cout << "Port " << nic.id()
			<< std::format(" MAC: {:02x} {:02x} {:02x} {:02x} {:02x} {:02x}", RTE_ETHER_ADDR_BYTES(&addr))
			<< std::endl;

    // Enable RX in promiscuous mode for the Ethernet device.
		if (!nic.promiscuous(true)) throw std::runtime_error {
			std::format("Cannot enable promiscuous mode for port {}", nic.id())
		};
	} catch (const std::exception &e) {
		rte_exit(EXIT_FAILURE, "Error initializing port %u: %s\n", nic.id(), e.what());
	}

	lcore_main();

	return 0;
}
