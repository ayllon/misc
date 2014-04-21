cmake_minimum_required (VERSION 2.6)

include_directories (/usr/include/GraphicsMagick/)

file (GLOB src_bootstrap "*.cpp")
add_library (bootstrap STATIC ${src_bootstrap})
target_link_libraries (bootstrap GraphicsMagick++ rt)
