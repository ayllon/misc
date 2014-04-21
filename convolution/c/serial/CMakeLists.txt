cmake_minimum_required (VERSION 2.6)

file (GLOB src_serial "*.cpp")
add_executable (convolution_serial ${src_serial})
target_link_libraries (convolution_serial bootstrap)
