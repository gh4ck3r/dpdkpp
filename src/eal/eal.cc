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

EAL::EAL(int &argc, char **&argv) : Context{argc, argv}, lcores_{}
{
}

EAL::~EAL() noexcept
{
}

} // namespace dpdk::inline eal
