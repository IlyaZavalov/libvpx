cmake_minimum_required(VERSION 3.25)
project(libvpx)

include(libvpx-config.cmake)

add_library(vpx INTERFACE)
target_link_libraries(vpx INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/libvpx${CMAKE_STATIC_LIBRARY_SUFFIX})
target_include_directories(vpx INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(third_party/libyuv)
