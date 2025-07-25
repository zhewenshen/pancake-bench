add_benchmark_executable(quicksort
    PANCAKE_ENABLED
    C_SOURCES      quicksort.c
    PANCAKE_SOURCES quicksort.🥞
)

if (TARGET quicksort-c)
  add_dependencies(all-c quicksort-c)
endif()
if (TARGET quicksort-pancake)
  add_dependencies(all-pancake quicksort-pancake)
endif()