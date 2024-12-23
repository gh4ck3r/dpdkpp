#pragma once
#include <algorithm>
#include <utility>
#include <stdexcept>
#include <rte_lcore.h>

namespace dpdk::lcore {

using id_t = decltype(rte_lcore_id());

inline auto id() { return rte_lcore_id(); }

/// Lcore stands for thread, pinned to specific CPU core, in DPDK.
class Lcore {
 public:
	class Iterator;
	class Sentinel {};

	Lcore(id_t id) : id_(id) {
		if (id_ >= RTE_MAX_LCORE) {
			throw std::out_of_range("lcore id out of range: " + std::to_string(id_));
		}
	}
	Lcore() = delete;
#if 0 // XXX
	Lcore(const Lcore &) = delete;
#endif

	inline auto index() const     	{ return rte_lcore_index(id_); }
	inline auto socket_id() const 	{ return rte_lcore_to_socket_id(id_); }
	inline auto cpu_id() const    	{ return rte_lcore_to_cpu_id(id_); }
	inline auto cpu_set() const 		{ return rte_lcore_cpuset(id_); }
	inline bool is_enabled() const 	{ return rte_lcore_is_enabled(id_); }

 private:
	id_t id_;

	friend inline auto launch(Lcore lcore, auto &&fn, void *arg) {
		return rte_eal_remote_launch(std::forward<decltype(fn)>(fn), arg, lcore.id_);
	}
};

class Lcore::Iterator {
 public:
	using difference_type = int; // XXX
	using value_type = Lcore;

	Iterator(id_t id) : id_(id) {}
	inline Iterator &operator++() {
		id_ = rte_get_next_lcore(id_, 0, 0);	// all lcores including main
		return *this;
	}
	inline Iterator operator++(int) {
		auto tmp = *this;
		++*this;
		return tmp;
	}
	bool operator!=(const Iterator &rhs) const { return id_ != rhs.id_; }
	value_type operator*() const { return id_; }

 private:
	friend bool operator==(const Iterator &lhs, const Sentinel &) {
		return lhs.id_ == RTE_MAX_LCORE;
	}
	id_t id_;

};

class Enabled : public std::ranges::view_interface<Enabled>
{
 public:
	Enabled() = default;
	Enabled(Lcore::Sentinel) {}

	Lcore::Iterator begin() const { return rte_get_next_lcore(-1, 0, 0); }
	Lcore::Sentinel end() const { return {}; }
	inline size_t size() { return std::ranges::distance(begin(), end()); }
};

class Workers : public std::ranges::view_interface<Workers>
{
 public:
	Workers() = default;
	Workers(Lcore::Sentinel) {}

	Lcore::Iterator begin() const { return rte_get_next_lcore(-1, 1, 0); }
	Lcore::Sentinel end() const { return {}; }
	inline size_t size() { return std::ranges::distance(begin(), end()); }

};

inline auto for_each(auto &&view, auto &&fn) {
	return std::ranges::for_each(
			std::forward<decltype(view)>(view),
			std::forward<decltype(fn)>(fn)
		);
}

} // namespace dpdk::lcore
