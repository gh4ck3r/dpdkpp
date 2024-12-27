project(basicfwd)

set(TARGET dpdk-basicfwd)
add_executable(${TARGET} basicfwd.cc)

target_compile_definitions(${TARGET}
  PRIVATE
    ALLOW_EXPERIMENTAL_API
)

set_target_properties(${TARGET} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
