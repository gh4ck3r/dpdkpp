#pragma once
#include <algorithm>
#include <concepts>
#include <cmdline_token.hh>
#include <cmdline.h>

namespace dpdk::cmdline {

namespace command {

class Context {
  using ctx_t = cmdline_parse_inst_t;
  Context(size_t n_tokens = 0);

 public:
  virtual ~Context() noexcept;
  inline auto get() const { return ctx_; }

 protected:
  template <typename...ARGS>  // requires TokenImpl
  Context(const ctx_t &ctx, ARGS&&...args) :
    Context(sizeof...(ARGS)) {
      *ctx_ = ctx;
      (token_parsers_.append<ARGS>(std::forward<ARGS>(args)), ...);
      std::copy_n(token_parsers_.get(), sizeof...(ARGS) + 1, ctx_->tokens);
    }
  Context(const Context&) = delete;
  Context(Context&&) = delete;

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
  { msg = data(T::helpmsg) };
  requires std::is_class_v<typename T::data_t>;
  requires requires (T::data_t d, struct cmdline cl) {
    { t.on_parsed(d, cl) } -> std::same_as<void>;
  };
};

} // namespace command

template <typename CRTP>
class Parser : public command::Context {
 public:
  template <typename...ARGS>
  Parser(ARGS&&...args) : Context({
      .f = on_parsed,
      .data = this,
      .help_str = data(CRTP::helpmsg),
		}, std::forward<ARGS>(args)...)
  {
    static_assert(std::is_base_of_v<Parser, CRTP>, "use CRTP");
    static_assert(command::Impl<CRTP>);
	}
  virtual ~Parser() noexcept {}

 private:
  static void on_parsed(void *result, struct cmdline *cl, void *THIS) {
    static_assert(sizeof(typename CRTP::data_t) <= CMDLINE_PARSE_RESULT_BUFSIZE);
    reinterpret_cast<CRTP*>(THIS)->on_parsed(
        *reinterpret_cast<CRTP::data_t*>(result),
        *cl); // FIXME remove
  }
};

} // namespace dpdk::cmdline
