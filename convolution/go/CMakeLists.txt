cmake_minimum_required (VERSION 2.6)

file (GLOB_RECURSE src_convolution_go convolution/ "*.go")

add_custom_command (OUTPUT convolution
                    COMMAND go build -v -gcflags '-m' -o ${CMAKE_CURRENT_BINARY_DIR}/convolution
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/convolution
                    DEPENDS ${src_convolution_go}
                    )

add_custom_target (convolution_go ALL DEPENDS convolution)
