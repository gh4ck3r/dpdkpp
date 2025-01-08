#include <cmdline_parser.hh>
#include <cstdlib>
#include <system_error>

namespace dpdk::cmdline::command {

Context::Context(size_t n_tokens) :
  ctx_(reinterpret_cast<std::remove_const_t<ptr_t>>(std::malloc(
        sizeof(decltype(*ctx_)) + (1 + n_tokens) * sizeof(ptr_t))))
{
  if (!ctx_) throw std::system_error {
    errno, std::system_category(), "failed to create Parser Context"};
}

Context::~Context() noexcept
{
  std::free(ctx_);
}

} // namespace dpdk::cmdline::command
