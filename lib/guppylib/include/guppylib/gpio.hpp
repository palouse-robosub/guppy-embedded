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

    /**
     * @brief Initializes a pin for GPIO with a pull resistor
     * 
     * @param pin_num pin to initialize
     * @param pull_resistor what type of pull, up, down, or float
     */
    void init_input(uint32_t pin_num, Pull pull_resistor);

    /**
     * @brief Initializes a pin for GPIO output
     * 
     * @param pin_num pin to initialize
     */
    void init_output(uint32_t pin_num);

    /**
     * @brief Does a digital read of a GPIO pin
     * 
     * @param pin_num pin to read. Must be initialized
     * @return boolean
     */
    bool read(uint32_t pin_num);
}

#endif //GUPPY_EMBEDDED_GPIO_HPP