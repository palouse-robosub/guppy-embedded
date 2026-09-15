// TODO: look over function rq
#include <pico/stdlib.h>
#include <hardware/pwm.h>


#include "guppylib/pwm.hpp"

namespace guppylib::pwm
{

void init_pin(uint pin_num)
{
    gpio_set_function(pin_num, GPIO_FUNC_PWM);

    const uint slice_num = pwm_gpio_to_slice_num(pin_num);
    const uint channel_num = pwm_gpio_to_channel(pin_num);

    const float divider = 150.0f; // this maps the level to be in micro seconds
    const int wrap = 20000;

    // Configure PWM frequency (wrap value) and duty cycle (channel level)
    pwm_set_clkdiv(slice_num, divider);
    pwm_set_wrap(slice_num, wrap); // TODO: look at wrap

    // set initial signal to 1500, which is neutral state
    pwm_set_chan_level(slice_num, channel_num, 1500);
    pwm_set_enabled(slice_num, true);
}

void write(uint pin_num, uint16_t level)
{
    const uint slice_num = pwm_gpio_to_slice_num(pin_num);
    const uint channel_num = pwm_gpio_to_channel(pin_num);

    pwm_set_chan_level(slice_num, channel_num, level);
}

int float_to_signal(float value)
{
    return 1500 + (int)(value * 400);
}

}