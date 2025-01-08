#pragma once
#include <concepts>
#include <tuple>
#include <utility>
#include <vector>
#include <cmdline_token.hh>
#include <cmdline.h>

namespace dpdk::cmdline {

template <typename BaseT>
struct PolyStore : private std::vector<BaseT> {
  using container_type = std::vector<BaseT>;
  PolyStore() : container_type({nullptr}) {}
  virtual ~PolyStore() noexcept {
    for (const auto &p : *this) delete p;
  }

	inline operator BaseT*() const { return get(); }
  inline BaseT *get() const { return container_type::data(); }

  template <typename T, typename...ARGS>
    requires std::derived_from<T, BaseT> and std::constructible_from<T, ARGS...>
  inline auto append(ARGS&&...args) {
    return container_type::emplace_back(
        std::exchange(container_type::back(),
          new T{std::forward<ARGS>(args)...}));
  }
};

namespace command {

class Context {
	cmdline_parse_ctx_t const ctx_;
  using ptr_t = decltype(ctx_);
  static_assert(std::is_pointer_v<ptr_t>);

 protected:
  Context(size_t n_tokens = 0);
  Context(const Context&) = delete;
  Context(Context&&) = delete;

  inline auto &initialize(const cmdline_parse_inst_t &arg) {
    *ctx_ = arg;
    return ctx_->tokens;
  }

 public:
  virtual ~Context() noexcept;
  inline auto get() const { return ctx_; }
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

template <typename T>
class Parser : public command::Context {
 public:
	Parser() : Context(std::tuple_size_v<decltype(T::token_parsers)>) {
    static_assert(std::is_base_of_v<Parser, T>, "use CRTP");
    static_assert(command::Impl<T>);

    auto &token_parsers = initialize({
      .f = on_parsed,
      .data = this,
      .help_str = data(T::helpmsg),
		});

    std::apply([&token_parsers] (auto&&...token) {
        constexpr auto as_hdr = [] (auto&&t) {
          using hdr_t = cmdline_parse_token_hdr_t;
          return const_cast<hdr_t*>(reinterpret_cast<const hdr_t*>(&t));
        };
        size_t i = 0;
        ((token_parsers[i++] = as_hdr(token)), ...), token_parsers[i] = nullptr;
      }, T::token_parsers);
	}
  virtual ~Parser() noexcept {}

 private:
  static void on_parsed(void *result, struct cmdline *cl, void *THIS) {
    static_assert(sizeof(typename T::data_t) <= CMDLINE_PARSE_RESULT_BUFSIZE);
    reinterpret_cast<T*>(THIS)->on_parsed(
        *reinterpret_cast<T::data_t*>(result),
        *cl);
  }
};

} // namespace dpdk::cmdline
