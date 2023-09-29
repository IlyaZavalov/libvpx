cmake_minimum_required(VERSION 3.25)
include(ExternalProject)
project(libvpx)

set(CONFIGURE_OPTIONS --enable-pic --enable-vp8-decoder --enable-vp9-decoder --disable-vp8-encoder --disable-vp9-encoder --disable-docs --disable-examples --disable-libyuv --disable-unit-tests --disable-tools --as=nasm)
set(PATH_TO_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/configure)

if (WIN32)
    set(COMMAND_PREFIX wsl --exec)
    list(APPEND CONFIGURE_OPTIONS --target=x86_64-win64-vs17)
    cmake_path(RELATIVE_PATH PATH_TO_CONFIG BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()

if (NOT WIN32 AND BUILD_SHARED_LIBS)
    list(APPEND CONFIGURE_OPTIONS --enable-shared --disable-static)
endif()

ExternalProject_Add (vpx
        PREFIX ${PROJECT_BINARY_DIR}
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
        BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}
        CONFIGURE_COMMAND ${COMMAND_PREFIX} bash ${PATH_TO_CONFIG} ${CONFIGURE_OPTIONS}
        BUILD_COMMAND ${COMMAND_PREFIX} make
        INSTALL_COMMAND ""
)

add_subdirectory(third_party/libyuv)
