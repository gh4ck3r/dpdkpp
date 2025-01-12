#include <cmdline.hh>
#include "commands.h"

int main()
{
  dpdk::CLI cl{};

  cl.add<ObjDelShowCmd>(global_obj_map)
    .add<ObjAddCmd>(global_obj_map)
    .add<ListCmd>(global_obj_map)
    .add<HelpCmd>()
    .interact("example> ");

  return 0;
}
