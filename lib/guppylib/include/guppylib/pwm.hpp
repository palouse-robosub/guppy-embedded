#ifndef GUPPY_EMBEDDED_PWM_H
#define GUPPY_EMBEDDED_PWM_H

#include <pico/stdlib.h>

namespace guppylib::pwm
{

/**
 * @brief Initializes a pin for PWM with an initial duration of 1500us.
 * 
 * @param pin_num pin number to initialize
 */
void init_pin(uint pin_num);

/**
 * @brief Writes a pwm signal to a pin.
 * 
 * @param pin_num pin number to write to
 * @param level pwm duration in microseconds
 */
void write(uint pin_num, uint16_t level);

/**
 * @brief Converts a float from -1, to 1 to a pwm signal.
 * Input value is clamped to [-1, 1]
 * 
 * @param value input float
 * @return pwm signal
 */
int from_float(float value);

}

#endif // GUPPY_EMBEDDED_PWM_H