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

# ---- User-defined sketches ----

function(add_user_sketch)
  add_custom_target(${ARGV1})
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
