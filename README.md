## Overview
- This is a very much work in progress project.
- The original plan was to port over ["MiBench"](https://github.com/embecosm/mibench) to Pancake so that we can have a comparison and analysis against GCC and CompCert using some known and established benchmarking suite.
- However, due to the complexity of the "Mibench" source code, and the limitations of the Pancake langauge, that plan was scrapped. 
- Instead, we’ll collect feasible benchmarks from existing suites or develop custom ones.

## Progress
- So far, we have only ported over the Dijkstra's algorithm benchmark from MiBench.

## How to run
```bash
mkdir build && cd build
cmake ..
make dijkstra-c dijkstra-pancake
```

To enable print:
```bash
cmake -DCMAKE_C_FLAGS="-DENABLE_PRINT" ..
make dijkstra-c dijkstra-pancake
```

Cross-compile for ARM64 Linux:
```bash
cmake -DCMAKE_TOOLCHAIN_FILE=../aarch64-linux-gnu.cmake ..
make dijkstra-c dijkstra-pancake
```

aarch64-linux-gnu.cmake:
```cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```

```bash
cd bin/network/dijkstra
./dijkstra-c input.dat
./dijkstra-pancake input.dat
```
