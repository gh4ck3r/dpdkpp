#pragma once
#include <string_view>
#include <cmdline_parser.hh>
#include <cmdline_token.hh>
#include "parse_obj_list.h"

/**********************************************************/
struct ObjDelShowCmd : dpdk::cmdline::Parser<ObjDelShowCmd>
{
  using CMD_T = dpdk::cmdline::token::Command<"show#del", TokenObjList>;
  struct data_t {
    cmdline_fixed_string_t action;
    struct object *obj;
  };

  static constexpr inline std::string_view helpmsg {"Show/del an object"};
  static constexpr std::tuple token_parsers {
    dpdk::cmdline::token::String {offsetof(data_t, action), "show#del"},
    TokenObjList{offsetof(data_t, obj)}
  };

  void on_parsed(const data_t &d, struct cmdline &cl);
  ObjDelShowCmd(global_obj_map_t &m) : data_store_(m) {}

 private:
  ObjDelShowCmd() = delete;
  global_obj_map_t & data_store_;
};

/**********************************************************/
struct ObjAddCmd : dpdk::cmdline::Parser<ObjAddCmd>
{
  using CMD_T = dpdk::cmdline::token::Command<"add", std::string_view, cmdline_parse_token_ipaddr_t>;
  struct data_t {
    cmdline_fixed_string_t action;
    cmdline_fixed_string_t name;
    cmdline_ipaddr_t ip;
  };

  static constexpr inline std::string_view helpmsg {"Add an object (name, val)"};
  static constexpr std::tuple token_parsers {
    dpdk::cmdline::token::String{offsetof(data_t, action), "add"},
    dpdk::cmdline::token::String{offsetof(data_t, name)},
    dpdk::cmdline::token::IpAddr{offsetof(data_t, ip)},
  };

  void on_parsed(const data_t &d, struct cmdline &cl);
  ObjAddCmd(global_obj_map_t &m) : data_store_(m) {}

 private:
  ObjAddCmd() = delete;
  global_obj_map_t & data_store_;
};

/**********************************************************/
struct HelpCmd :
  dpdk::cmdline::Parser<HelpCmd>,
  dpdk::cmdline::token::Command<"help">
{
  struct data_t {
    const cmdline_fixed_string_t help;
  };

  static constexpr inline std::string_view helpmsg {"show help"};
  static constexpr std::tuple token_parsers {
    dpdk::cmdline::token::String{offsetof(data_t, help), "help"},
  };

  void on_parsed(const data_t &, struct cmdline &cl);
};

class ListCmd :
  public dpdk::cmdline::Parser<ListCmd>,
  public dpdk::cmdline::token::Command<"list">
{
  ListCmd() = delete;
  global_obj_map_t & data_store_;

 public:
  ListCmd(global_obj_map_t &m) : data_store_(m) {}

  struct data_t {
    const cmdline_fixed_string_t list;
  };

  static constexpr inline std::string_view helpmsg {"list objects"};
  static constexpr std::tuple token_parsers {
    dpdk::cmdline::token::String{offsetof(data_t, list), "list"},
  };
  using tokens_t = std::tuple<std::string_view>;

  void on_parsed(const data_t &, struct cmdline &cl);
};
