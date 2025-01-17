#pragma once
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace dpdk {

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
  inline T& append(ARGS&&...args) {
    auto p = std::make_unique<T>(std::forward<ARGS>(args)...);
    if (!p) throw std::runtime_error {
      std::string{"failed to allocate PolyStore object: "} + typeid(T).name()};

    try {
      auto &ptr = container_type::emplace_back(std::exchange(
            container_type::back(),
            &static_cast<element_type&>(*p.get())));
      p.release();
      return *reinterpret_cast<T*>(ptr);
    } catch(...) {
      throw std::runtime_error {
        std::string{"failed to append PolyStore object: "} + typeid(T).name()};
    }
  }
};

} // namespace dpdk
