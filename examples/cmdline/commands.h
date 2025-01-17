#pragma once
#include <string_view>
#include <cmdline/parser.hh>
#include <cmdline/token.hh>
#include "parse_obj_list.h"

using dpdk::cmdline::CmdParser;
using TokenString = dpdk::cmdline::token::String;
using TokenIpAddr = dpdk::cmdline::token::IpAddr;

/**********************************************************/
class ObjDelShowCmd : public CmdParser<TokenString, TokenObjList>
{
  friend class CmdParser;
  ObjDelShowCmd() = delete;
  global_obj_map_t & data_store_;

 public:
  ObjDelShowCmd(global_obj_map_t &m) : CmdParser {
      "Show/del an object",
      {"show#del"},
      {m},
    }, data_store_(m)
  {}

  void on_parsed(const cmdline_fixed_string_t &, TokenObjList::arg_type &, struct cmdline &cl) override;
};

/**********************************************************/
struct ObjAddCmd : CmdParser<TokenString, TokenString, TokenIpAddr>
{
  void on_parsed(
      const cmdline_fixed_string_t &,
      const cmdline_fixed_string_t &,
      const cmdline_ipaddr_t &,
      struct cmdline &cl) override;

  ObjAddCmd(global_obj_map_t &m) :
    CmdParser {
      "Add an object (name, val)",
      {"add"},
      {},
      {},
    },
    data_store_(m)
  {}

 private:
  ObjAddCmd() = delete;
  global_obj_map_t & data_store_;
};

/**********************************************************/
struct HelpCmd : CmdParser<TokenString>
{
  void on_parsed(const cmdline_fixed_string_t&, struct cmdline &) override;

  HelpCmd() : CmdParser {"show help", {"help"}, }
  {
  }
};

class ListCmd : public CmdParser<TokenString>
{
  ListCmd() = delete;
  global_obj_map_t & data_store_;

 public:
  ListCmd(global_obj_map_t &m) :
    CmdParser {"list objects", {"list"}, },
    data_store_(m)
  {}

  void on_parsed(const cmdline_fixed_string_t &, struct cmdline &cl) override;
};
