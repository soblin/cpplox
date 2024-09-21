cmake_minimum_required(VERSION 3.12)

project("lox" VERSION 0.0.0)

enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost REQUIRED)

add_compile_options(-std=c++17 -Wall -g -O2 -flto)
add_link_options(-flto)

include_directories(include/ ${Boost_INCLUDE_DIRS})

add_library(${PROJECT_NAME}_lib SHARED src/tokenizer.cpp src/error.cpp
                                       src/expression.cpp src/parser.cpp)
target_link_libraries(${PROJECT_NAME}_lib readline)

add_executable(lox src/main.cpp)
target_link_libraries(lox ${PROJECT_NAME}_lib readline)

add_subdirectory(test)
