#pragma once
#include <map>
#include <string>
#include <cmdline/token.hh>

#define OBJ_NAME_LEN_MAX sizeof(cmdline_fixed_string_t)

struct object {
  char name[OBJ_NAME_LEN_MAX];
  cmdline_ipaddr_t ip;
};

using global_obj_map_t = std::map<std::string, object>;

struct TokenObjList : dpdk::cmdline::token::TokenParser<TokenObjList> {
  using arg_type = object * const;
  inline const arg_type &to_args(const void * const p) {
    return *reinterpret_cast<const arg_type*>(
        reinterpret_cast<const uint8_t*>(p) + TokenParser::offset);
  }

  constexpr TokenObjList(global_obj_map_t &m) : TokenParser {}, data_store_(m) {}

 private:
  friend TokenParser;
  int parse(const char *buf, void *res, unsigned ressize);
  int complete_get_nb();
  int complete_get_elt(int idx, char *dstbuf, unsigned int size);
  int get_help(char *dstbuf, unsigned int size);

  global_obj_map_t &data_store_;
};
