#pragma once
#include <ranges>
#include <rte_eal.h>
#include <rte_debug.h>
#include <lcore.hh>

extern int main(int argc, char **argv);

namespace dpdk{
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

  inline auto n_lcores() { return rte_lcore_count(); }
  inline auto lcore_main() { return lcores_.main(); }
  inline auto workers() {
    return lcores_.workers() | std::views::filter([](auto &lcore) {
      return lcore.is_enabled();
    });
  }

 private:
  lcore::Manager lcores_;
};

} // namespace eal
using eal::EAL;
} // namespace dpdk
