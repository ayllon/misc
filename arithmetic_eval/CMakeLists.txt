cmake_minimum_required(VERSION 3.10)
project(arithmetic_eval)

set(CMAKE_CXX_STANDARD 11)

find_package(Boost REQUIRED unit_test_framework)

include_directories(${Boost_INCLUDE_DIRS})

add_library(arithmetic_eval SHARED
    ArithmeticEval/Parser.cpp
    ArithmeticEval/Exception.cpp
)
target_link_libraries(arithmetic_eval
    ${Boost_LIBRARIES}
)

add_executable(generate_tree main.cpp)
target_link_libraries(generate_tree
    arithmetic_eval
)

add_executable(unit_test test.cpp)
target_link_libraries(unit_test
    arithmetic_eval
)
target_compile_definitions(unit_test
    PUBLIC -DBOOST_TEST_DYN_LINK -DBOOST_TEST_MAIN
)