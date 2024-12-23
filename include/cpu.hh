#pragma once
#include <rte_lcore.h>
namespace dpdk::cpu {

inline auto socket_id() { return rte_socket_id(); }
inline auto socket_cnt() { return rte_socket_count(); }

} // namespace dpdk::cpu
