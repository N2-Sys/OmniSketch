# ---- CMake requirements ----

cmake_minimum_required(VERSION 3.11)

set(TOP_PROJECT_NAME "OmniSketch")
project(${TOP_PROJECT_NAME})

# ---- CXX standard ----

set(CMAKE_CXX_STANDARD 17)
# To disable warnings in common/toml.h on a few c++20 compiler flags
set(CMAKE_CXX_FLAGS "-Wno-unknown-warning-option -Wno-deprecated-declarations")

# ---- Python Components ----

find_package(Python COMPONENTS Interpreter)

# ---- Include testing ----

include(CTest)

# ---- Include path of header files ----

include_directories(src)

# ---- Submodules ----

add_subdirectory(third_party/eigen)
add_subdirectory(third_party/fmt)
add_subdirectory(third_party/toml)
# Configuring toml & eigen needs a bit more work
include_directories(third_party/toml/include)
include_directories(third_party/eigen)

# ---- Boost libraries ----

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# ---- Compile static libraries ----

add_library(OmniTools src/impl/utils.cpp src/impl/logger.cpp src/impl/data.cpp src/impl/test.cpp src/impl/hash.cpp)
target_link_libraries(OmniTools fmt)

# ---- Add testing ----

add_subdirectory(test)

# ---- Pcap Library ----

function(check_library)
  find_library(${ARGV0} ${ARGV1})
  if(NOT ${ARGV0})
    message(FATAL_ERROR "lib ${ARGV1} not found")
  else()
    message(STATUS "lib ${ARGV1} found at ${${ARGV0}}")
  endif()
endfunction(check_library)

if(BUILD_PCAP_PARSER)
  check_library(PCPP Pcap++)
  check_library(PACPP Packet++)
  check_library(COMPP Common++)
  check_library(PCAP pcap)
endif()

# ---- Pcap Parser ----

if(NOT PCPP_INCLUDE_PATH)
  set(PCPP_INCLUDE_PATH /usr/local/include/pcapplusplus)
  message(STATUS "pcapplusplus is included at ${PCPP_INCLUDE_PATH} by default")
else()
  message(STATUS "pcapplusplus is included at ${PCPP_INCLUDE_PATH}")
endif()

if(BUILD_PCAP_PARSER)
  add_executable(parser ${CMAKE_CURRENT_SOURCE_DIR}/src/pcap_parser/parser.cpp)
  target_include_directories(parser PRIVATE ${PCPP_INCLUDE_PATH})
  target_link_libraries(parser ${PCPP} ${PACPP} ${COMPP} ${PCAP} fmt OmniTools)
endif()

# ---- User-defined sketches ----

function(add_user_sketch)
  add_custom_target(${ARGV1}
  DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/driver/${ARGV1}Driver.cpp
  )
  add_custom_command(
    OUTPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/src/driver/${ARGV1}Driver.cpp
    COMMAND
        ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/src/generate_driver.py sketch_test/${ARGV1}Test.h
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/sketch_test/${ARGV1}Test.h
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/src
  )
  
  add_executable(${ARGV0} ${CMAKE_CURRENT_SOURCE_DIR}/src/driver/${ARGV1}Driver.cpp)
  target_link_libraries(${ARGV0} OmniTools)
endfunction(add_user_sketch)

# Bloom Filter
add_user_sketch(BF BloomFilter)

# Count Min Sketch
add_user_sketch(CM CMSketch)

# CH-optimized Count Min Sketch
add_user_sketch(CHCM CHCMSketch)

# CU Sketch
add_user_sketch(CU CUSketch)

# HashPipe
add_user_sketch(HP HashPipe)