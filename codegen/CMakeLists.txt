cmake_minimum_required(VERSION 3.13)
project(guppy_codegen)

include(FetchContent)
# Getting tinyXML
FetchContent_Declare(
    tinyxml2
    GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
    GIT_TAG 9148bdf719e997d1f474be6bcc7943881046dba1
)
FetchContent_MakeAvailable(tinyxml2)

set(GUPPY_CAN ${CMAKE_BINARY_DIR}/generated/GuppyCAN.hpp)

configure_file(
    ${CMAKE_SOURCE_DIR}/can-ids.xml
    ${CMAKE_BINARY_DIR}/can-ids.xml
    COPYONLY
)

add_custom_command(
    OUTPUT ${GUPPY_CAN}
    COMMAND can_generator
    DEPENDS can_generator # codegen
    COMMENT "Generating GuppyCAN.hpp"
)
add_custom_target(run_can_generator ALL DEPENDS ${GUPPY_CAN})

add_executable(can_generator can_generator.cpp)
target_link_libraries(can_generator tinyxml2)



