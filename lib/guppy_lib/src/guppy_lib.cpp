#include "hardware/pwm.h"
#include <string.h>

extern "C"
{
#include "can2040.h"
}

#include "guppylib/guppy_lib.h"
#include "guppylib/canbus.hpp"
#include "guppylib/core.hpp"
#include "guppylib/gpio.hpp"

namespace guppylib
{

bool allowed_to_motor(State state)
{
    return state == State::Holding
        || state == State::Nav
        || state == State::Task
        || state == State::Teleop;
}
}