add_benchmark_executable(dijkstra
    PANCAKE_ENABLED
    C_SOURCES dijkstra.c
    PANCAKE_SOURCES dijkstra.🥞
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/input.dat
    ${CMAKE_BINARY_DIR}/bin/network/dijkstra/input.dat
    COPYONLY
)
if(TARGET dijkstra-c)
    add_dependencies(all-c dijkstra-c)
endif()
if(TARGET dijkstra-pancake)
    add_dependencies(all-pancake dijkstra-pancake)
endif()

