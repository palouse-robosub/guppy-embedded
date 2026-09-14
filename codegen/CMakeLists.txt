cmake_minimum_required(VERSION 3.13)

# Getting tinyXML
FetchContent_Declare(
        tinyxml2
        GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
        GIT_TAG 9148bdf719e997d1f474be6bcc7943881046dba1
)
FetchContent_MakeAvailable(tinyxml2)

# Running this on the host
find_program(NATIVE_CXX_COMPILER g++ REQUIRED)

include(ExternalProject)

set(GUPPY_CAN ${CMAKE_BINARY_DIR}/generated/GuppyCAN.hpp)

configure_file(
        ${CMAKE_SOURCE_DIR}/codegen/can-ids.xml
        ${CMAKE_BINARY_DIR}/codegen/can-ids.xml
        COPYONLY
)

add_custom_command(
        OUTPUT ${GUPPY_CAN}
        COMMAND can_generator
        DEPENDS codegen can_generator
        COMMENT "Generating GuppyCAN.hpp"
)
add_custom_target(run_can_generator DEPENDS ${GUPPY_CAN})

add_executable(can_generator codegen/can_generator.cpp)
target_link_libraries(can_generator tinyxml2)



