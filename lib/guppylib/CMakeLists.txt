add_library(guppylib INTERFACE)

target_sources(guppylib INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/src/guppylib.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/pwm.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/canbus.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/gpio.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/timer.cpp
)

pico_enable_stdio_usb(guppylib 1)
pico_enable_stdio_uart(guppylib 0)

target_include_directories(guppylib INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

# Pull in pico libraries that we need
target_link_libraries(guppylib INTERFACE 
    pico_stdlib
    hardware_pwm
    can2040
    pico_neopixel
)