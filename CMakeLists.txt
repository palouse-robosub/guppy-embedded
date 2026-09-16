cmake_minimum_required(VERSION 3.13)

# Pico SDK
set(PICO_PLATFORM rp2350)
include(pico_sdk_import.cmake)

project(guppy_embedded)
set(CMAKE_C_STANDARD 11)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pico_sdk_init()

# Add can2040 lib
include(FetchContent)

FetchContent_Declare(
    can2040
    GIT_REPOSITORY https://github.com/KevinOConnor/can2040.git
    GIT_TAG master
)
FetchContent_MakeAvailable(can2040)

add_library(can2040 STATIC
    ${can2040_SOURCE_DIR}/src/can2040.c
)

target_include_directories(can2040 PUBLIC
    ${can2040_SOURCE_DIR}/src
)

target_link_libraries(can2040
    hardware_pio
)

include(ExternalProject)
ExternalProject_Add(guppy_codegen
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/codegen
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/guppy-codegen
    CMAKE_ARGS
    -DCMAKE_C_COMPILER=cc
    -DCMAKE_CXX_COMPILER=c++
    -DCMAKE_TOOLCHAIN_FILE=
    BUILD_ALWAYS 1
    BUILD_BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/guppy-codegen/generated/GuppyCAN.hpp
    INSTALL_COMMAND ""
)
#include(codegen/CMakeLists.txt)
#add_subdirectory(codegen)
# TODO: move some stuff from can_generator.cpp into here so that we can depend on it??

# add libraries
add_subdirectory(lib/pico_neopixels)
add_subdirectory(lib/led)
add_subdirectory(lib/barometer)
add_subdirectory(lib/guppy_lib)

# add stuff from modules folder
file(GLOB MODULE_SOURCES
    src/modules/*.c
    src/modules/*.cpp
)

function(add_board BOARD_NAME BOARD_TYPE)

    add_executable(${BOARD_NAME}
        src/main.cpp
        ${MODULE_SOURCES}
        ${GUPPY_CAN}
    )
    target_include_directories(${BOARD_NAME} PRIVATE ${CMAKE_BINARY_DIR}/guppy-codegen/generated)
    add_dependencies(${BOARD_NAME} guppy_codegen)

    pico_set_binary_type(${BOARD_NAME} copy_to_ram)
    target_link_libraries(${BOARD_NAME}
        pico_stdlib
        can2040
        guppy_lib
        pico_neopixel
        led_strip
        barometer
    )

    target_compile_definitions(${BOARD_NAME} PRIVATE BOARD_TYPE=${BOARD_TYPE})
    
    pico_enable_stdio_usb(${BOARD_NAME} 1)
    pico_enable_stdio_uart(${BOARD_NAME} 0)

endfunction()

add_board(board_motor 1)
add_board(board_wet 2)
add_board(board_test -1)

pico_add_extra_outputs(board_motor)
pico_add_extra_outputs(board_wet)
pico_add_extra_outputs(board_test)