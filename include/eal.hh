#pragma once
#include <rte_eal.h>
#include <rte_debug.h>
#include <lcore.hh>

extern int main(int argc, char **argv);

namespace dpdk::inline eal {

class EAL {
  friend int ::main(int argc, char **argv);
  EAL(int &argc, char **&argv);
  EAL() = delete;
  EAL(const EAL &) = delete;
  ~EAL() noexcept;

  inline lcore::Enabled lcores() { return {}; }
  inline lcore::Workers workers() { return {}; }
};

} // namespace dpdk::inline eal
