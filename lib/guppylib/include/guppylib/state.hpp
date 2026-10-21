#ifndef GUPPY_EMBEDDED_STATE_HPP
#define GUPPY_EMBEDDED_STATE_HPP

namespace guppylib
{

enum class State : int
{
    Startup = 0,
    Holding = 1, 
    Nav = 2,
    Task = 3,
    Teleop = 4,
    Disabled = 5,
    Fault = 6
};

}

#endif //GUPPY_EMBEDDED_STATE_HPP