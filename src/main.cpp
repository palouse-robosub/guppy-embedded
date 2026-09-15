// This is the main program going on the Guppy microcontrollers
//
// This code listens to id 0x101 for float values, echoes the can frame on id 0x102, and outputs analog output on pin 20
extern "C" {
#include "pico/stdlib.h"
}
#include "guppylib/guppy_lib.h"
#include <guppylib/canbus.hpp>
#include "modules/board_motor.h"
#include "modules/board_wet.h"
#include "guppylib/led.hpp"
#include <iostream>

using namespace guppylib;

int main()
{
    stdio_init_all();
    canbus::setup();

    switch(BOARD_TYPE)
    {
        case 1: // TODO: should be enum?
            board_motor_loop();
            break;
        case 2:
            board_wet_loop();
            break;
        case -1:
        default: // test
            break;
    }
}