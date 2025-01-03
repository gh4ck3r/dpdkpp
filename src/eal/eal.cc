#include <eal.hh>
#include <rte_launch.h>

namespace dpdk::eal {
Context::Context(int &argc, char **&argv)
{
  int ret = rte_eal_init(argc, argv);
  if (ret < 0)
    rte_panic("Cannot init EAL\n");
}

Context::~Context() noexcept
{
	rte_eal_mp_wait_lcore();

  rte_eal_cleanup();
}

EAL::EAL(int &argc, char **&argv) :
  Context{argc, argv},
  lcores_{},
  pktbuf_{nullptr, &rte_mempool_free},
  ethdevs_{}
{
}

EAL::~EAL() noexcept
{
}

std::vector<device::Ethernet> &EAL::ethdevs()
{
  if (ethdevs_.empty()) {
    if (!pktbuf_) throw std::logic_error {"No Pktbuf pool at the moment"};
    ethdevs_.reserve(n_ethdevs());
    for (uint16_t port_id = 0; port_id < n_ethdevs(); ++port_id) {
      ethdevs_.emplace_back(port_id, pktbuf_);
    }
  }
  return ethdevs_;
}
} // namespace dpdk::inline eal
