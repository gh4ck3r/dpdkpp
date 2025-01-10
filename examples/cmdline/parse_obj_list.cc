#include "parse_obj_list.h"
#include <string_view>
#include <ranges>

global_obj_map_t global_obj_map;

int TokenObjList::parse(const char *buf, void *res, unsigned ressize)
{
	if (*buf == 0) return -1;

	if (res && ressize < sizeof(object *)) return -1;

	unsigned int token_len = 0;
	while(!cmdline_isendoftoken(buf[token_len])) token_len++;

	for (const auto &[name, obj] : global_obj_map) {
		if (name.size() == token_len && name.compare(0, token_len, buf)) {
			if (res) *(const object **)res = &obj;	// XXX
			return token_len;
		}
	}
	return -ENOENT;
}

int TokenObjList::complete_get_nb()
{
	return global_obj_map.size();
}

int TokenObjList::complete_get_elt(int idx, char *dstbuf, unsigned int size)
{
	for (const auto &[name, _] : global_obj_map
			| std::views::drop(idx)
			| std::views::take(1))
	{
		if (name.size() + 1 > size) return -ENOBUFS;
		if (dstbuf) dstbuf[name.copy(dstbuf, size - 1)] = 0x00;
		break;
	}

	return 0;
}

int TokenObjList::get_help(char *dstbuf, unsigned int size)
{
	static constexpr std::string_view msg {"Obj-List"};
	dstbuf[msg.copy(dstbuf, size - 1)] = 0x00;

	return 0;
}

