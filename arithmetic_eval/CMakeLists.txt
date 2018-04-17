cmake_minimum_required(VERSION 3.10)
project(arithmetic_eval)

set(CMAKE_CXX_STANDARD 11)

find_package(Boost REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

add_library(arithmetic_eval SHARED
        ArithmeticEval/Evaluator.cpp
    ArithmeticEval/Functions.cpp
)

add_executable(main main.cpp)

target_link_libraries(main
    arithmetic_eval
    ${Boost_LIBRARIES}
)
