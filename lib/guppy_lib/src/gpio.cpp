#include "guppylib/gpio.hpp"

namespace guppylib::gpio
{

void init_input(uint32_t pin_num, Pull pull_resistor)
{
    gpio_init(pin_num);
    gpio_set_dir(pin_num, false);
    switch (pull_resistor)
    {
        case Pull::PullDown:
            gpio_pull_down(pin_num);
            break;
        case Pull::PullUp:
            gpio_pull_up(pin_num);
            break;
        case Pull::Float:
        default:
            break;
    }
}

void init_output(uint32_t pin_num)
{
    gpio_init(pin_num);
    gpio_set_dir(pin_num, true);
}

bool read(uint32_t pin_num)
{
    return gpio_get(pin_num);
}

}