#include "commands.h"
#include <format>

static inline std::string to_string(const cmdline_ipaddr_t &ip)
{
/* not defined under linux */
#ifndef NIPQUAD
#define NIPQUAD(addr)				\
	(unsigned)((unsigned char *)&addr)[0],	\
	(unsigned)((unsigned char *)&addr)[1],	\
	(unsigned)((unsigned char *)&addr)[2],	\
	(unsigned)((unsigned char *)&addr)[3]
#endif

	return (ip.family == AF_INET) ?
		std::format("{:d}.{:d}.{:d}.{:d}", NIPQUAD(ip.addr.ipv4)) :
		std::format("{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:"
								"{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}",
								RTE_IPV6_ADDR_SPLIT(&ip.addr.ipv6));
}

void ObjDelShowCmd::on_parsed(const cmdline_fixed_string_t &action, TokenObjList::arg_type &obj, struct cmdline &cl)
{
	const auto ip_str = to_string(obj->ip);

	using std::operator""sv;
	if (action == "del"sv) {
		auto entry = data_store_.find(obj->name);
		if (entry != data_store_.end()) {
			cmdline_printf(&cl, "Object %s removed, ip=%s\n", obj->name, ip_str.c_str());
			data_store_.erase(entry);
		}
	} else if (action == "show"sv) {
		cmdline_printf(&cl, "Object %s, ip=%s\n", obj->name, ip_str.c_str() );
	}
}

void ObjAddCmd::on_parsed(const cmdline_fixed_string_t &,
      const cmdline_fixed_string_t &_name,
      const cmdline_ipaddr_t &ip,
      struct cmdline &cl)
{
	const std::string name {_name};
	if (data_store_.contains(name)) {
		cmdline_printf(&cl, "Object %s already exist\n", name.data());
		return;
	}

	object o {
		.name = {},
		.ip = ip,
	};
	name.copy(o.name, sizeof(o.name));
	data_store_[name] = std::move(o);

	const auto ip_str = to_string(ip);
	cmdline_printf(&cl, "Object %s added, ip=%s\n", name.c_str(), ip_str.c_str());
}

void HelpCmd::on_parsed(const cmdline_fixed_string_t&, struct cmdline &cl)
{
	cmdline_printf(&cl,
			"Demo example of command line interface in RTE\n\n"
			"This is a readline-like interface that can be used to\n"
			"debug your RTE application. It supports some features\n"
			"of GNU readline like completion, cut/paste, and some\n"
			"other special bindings.\n\n"
			"This demo shows how rte_cmdline library can be\n"
			"extended to handle a list of objects. There are\n"
			"3 commands:\n"
			"- add obj_name IP\n"
			"- del obj_name\n"
			"- show obj_name\n\n");
}

void ListCmd::on_parsed(const cmdline_fixed_string_t &, struct cmdline &cl)
{
	for (const auto &[name, obj] : data_store_) {
		cmdline_printf(&cl,
				"%s, %s, %s\n", name.c_str(), obj.name, to_string(obj.ip).c_str());
	}
}
