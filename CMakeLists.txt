cmake_minimum_required(VERSION 3.12)

project("lox" VERSION 0.0.0)

option(TRACE_COVERAGE "Measure Coverage" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost REQUIRED)

add_compile_options(-std=c++17 -Wall -g -O2 -flto)
if(TRACE_COVERAGE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
  set(CMAKE_EXE_LINKER_FLAGS
      "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

add_link_options(-flto)

include_directories(include/ ${Boost_INCLUDE_DIRS})

include(FetchContent)
FetchContent_Declare(
  magic-enum
  GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
  GIT_TAG v0.9.6)
FetchContent_MakeAvailable(magic-enum)

add_library(${PROJECT_NAME}_lib SHARED src/tokenizer.cpp src/expression.cpp
                                       src/parser.cpp src/interpreter.cpp)
target_link_libraries(${PROJECT_NAME}_lib readline magic_enum::magic_enum)

add_executable(lox src/main.cpp)
target_link_libraries(lox ${PROJECT_NAME}_lib readline magic_enum::magic_enum)

enable_testing()
add_subdirectory(test)
