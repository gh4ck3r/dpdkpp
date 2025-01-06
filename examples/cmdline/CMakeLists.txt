project(cmdline)

set(TARGET dpdk-cmdline)
add_executable(${TARGET} main.cc commands.cc parse_obj_list.cc)

target_compile_definitions(${TARGET}
  PRIVATE
    ALLOW_EXPERIMENTAL_API
)

set_target_properties(${TARGET} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
