add_benchmark_executable(mibench-dijkstra
    PANCAKE_ENABLED
    C_SOURCES      dijkstra.c
    PANCAKE_SOURCES dijkstra.🥞
)

get_filename_component(BENCH_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(OUT_DIR "${CMAKE_BINARY_DIR}/bin/${BENCH_NAME}")
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/input.dat
    ${OUT_DIR}/input.dat
    COPYONLY
)

if (TARGET mibench-dijkstra-c)
  add_dependencies(all-c mibench-dijkstra-c)
endif()
if (TARGET mibench-dijkstra-pancake)
  add_dependencies(all-pancake mibench-dijkstra-pancake)
endif()
