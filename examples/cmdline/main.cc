#include <cmdline.hh>
#include "commands.h"

int main()
{
  struct CLI : dpdk::CLI {
    global_obj_map_t global_obj_map;
    CLI() {
      add<ObjAddCmd>(global_obj_map);
      add<ObjDelShowCmd>(global_obj_map);
      add<ListCmd>(global_obj_map);
      add<HelpCmd>();
    }
  } cli;

  cli.interact("example> ");

  return 0;
}
