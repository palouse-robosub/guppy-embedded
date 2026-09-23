#ifndef GUPPY_BOARD_MOTOR_HPP
#define GUPPY_BOARD_MOTOR_HPP

#include <guppylib/state.hpp>

void board_motor_loop();
bool allowed_to_motor(guppylib::State state);

#endif // GUPPY_BOARD_MOTOR_HPP