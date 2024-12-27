#include <eal.hh>
#include <rte_launch.h>

namespace dpdk::inline eal {

EAL::EAL(int &argc, char **&argv)
{
  int ret = rte_eal_init(argc, argv);
  if (ret < 0)
    rte_panic("Cannot init EAL\n");
}

EAL::~EAL() noexcept
{
	rte_eal_mp_wait_lcore();

  rte_eal_cleanup();
}

} // namespace dpdk::inline eal
