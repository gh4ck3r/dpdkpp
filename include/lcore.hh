#pragma once
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>
#include <rte_lcore.h>

namespace dpdk::lcore {

using id_t = decltype(rte_lcore_id());

inline auto id() { return rte_lcore_id(); }

/// Lcore stands for thread, pinned to specific CPU core, in DPDK.
class Lcore {
	id_t id_;

	friend class Manager;
	Lcore(id_t id) : id_(id) {
		if (id_ >= RTE_MAX_LCORE) {
			throw std::out_of_range("lcore id out of range: " + std::to_string(id_));
		}
	}
	Lcore() = delete;
	Lcore(const Lcore &) = delete;
	Lcore &operator=(const Lcore &) = delete;
	Lcore &operator=(Lcore &&) = delete;

 public:
	Lcore(Lcore &&) = default;
	inline auto index() const     	{ return rte_lcore_index(id_); }
	inline auto socket_id() const 	{ return rte_lcore_to_socket_id(id_); }
	inline auto cpu_id() const    	{ return rte_lcore_to_cpu_id(id_); }
	inline auto cpu_set() const 		{ return rte_lcore_cpuset(id_); }
	inline bool is_enabled() const 	{ return rte_lcore_is_enabled(id_); }

	inline auto launch(auto &&fn, void *arg) {
		return rte_eal_remote_launch(std::forward<decltype(fn)>(fn), arg, id_);
	}
};

class Manager {
 public:
	Manager() : main_{rte_get_main_lcore()}, workers_{} {
		id_t id;
		RTE_LCORE_FOREACH_WORKER(id) {
			workers_.emplace_back(Lcore{id});
		}
	}
	inline auto main() {
		return std::span<Lcore, 1>{std::addressof(main_), 1};
	}
	inline auto workers() {
		return std::span<Lcore>{workers_};
	}

 private:
	Lcore main_;
	std::vector<Lcore> workers_;
};

} // namespace dpdk::lcore
