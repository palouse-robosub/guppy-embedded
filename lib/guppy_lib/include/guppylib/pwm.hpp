#ifndef GUPPY_EMBEDDED_PWM_H
#define GUPPY_EMBEDDED_PWM_H

#include <pico/stdlib.h>

namespace guppylib::pwm
{

// sets up a pin for PWM and gives it an initial signal of 1500 microseconds.
void init_pin(uint pin_num);

// writes a pwm signal to a pin. It must first be set up using add_pwm_pin()
void write(uint pin_num, uint16_t level);

// converts a float from -1.0 to 1.0 into a pwm signal
int float_to_signal(float value);

}

#endif // GUPPY_EMBEDDED_PWM_H