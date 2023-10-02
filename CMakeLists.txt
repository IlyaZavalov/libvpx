cmake_minimum_required(VERSION 3.25)
include(ExternalProject)
project(libvpx)

function(generate_option_args_list OUTPUT_VAR)
    foreach (COMP  ${CONFIGURE_OPTIONS_ENABLE})
        list(APPEND RESULT --enable-${COMP} )
    endforeach()
    foreach (COMP  ${CONFIGURE_OPTIONS_DISABLE})
        list(APPEND RESULT --disable-${COMP} )
    endforeach()
    if (CONFIGURE_OPTIONS_TARGET)
        list(APPEND RESULT --target=${CONFIGURE_OPTIONS_TARGET} )
    endif()
    if (CONFIGURE_OPTIONS_AS)
        list(APPEND RESULT --as=${CONFIGURE_OPTIONS_AS} )
    endif()
    set(${OUTPUT_VAR} ${RESULT} PARENT_SCOPE)
endfunction()

set(CONFIGURE_OPTIONS_ENABLE pic vp8-decoder vp9-decoder)
set(CONFIGURE_OPTIONS_DISABLE vp8-encoder vp9-encoder docs examples libyuv unit-tests tools)
set(CONFIGURE_OPTIONS_AS yasm)
set(PATH_TO_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/configure)

if (WIN32)
    set(COMMAND_PREFIX bash -l -c)
    list(APPEND CONFIGURE_OPTIONS_TARGET x86_64-win64-vs17)
    set(CONFIGURE_OPTIONS_AS nasm)
    cmake_path(RELATIVE_PATH PATH_TO_CONFIG BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()

if (NOT WIN32 AND BUILD_SHARED_LIBS)
    list(APPEND --enable-shared --disable-static)
endif()

generate_option_args_list(CONFIGURE_ARGS)
set(CONFIGURE_COMMAND ${PATH_TO_CONFIG} ${CONFIGURE_ARGS})
#string(APPEND CONFIGURE_COMMAND " " ${CONFIGURE_ARGS})

if(NOT WIN32)
    set(INSTALL_COMMAND make install)
endif()

ExternalProject_Add (libvpx
        PREFIX ${PROJECT_BINARY_DIR}
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
        BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}
        CONFIGURE_COMMAND ${COMMAND_PREFIX} ${CONFIGURE_COMMAND}
        BUILD_COMMAND ${COMMAND_PREFIX} make
        INSTALL_COMMAND ${COMMAND_PREFIX} ${INSTALL_COMMAND}
)

add_subdirectory(third_party/libyuv)
