cmake_minimum_required (VERSION 2.6)

# Use C++11
add_definitions (-std=c++11)

# Bootstrap
add_subdirectory (bootstrap)

add_subdirectory (serial)
add_subdirectory (omp)
