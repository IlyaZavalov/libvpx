cmake_minimum_required(VERSION 3.25)
include(ExternalProject)
project(libvpx)

set(CONFIGURE_OPTIONS --enable-pic --enable-vp8-decoder --enable-vp9-decoder --disable-vp8-encoder --disable-vp9-encoder --disable-docs --disable-examples --disable-libyuv --disable-unit-tests --disable-tools)


if (WIN32)
    set(COMMAND_PREFIX wsl --exec)
    list(APPEND CONFIGURE_OPTIONS --target=x86_64-win64-vs17)
endif()

if (BUILD_SHARED_LIBS)
    list(APPEND CONFIGURE_OPTIONS --enable-shared --disable-static)
endif()

configure_file(configure configure COPYONLY)

message(NOTICE ${CMAKE_CURRENT_SOURCE_DIR}/configure)
execute_process(COMMAND wslpath -a ${CMAKE_CURRENT_SOURCE_DIR}/configure RESULT_VARIABLE PATH_TO_CONFIG)

ExternalProject_Add (libvpx
        PREFIX ${PROJECT_BINARY_DIR}
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
        CONFIGURE_COMMAND ${COMMAND_PREFIX} bash ${PATH_TO_CONFIG} ${CONFIGURE_OPTIONS}
        BUILD_COMMAND ${COMMAND_PREFIX} make
        INSTALL_COMMAND ${COMMAND_PREFIX} make install
)

add_subdirectory(third_party/libyuv)
