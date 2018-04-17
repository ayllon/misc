cmake_minimum_required(VERSION 3.10)
project(arithmetic_eval)

set(CMAKE_CXX_STANDARD 11)

find_package(Boost REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

add_executable(arithmetic_eval main.cpp ArithmeticEvaluator.cpp)

target_link_libraries(arithmetic_eval ${Boost_LIBRARIES})