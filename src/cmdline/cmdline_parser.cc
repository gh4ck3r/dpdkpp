#include <cmdline/parser.hh>
#include <cstdlib>
#include <system_error>

namespace dpdk::cmdline::command {

Context::Context(size_t n_tokens) :
  ctx_(reinterpret_cast<std::remove_const_t<pointer>>(std::malloc(
        sizeof(decltype(*ctx_)) + (1 + n_tokens) * sizeof(pointer))))
{
  if (!ctx_) throw std::system_error {
    errno, std::system_category(), "failed to create Parser Context"};
}

Context::~Context() noexcept
{
  std::free(ctx_);
}

} // namespace dpdk::cmdline::command
