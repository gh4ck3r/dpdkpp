#pragma once
#include <algorithm>
#include <cmdline_parse_string.h>
#include <cmdline_parse_ipaddr.h>

namespace dpdk::cmdline::token {

template <typename CharT, std::size_t N>
struct Literal {
  using value_type = CharT;
  value_type data[N] {};
  constexpr inline Literal(const value_type (&s)[N]) : data{} {
    std::copy_n(s, N, data);
  }
  constexpr inline size_t size() const { return N; }
};

template <Literal CMD, typename...ARGS>
struct Command {
  decltype(CMD.data) cmd_;
};  // TODO implement

using base_t = cmdline_token_hdr;
template <typename CRTP>	// TODO add requires TokenImpl
class Parser : public base_t {
  inline static constexpr CRTP &cast(base_t *&THIS) {
    return *static_cast<CRTP*>(THIS);
  }
  inline static constexpr cmdline_token_ops ops_ {
    .parse = [] (base_t *tk, const char *buf, void *res, unsigned size) {
      return cast(tk).parse(buf, res, size);
    },
    .complete_get_nb = [] (base_t *tk) {
      return cast(tk).complete_get_nb();
    },
    .complete_get_elt = [] (base_t *tk, int idx, char *dstbuf, unsigned int size) {
      return cast(tk).complete_get_elt(idx, dstbuf, size);
    },
    .get_help = [] (base_t *tk, char *dstbuf, unsigned int size) {
      return cast(tk).get_help(dstbuf, size);
    },
  };

  protected:
  // TODO
  //Parser(const Parser &) = delete;
  //Parser(Parser &&) = delete;
  constexpr Parser(unsigned int offset) :
    base_t {
      .ops = const_cast<decltype(base_t::ops)>(&ops_),
      .offset = offset,
    }
  {}

  private:
};

class String : cmdline_token_string
{
 public:
  constexpr String(unsigned int offset, const char *s = nullptr) :
    cmdline_token_string TOKEN_STRING_INITIALIZER(cmdline_token_string, hdr, s)
  {
    hdr.offset = offset;
  }
};

class IpAddr : cmdline_parse_token_ipaddr_t
{
 public:
  constexpr IpAddr(unsigned int offset) :
    cmdline_parse_token_ipaddr_t TOKEN_IPADDR_INITIALIZER(cmdline_parse_token_ipaddr_t, hdr)
  {
    hdr.offset = offset;
  }
};

} // namespace dpdk::cmdline::token
