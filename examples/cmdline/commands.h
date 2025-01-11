#pragma once
#include <string_view>
#include <cmdline_parser.hh>
#include <cmdline_token.hh>
#include "parse_obj_list.h"

/**********************************************************/
class ObjDelShowCmd : public dpdk::cmdline::Parser<ObjDelShowCmd>
{
  friend class Parser;
  ObjDelShowCmd() = delete;
  global_obj_map_t & data_store_;

 public:
  struct data_t {
    cmdline_fixed_string_t action;
    struct object *obj;
  };

  using String = dpdk::cmdline::token::String;
  ObjDelShowCmd(global_obj_map_t &m) : Parser {
        String {offsetof(data_t, action), "show#del"},
        TokenObjList {offsetof(data_t, obj)},
      }, data_store_(m)
  {
  }

  static constexpr inline std::string_view helpmsg {"Show/del an object"};

  void on_parsed(const data_t &d, struct cmdline &cl);
};

/**********************************************************/
struct ObjAddCmd : dpdk::cmdline::Parser<ObjAddCmd>
{
  struct data_t {
    cmdline_fixed_string_t action;
    cmdline_fixed_string_t name;
    cmdline_ipaddr_t ip;
  };

  static constexpr inline std::string_view helpmsg {"Add an object (name, val)"};

  void on_parsed(const data_t &d, struct cmdline &cl);

  using String = dpdk::cmdline::token::String;
  using IpAddr = dpdk::cmdline::token::IpAddr;
  ObjAddCmd(global_obj_map_t &m) : Parser {
      String {offsetof(data_t, action), "add"},
      String {offsetof(data_t, name)},
      IpAddr{offsetof(data_t, ip)},
    },
    data_store_(m)
  {
  }

 private:
  ObjAddCmd() = delete;
  global_obj_map_t & data_store_;
};

/**********************************************************/
struct HelpCmd : dpdk::cmdline::Parser<HelpCmd>
{
  struct data_t {
    const cmdline_fixed_string_t help;
  };

  static constexpr inline std::string_view helpmsg {"show help"};

  void on_parsed(const data_t &, struct cmdline &cl);

  using String = dpdk::cmdline::token::String;
  HelpCmd() : Parser {
      String {offsetof(data_t, help), "help"},
    }
  {
  }
};

class ListCmd : public dpdk::cmdline::Parser<ListCmd>
{
  ListCmd() = delete;
  global_obj_map_t & data_store_;

  using String = dpdk::cmdline::token::String;
 public:
  ListCmd(global_obj_map_t &m) : Parser {
      String{offsetof(data_t, list), "list"},
    },
    data_store_(m)
  {
  }

  struct data_t {
    const cmdline_fixed_string_t list;
  };

  static constexpr inline std::string_view helpmsg {"list objects"};
  using tokens_t = std::tuple<std::string_view>;

  void on_parsed(const data_t &, struct cmdline &cl);

};
