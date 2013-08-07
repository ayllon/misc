cmake_minimum_required (VERSION 2.6)

add_definitions(-std=c++0x)

add_executable (test test.cpp)
target_link_libraries (test cppunit dl rt)
