#include <cmdline.hh>
#include <algorithm>
#include <cmdline_rdline.h>
#include <cmdline_socket.h>

namespace dpdk::cmdline {

void Interface::interact(const std::string &prompt)
{
  using ctx_t = cmdline_parse_ctx_t;
  std::vector<ctx_t> ctx(cmds_.size() + 1, nullptr);
  std::ranges::transform(cmds_, std::begin(ctx), [](const auto &ctx) {
      return ctx->get();
  });

  std::unique_ptr<struct cmdline, decltype(&cmdline_stdin_exit)> cl {
    cmdline_stdin_new(ctx.data(), prompt.c_str()), cmdline_stdin_exit
  };
  if (!cl) throw std::runtime_error {"Cannot create CmdLine instance"};

  cmdline_interact(cl.get());
  cmdline_printf(cl.get(), "\n");
}

} // namespace dpdk::inline cmdline
