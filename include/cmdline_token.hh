#pragma once
#include <concepts>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <cmdline_parse_string.h>
#include <cmdline_parse_ipaddr.h>

namespace dpdk::cmdline::token {

template <typename BaseT>
struct PolyStore : public std::vector<BaseT*> {
  using element_type = BaseT;
  using pointer = element_type*;
  using container_type = std::vector<pointer>;
  PolyStore() : container_type({nullptr}) {}
  virtual ~PolyStore() noexcept {
    for (const auto &p : *this) delete p;
  }

	inline operator pointer() const { return get(); }
  inline pointer const *get() const { return container_type::data(); }

  template <typename T, typename...ARGS>
    requires std::constructible_from<T, ARGS...>
         and std::convertible_to<T, element_type>
  inline PolyStore &append(ARGS&&...args) {
    auto p = std::make_unique<T>(std::forward<ARGS>(args)...);
    if (!p) throw std::runtime_error {
      std::string{"failed to allocate PolyStore object: "} + typeid(T).name()};

    try {
      container_type::emplace_back(std::exchange(
            container_type::back(),
            &static_cast<element_type&>(*p.get())));
      p.release();
    } catch(...) {
      throw std::runtime_error {
        std::string{"failed to append PolyStore object: "} + typeid(T).name()};
    }

    return *this;
  }
};

#if 0
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
#endif

using ParserHdr = cmdline_parse_token_hdr_t;
using TokenParsers = PolyStore<ParserHdr>;

template <typename CRTP>	// TODO add requires TokenImpl
class Parser : public ParserHdr {
  inline static constexpr CRTP &cast(ParserHdr *&THIS) {
    return *static_cast<CRTP*>(THIS);
  }
  inline static constexpr cmdline_token_ops ops_ {
    .parse = [] (ParserHdr *tk, const char *buf, void *res, unsigned size) {
      return cast(tk).parse(buf, res, size);
    },
    .complete_get_nb = [] (ParserHdr *tk) {
      return cast(tk).complete_get_nb();
    },
    .complete_get_elt = [] (ParserHdr *tk, int idx, char *dstbuf, unsigned int size) {
      return cast(tk).complete_get_elt(idx, dstbuf, size);
    },
    .get_help = [] (ParserHdr *tk, char *dstbuf, unsigned int size) {
      return cast(tk).get_help(dstbuf, size);
    },
  };

  protected:
  // TODO
  //Parser(const Parser &) = delete;
  //Parser(Parser &&) = delete;
  constexpr Parser(unsigned int offset) :
    ParserHdr {
      .ops = const_cast<decltype(ParserHdr::ops)>(&ops_),
      .offset = offset,
    }
  {}

  private:
};

template <typename T>
concept AdoptableTokenParser = requires (T t) {
  requires std::same_as<decltype(T::hdr), ParserHdr> and offsetof(T, hdr) == 0;
};

template <typename T> requires AdoptableTokenParser<T>
struct Adopter : T {
  constexpr Adopter(T &&t, unsigned int offset) : T(std::move(t)) {
    static_assert(offsetof(Adopter, hdr) == 0, "hdr is not this");
    T::hdr.offset = offset;
  }

  inline operator ParserHdr&() { return T::hdr; }
};

struct String : Adopter<cmdline_token_string>
{
  constexpr String(unsigned int offset, const char *s = nullptr) :
    Adopter {TOKEN_STRING_INITIALIZER(cmdline_token_string, hdr, s), offset}
  {}
};

struct IpAddr : Adopter<cmdline_parse_token_ipaddr_t>
{
  constexpr IpAddr(unsigned int offset) :
    Adopter {TOKEN_IPADDR_INITIALIZER(cmdline_parse_token_ipaddr_t, hdr), offset}
  {}
};

} // namespace dpdk::cmdline::token
