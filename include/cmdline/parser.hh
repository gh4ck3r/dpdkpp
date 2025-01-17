#pragma once
#include <algorithm>
#include <array>
#include <concepts>
#include <type_traits>
#include <cmdline/token.hh>
#include <cmdline.h>

namespace dpdk::cmdline::command {

class Context {
  using ctx_t = cmdline_parse_inst_t;
  Context(size_t n_tokens = 0);

 public:
  virtual ~Context() noexcept;
  inline auto get() const { return ctx_; }

 protected:
  template <typename...TOKENS>  // requires TokenImpl
  constexpr Context(const ctx_t &ctx, TOKENS&&...tokens) :
    Context(sizeof...(TOKENS)) {
      *ctx_ = ctx;
      (token_parsers_.append<TOKENS>(std::forward<TOKENS>(tokens)), ...);

      const std::array size_arr {sizeof(typename TOKENS::arg_type)...};
      auto iter = token_parsers_.begin();
      std::size_t offset =0;
      for (const auto &s : size_arr) {
        (*iter)->offset = offset;
        offset += s;  // TODO consider alignment
        iter++;
      }

      if (CMDLINE_PARSE_RESULT_BUFSIZE < offset) {
        throw std::invalid_argument {"insufficient buffer for given tokens for CmdParser"};
      }

      std::copy_n(token_parsers_.get(), sizeof...(TOKENS) + 1, ctx_->tokens);
    }
  Context(const Context&) = delete;
  Context(Context&&) = delete;

  template <size_t N, typename T>
  decltype(auto) get_arg(const void * const p) {
    return (reinterpret_cast<T*>(token_parsers_.at(N))->to_args(p));
  }

 private:
  using pointer = cmdline_parse_ctx_t;
  static_assert(std::is_pointer_v<pointer>
            and std::is_same_v<std::add_pointer_t<ctx_t>, pointer>);
	pointer const ctx_;
  token::TokenParsers token_parsers_;
};

template <typename T>
concept Impl = requires(T t, decltype(cmdline_inst::help_str) msg) {
  requires std::derived_from<T, Context>;
  // TODO check on_parsed is available
};

template <typename...TOKENS>
class CmdParser : public Context {
  template <typename T>
  using add_cref_t = std::add_lvalue_reference_t<std::add_const_t<T>>;
  virtual void on_parsed(
      const add_cref_t<typename TOKENS::arg_type>...,
      struct cmdline &) = 0;

  template <size_t...N>
  inline void on_parsed(
      std::integer_sequence<size_t, N...>,
      void *result,
      struct cmdline *cl)
  {
    on_parsed(get_arg<N, TOKENS>(result)..., *cl);
  }

  static inline void on_parsed(void *result, struct cmdline *cl, void *THIS) {
    static constexpr std::make_index_sequence<sizeof...(TOKENS)> tkn_idx{};
    reinterpret_cast<CmdParser*>(THIS)->on_parsed(tkn_idx, result, cl);
  }

 public:
  constexpr CmdParser(const std::string_view &helpmsg, TOKENS&&...tokens) : Context({
      .f = on_parsed,
      .data = this,
      .help_str = helpmsg.data(),
		}, std::forward<TOKENS>(tokens)...)
  {
    if (*helpmsg.end() != 0x00) {
      throw std::invalid_argument{"helpmsg requires to be null ended string"};
    }
	}
  virtual ~CmdParser() noexcept {}
};

} // namespace dpdk::cmdline::command

namespace dpdk::cmdline {
using cmdline::command::CmdParser;
} // namespace dpdk::cmdline
