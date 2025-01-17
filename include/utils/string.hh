#pragma once
#include <algorithm>

namespace dpdk {

template <typename CharT, std::size_t N>
struct Literal {
  using value_type = CharT;
  value_type data[N] {};
  constexpr inline Literal(const value_type (&s)[N]) : data{} {
    std::copy_n(s, N, data);
  }
  constexpr inline size_t size() const { return N; }
};

} // namespace dpdk
