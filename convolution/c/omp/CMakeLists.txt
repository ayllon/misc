cmake_minimum_required (VERSION 2.6)

add_definitions (-fopenmp)

file (GLOB src_omp "*.cpp")
add_executable (convolution_omp ${src_omp})
target_link_libraries (convolution_omp bootstrap gomp)
