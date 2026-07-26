cmake_minimum_required(VERSION 3.12)

project("lox" VERSION 0.0.0)

option(TRACE_COVERAGE "Measure Coverage" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(USE_LTO "Use Link Time Optimization" ON)

add_compile_options(-std=c++17 -Wall -g -O2)
if(USE_LTO)
  add_compile_options(-flto)
  add_link_options(-flto)
endif()

if(TRACE_COVERAGE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
  set(CMAKE_EXE_LINKER_FLAGS
      "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

find_package(Boost REQUIRED COMPONENTS program_options)

find_package(magic_enum REQUIRED)

# targets
include_directories(include/)

add_library(
  ${PROJECT_NAME}_lib SHARED
  src/tokenizer.cpp
  src/expression.cpp
  src/parser.cpp
  src/interpreter.cpp
  src/debug.cpp
  src/environment.cpp
  src/resolver.cpp)
target_link_libraries(${PROJECT_NAME}_lib readline magic_enum::magic_enum)

add_executable(lox src/main.cpp)
target_link_libraries(lox ${PROJECT_NAME}_lib readline magic_enum::magic_enum
                      Boost::program_options)

enable_testing()
add_subdirectory(test)
