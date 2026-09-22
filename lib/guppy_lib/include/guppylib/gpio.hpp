#ifndef GUPPY_EMBEDDED_GPIO_HPP
#define GUPPY_EMBEDDED_GPIO_HPP

#include <pico/stdlib.h>

namespace guppylib::gpio
{

    enum class Pull
    {
        PullDown,
        PullUp,
        Float
    };

    void init_input(uint32_t pin_num, Pull pull_resistor);
    void init_output(uint32_t pin_num);
    bool read(uint32_t pin_num);
}

#endif //GUPPY_EMBEDDED_GPIO_HPP