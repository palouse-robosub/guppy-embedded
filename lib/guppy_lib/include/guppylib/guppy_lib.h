#ifndef _GUPPY_LIB_H
#define _GUPPY_LIB_H

#include "pico/stdlib.h"
#include "guppylib/state.hpp"
#include "guppylib/ratelimit.hpp"

#define MS_BETWEEN_HEARTBEATS 1000

namespace guppylib
{

// returns if motors are allowed to run in the state
bool allowed_to_motor(State state);

}

#endif // _GUPPY_LIB_H