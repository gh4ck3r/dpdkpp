#pragma once
#include <map>
#include <string>
#include <cmdline_token.hh>

#define OBJ_NAME_LEN_MAX sizeof(cmdline_fixed_string_t)

struct object {
  char name[OBJ_NAME_LEN_MAX];
  cmdline_ipaddr_t ip;
};

struct TokenObjList : dpdk::cmdline::token::Parser<TokenObjList> {
  constexpr TokenObjList(unsigned int offset) : Parser(offset) {}

 private:
  friend class Parser;
  int parse(const char *buf, void *res, unsigned ressize);
  int complete_get_nb();
  int complete_get_elt(int idx, char *dstbuf, unsigned int size);
  int get_help(char *dstbuf, unsigned int size);
};

using global_obj_map_t = std::map<std::string, object>;
extern global_obj_map_t global_obj_map;
