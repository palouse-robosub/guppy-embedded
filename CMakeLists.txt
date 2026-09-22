cmake_minimum_required(VERSION 3.13)

# Pico SDK
set(PICO_PLATFORM rp2350)
include(pico_sdk_import.cmake)

project(guppy_embedded)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pico_sdk_init()

# gets the version from most recent tag, git describe command
find_package(Git REQUIRED)

set(GUPPY_EMBEDDED_VERSION "unknown")
# runs git command, strips whitespace, puts output into GUPPY_EMBEDDED_VERSION
execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE GUPPY_EMBEDDED_VERSION
)


# Add can2040 lib  TODO: don't use fetchcontent just add to vendor folder
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

# add libraries
add_subdirectory(vendor/pico_neopixels) # TODO: update guppylib to not depend on pico_neopixels
add_subdirectory(vendor/barometer)
add_subdirectory(lib/guppylib)



function(guppy_init_target TARGET_NAME)
    target_include_directories(${TARGET_NAME} PRIVATE include)

    #pico_set_program_version embeds a version string into compiled binary
    #can look at version with picotool info
    pico_set_program_version(${TARGET_NAME} "${GUPPY_EMBEDDED_VERSION}")

    pico_set_binary_type(${TARGET_NAME} copy_to_ram)
    pico_enable_stdio_usb(${TARGET_NAME} 1)
    pico_enable_stdio_uart(${TARGET_NAME} 0)
    pico_add_extra_outputs(${TARGET_NAME})

endfunction()

add_executable(board_motor src/board_motor.cpp)
target_link_libraries(board_motor guppylib)
guppy_init_target(board_motor)

add_executable(board_wet src/board_wet.cpp)
target_link_libraries(board_wet guppylib barometer)
guppy_init_target(board_wet)
