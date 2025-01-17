#pragma once
#include <concepts>
#include <utility>
#include <utils/polystore.hh>
#include <cmdline_parse_string.h>
#include <cmdline_parse_ipaddr.h>

namespace dpdk::cmdline::token {

using TokenParserHdr = cmdline_parse_token_hdr_t;
using TokenParsers = PolyStore<TokenParserHdr>;

template <typename CRTP>	// TODO add requires TokenImpl
class TokenParser : public TokenParserHdr {
  inline static constexpr CRTP &cast(TokenParserHdr *&THIS) {
    return *static_cast<CRTP*>(THIS);
  }
  inline static constexpr cmdline_token_ops ops_ {
    .parse = [] (TokenParserHdr *tk, const char *buf, void *res, unsigned size) {
      return cast(tk).parse(buf, res, size);
    },
    .complete_get_nb = [] (TokenParserHdr *tk) {
      return cast(tk).complete_get_nb();
    },
    .complete_get_elt = [] (TokenParserHdr *tk, int idx, char *dstbuf, unsigned int size) {
      return cast(tk).complete_get_elt(idx, dstbuf, size);
    },
    .get_help = [] (TokenParserHdr *tk, char *dstbuf, unsigned int size) {
      return cast(tk).get_help(dstbuf, size);
    },
  };

 protected:
  // TODO
  //TokenParser(const TokenParser &) = delete;
  //TokenParser(TokenParser &&) = delete;
  constexpr TokenParser() :
    TokenParserHdr {
      .ops = const_cast<decltype(TokenParserHdr::ops)>(&ops_),
      .offset = 0,
    }
  {}
};

template <typename T>
concept AdoptableTokenParser = requires (T t) {
  requires std::same_as<decltype(T::hdr), TokenParserHdr> and offsetof(T, hdr) == 0;
};

template <typename T> requires AdoptableTokenParser<T>
struct Adopter : T {
  constexpr Adopter(T &&t) : T(std::move(t)) {
    static_assert(offsetof(Adopter, hdr) == 0, "hdr is not this");
    T::hdr.offset = 0;
  }

  inline operator TokenParserHdr&() { return T::hdr; }
};

struct String : Adopter<cmdline_parse_token_string_t>
{
  constexpr String(const char *s = nullptr) :
    Adopter {TOKEN_STRING_INITIALIZER(cmdline_parse_token_string_t, hdr, s)}
  {}

  using arg_type = cmdline_fixed_string_t;
  inline const arg_type &to_args(const void * const p) {
    return *reinterpret_cast<const arg_type*>(
        reinterpret_cast<const uint8_t*>(p) + hdr.offset);
  }
};

struct IpAddr : Adopter<cmdline_parse_token_ipaddr_t>
{
  constexpr IpAddr() :
    Adopter {TOKEN_IPADDR_INITIALIZER(cmdline_parse_token_ipaddr_t, hdr)}
  {}

  using arg_type = cmdline_ipaddr_t;
  inline const arg_type &to_args(const void * const p) {
    return *reinterpret_cast<const arg_type*>(
        reinterpret_cast<const uint8_t*>(p) + hdr.offset);
  }
};

} // namespace dpdk::cmdline::token
