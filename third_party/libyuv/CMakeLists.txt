cmake_minimum_required(VERSION 3.25)

file(GLOB_RECURSE SRCS source/*.cc)

add_library(yuv ${SRCS})
target_include_directories(yuv PUBLIC include)
