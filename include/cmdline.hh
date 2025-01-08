#pragma once
#include <memory>
#include <vector>
#include <cmdline_parser.hh>

namespace dpdk::cmdline {

class CmdLine {
 public:
  CmdLine() = default;
  ~CmdLine() noexcept = default;

  template <typename CMD, typename...ARGS>
    requires command::Impl<CMD> and std::constructible_from<CMD, ARGS...>
  inline auto& add(ARGS&&...args) {
    cmds_.emplace_back(std::make_unique<CMD>(std::forward<ARGS>(args)...));
    return *this;
  }

  void interact(const std::string &prompt);

 private:
  std::vector<std::unique_ptr<command::Context>> cmds_;
};

} // namespace dpdk::cmdline

namespace dpdk {
using cmdline::CmdLine;
} // namespace dpdk
