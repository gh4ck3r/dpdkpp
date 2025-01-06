link_libraries(
    eal
  optimized
    PkgConfig::DPDK
  )

add_subdirectory(helloworld)
add_subdirectory(skeleton)
add_subdirectory(cmdline)
