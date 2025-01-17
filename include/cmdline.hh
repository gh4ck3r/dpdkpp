#pragma once
#include <memory>
#include <vector>
#include <cmdline/parser.hh>

namespace dpdk::cmdline {

class Interface {
 public:
  Interface() = default;
  ~Interface() noexcept = default;

  void interact(const std::string &prompt);

 protected:
  template <typename CMD, typename...ARGS>
    requires /*command::Impl<CMD> and*/ std::constructible_from<CMD, ARGS...>
  inline auto& add(ARGS&&...args) {
    cmds_.emplace_back(std::make_unique<CMD>(std::forward<ARGS>(args)...));
    return *this;
  }

 private:
  std::vector<std::unique_ptr<command::Context>> cmds_; // FIXME use ObjectStore
};

} // namespace dpdk::cmdline

namespace dpdk {

using CLI = cmdline::Interface;

} // namespace dpdk
