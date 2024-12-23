project(helloworld)

set(TARGET dpdk-helloworld)
add_executable(${TARGET} main.cc)

target_compile_options(${TARGET} PRIVATE -fconcepts-diagnostics-depth=3)

target_link_libraries(${TARGET}
  PRIVATE
    PkgConfig::DPDK
    eal
)

# set build executable to default binary directory
set_target_properties(${TARGET} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
