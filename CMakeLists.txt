cmake_minimum_required(VERSION 3.15)
project(MiBench-Pancake VERSION 1.0.0 DESCRIPTION "MiBench Suite ported to Pancake Language")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_program(CAKE_COMPILER cake REQUIRED
    HINTS ENV{CAKE_COMPILER_PATH}
    DOC "Path to the Cake compiler for Pancake language")

if(NOT CAKE_COMPILER)
    message(FATAL_ERROR "Cake compiler not found. Please set CAKE_COMPILER_PATH environment variable.")
endif()

option(BUILD_PANCAKE "Build Pancake versions of benchmarks" ON)
option(BUILD_C_REFERENCE "Build C reference versions of benchmarks" ON)
option(BUILD_ALL_BENCHMARKS "Build all benchmark categories" OFF)

option(BUILD_AUTOMOTIVE "Build automotive benchmarks" ON)
option(BUILD_CONSUMER "Build consumer benchmarks" ON) 
option(BUILD_NETWORK "Build network benchmarks" ON)
option(BUILD_OFFICE "Build office benchmarks" ON)
option(BUILD_SECURITY "Build security benchmarks" ON)
option(BUILD_TELECOMM "Build telecommunications benchmarks" ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

include_directories(${CMAKE_SOURCE_DIR}/common/include)

if(NOT DEFINED PANCAKE_TARGET)
    # First check if we're cross-compiling and use the target system processor
    if(CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_PROCESSOR)
        set(TARGET_ARCH ${CMAKE_SYSTEM_PROCESSOR})
        message(STATUS "Cross-compiling detected, target architecture: ${TARGET_ARCH}")
    else()
        # Use host architecture
        set(TARGET_ARCH ${CMAKE_HOST_SYSTEM_PROCESSOR})
        message(STATUS "Native compilation, host architecture: ${TARGET_ARCH}")
    endif()
    
    # Map architecture to Pancake target
    if(TARGET_ARCH MATCHES "^(aarch64|arm64|ARM64)")
        set(PANCAKE_TARGET "arm8")
    elseif(TARGET_ARCH MATCHES "^(x86_64|amd64|AMD64)")
        set(PANCAKE_TARGET "x64")  
    elseif(TARGET_ARCH MATCHES "^(riscv64|RISCV64)")
        set(PANCAKE_TARGET "riscv")
    else()
        message(WARNING "Unknown architecture ${TARGET_ARCH}, defaulting to arm8")
        set(PANCAKE_TARGET "arm8")
    endif()
endif()

message(STATUS "Pancake target architecture: ${PANCAKE_TARGET}")

function(add_pancake_target target_name pancake_files output_file)
    set(CONCATENATED_FILE "${CMAKE_CURRENT_BINARY_DIR}/${target_name}_concatenated.🥞")
    
    set(ABSOLUTE_PANCAKE_FILES "")
    foreach(file ${pancake_files})
        if(NOT IS_ABSOLUTE ${file})
            set(file "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
        endif()
        list(APPEND ABSOLUTE_PANCAKE_FILES ${file})
    endforeach()
    
    add_custom_command(
        OUTPUT ${CONCATENATED_FILE}
        COMMAND cat ${ABSOLUTE_PANCAKE_FILES} > ${CONCATENATED_FILE}
        DEPENDS ${ABSOLUTE_PANCAKE_FILES}
        COMMENT "Concatenating Pancake files for ${target_name}"
    )
    
    add_custom_command(
        OUTPUT ${output_file}
        COMMAND cat ${CONCATENATED_FILE} | cpp -P ${CMAKE_C_FLAGS} | ${CAKE_COMPILER} --target=${PANCAKE_TARGET} --pancake --main_return=true > ${output_file}
        DEPENDS ${CONCATENATED_FILE}
        COMMENT "Compiling Pancake code for ${target_name} (target: ${PANCAKE_TARGET})"
    )
    
    add_custom_target(${target_name}_pancake_compile DEPENDS ${output_file})
endfunction()

function(add_benchmark_executable target_name)
    set(options PANCAKE_ENABLED)
    set(oneValueArgs)
    set(multiValueArgs C_SOURCES PANCAKE_SOURCES INCLUDE_DIRS LIBRARIES)
    cmake_parse_arguments(BENCHMARK "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
    get_filename_component(CATEGORY_NAME ${PARENT_DIR} NAME)
    set(BIN_SUBDIR "${CMAKE_BINARY_DIR}/bin/${CATEGORY_NAME}/${CURRENT_DIR_NAME}")
    
    if(BUILD_C_REFERENCE AND BENCHMARK_C_SOURCES)
        add_executable(${target_name}-c ${BENCHMARK_C_SOURCES})
        if(BENCHMARK_INCLUDE_DIRS)
            target_include_directories(${target_name}-c PRIVATE ${BENCHMARK_INCLUDE_DIRS})
        endif()
        if(BENCHMARK_LIBRARIES)
            target_link_libraries(${target_name}-c ${BENCHMARK_LIBRARIES})
        endif()
        set_target_properties(${target_name}-c PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${BIN_SUBDIR}
        )
    endif()
    
    if(BUILD_PANCAKE AND BENCHMARK_PANCAKE_ENABLED AND BENCHMARK_PANCAKE_SOURCES)
        set(PANCAKE_ASM "${CMAKE_CURRENT_BINARY_DIR}/${target_name}_pancake.S")
        add_pancake_target(${target_name} "${BENCHMARK_PANCAKE_SOURCES}" ${PANCAKE_ASM})
        
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${target_name}_pancake.o
            COMMAND ${CMAKE_C_COMPILER} -c -mcpu=cortex-a53 ${PANCAKE_ASM} -o ${CMAKE_CURRENT_BINARY_DIR}/${target_name}_pancake.o
            DEPENDS ${PANCAKE_ASM}
            COMMENT "Assembling Pancake code for ${target_name}"
        )
        
        set(BOOTSTRAP_SOURCES ${BENCHMARK_C_SOURCES})
        list(FILTER BOOTSTRAP_SOURCES EXCLUDE REGEX ".*\\.🥞$")
        
        set(FFI_FILE "${CMAKE_CURRENT_SOURCE_DIR}/pancake_ffi.c")
        if(NOT EXISTS ${FFI_FILE})
            message(FATAL_ERROR "pancake_ffi.c not found in ${CMAKE_CURRENT_SOURCE_DIR}")
        endif()
        
        add_executable(${target_name}-pancake 
            ${BOOTSTRAP_SOURCES} 
            ${CMAKE_CURRENT_BINARY_DIR}/${target_name}_pancake.o
            ${FFI_FILE}
        )
        
        target_compile_definitions(${target_name}-pancake PRIVATE PANCAKE_VERSION=1)
        add_dependencies(${target_name}-pancake ${target_name}_pancake_compile)
        
        if(BENCHMARK_INCLUDE_DIRS)
            target_include_directories(${target_name}-pancake PRIVATE ${BENCHMARK_INCLUDE_DIRS})
        endif()
        if(BENCHMARK_LIBRARIES)
            target_link_libraries(${target_name}-pancake ${BENCHMARK_LIBRARIES})
        endif()
        set_target_properties(${target_name}-pancake PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${BIN_SUBDIR}
        )
    endif()
endfunction()


add_custom_target(all-c)
add_custom_target(all-pancake)
if(BUILD_AUTOMOTIVE OR BUILD_ALL_BENCHMARKS)
    add_subdirectory(automotive)
endif()

if(BUILD_CONSUMER OR BUILD_ALL_BENCHMARKS)
    add_subdirectory(consumer)
endif()

if(BUILD_NETWORK OR BUILD_ALL_BENCHMARKS)
    add_subdirectory(network)
endif()

if(BUILD_OFFICE OR BUILD_ALL_BENCHMARKS)
    add_subdirectory(office)
endif()

if(BUILD_SECURITY OR BUILD_ALL_BENCHMARKS)
    add_subdirectory(security)
endif()

if(BUILD_TELECOMM OR BUILD_ALL_BENCHMARKS)
    add_subdirectory(telecomm)
endif()

message(STATUS "MiBench-Pancake Configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Cake compiler: ${CAKE_COMPILER}")
message(STATUS "  Build Pancake versions: ${BUILD_PANCAKE}")
message(STATUS "  Build C reference versions: ${BUILD_C_REFERENCE}")
message(STATUS "  Categories enabled:")
message(STATUS "    Automotive: ${BUILD_AUTOMOTIVE}")
message(STATUS "    Consumer: ${BUILD_CONSUMER}")
message(STATUS "    Network: ${BUILD_NETWORK}")
message(STATUS "    Office: ${BUILD_OFFICE}")
message(STATUS "    Security: ${BUILD_SECURITY}")
message(STATUS "    Telecommunications: ${BUILD_TELECOMM}")
