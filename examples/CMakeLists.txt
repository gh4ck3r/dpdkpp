link_libraries(
    eal
  optimized
    PkgConfig::DPDK
  )

add_subdirectory(helloworld)
