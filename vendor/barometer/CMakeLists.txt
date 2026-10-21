add_library(barometer INTERFACE)

target_sources(barometer INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/barometer.cpp
)

pico_enable_stdio_usb(barometer 1)
pico_enable_stdio_uart(barometer 0)

target_include_directories(barometer INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

# Pull in pico libraries that we need
target_link_libraries(barometer INTERFACE 
    pico_stdlib
    hardware_i2c
)