extern "C" {
    #include "can2040.h"
    #include "guppy_lib.h"
    #include "pico/stdlib.h"
    #include <stdio.h>
}
#include "board_motor.h"
#include "led.hpp"

#define NUM_PINS 8
static const uint8_t pwm_pins[NUM_PINS] = { 16, 17, 18, 20, 19, 25, 26, 27 }; // motors 3 & 4 swapped in hardware
static RateLimit last_updates[NUM_PINS]{};

constexpr uint8_t estop_pin = 29;
constexpr uint8_t led_pin = 28;

constexpr uint16_t motor_board_heartbeat_id = 0x010;
constexpr uint16_t motor_board_id = 0x410;
constexpr uint16_t estop_triggered_id = 0x01B;
constexpr uint16_t ros_heartbeat_id = 0xAAA;
// constexpr uint16_t torpedo_servo_id = 0x019;
// constexpr uint16_t claw_servo_id = 0x01A;

#define MOTOR_MULT 1.0
#define ALLOW_STALE_MOTORS false // if this is true it won't set stale motors to 0

void board_motor_loop()
{
    for (int i = 0; i < NUM_PINS; i++) {
        add_pwm_pin(pwm_pins[i]);
        last_updates[i] = new_rate_limit(500);
    }
    // add_pwm_pin(claw_servo_pin);
    // add_pwm_pin(torpedo_servo_pin);
    gpio_init(estop_pin);
    gpio_set_dir(estop_pin, GPIO_IN);
    gpio_pull_up(estop_pin);
    RateLimit estop_rate_limit = new_rate_limit(20);
    bool estop_triggered = true;

    RateLimit ros_timeout = new_rate_limit(ROS_TIMEOUT_DELAY_MS);

    constexpr int strip_count = 3;
    size_t led_groups[strip_count] = {42, 42, 42};
    LEDController<strip_count> led_strip(led_pin, led_groups);

    while (true)
    {
        do_heartbeat(motor_board_heartbeat_id);
        led_strip.tick();

        const absolute_time_t cur_time = get_absolute_time();

        if (check_rate(&ros_timeout))
        {
            led_strip.timeout();
        }

        struct can2040_msg msg = { 0 };
        if (canbus_read(&msg)) // returns true if has message. Sets the &msg to the message
        {
            led_strip.update(msg);
            if (msg.id >= (motor_board_id+1) && msg.id <= (motor_board_id+NUM_PINS))
            {
                float value = can_read_float(msg) * MOTOR_MULT;
                if (value > 1.0) value = 1.0;
                if (value < -1.0) value = -1.0;
                int micro_seconds = throttle_to_pwm_us(value);

                // NDEBUG should be added by CMAKE on release builds
                printf("received motor power for id %x\n", msg.id);
                printf("pwm level (should be from 1100 to 1900): %d\n", micro_seconds);
                printf("recieved float: %f\n\n", value);

                const int index = msg.id - motor_board_id - 1;
                if (!estop_triggered && allowed_to_motor(led_strip.state))
                    pwm_write(pwm_pins[index], micro_seconds);
                last_updates[index].time = cur_time;
            }
            if (msg.id == ros_heartbeat_id) // reset ros timeout
            {
                ros_timeout = new_rate_limit(ROS_TIMEOUT_DELAY_MS);
                check_rate(&ros_timeout);
            }

            // TODO: test servo functionality, it may be different than motor throttle
            // if (msg.id == claw_servo_id) {
            //     pwm_write(claw_servo_pin, throttle_to_pwm_us(can_read_float(msg)));
            // }
            // if (msg.id == torpedo_servo_id) {
            //     pwm_write(torpedo_servo_pin, throttle_to_pwm_us(can_read_float(msg)));
            // }
        }

        #if ALLOW_STALE_MOTORS == false
        for (int i = 0; i < NUM_PINS; i++) // set stale motors to 0
        {
            if (check_rate(&last_updates[i]))
            {
                printf("stale motor %d\n", i);
                pwm_write(pwm_pins[i], throttle_to_pwm_us(0.0));
            }
        }
        #endif

        // E-Stop & state-based motor shutoff
        if (check_rate(&estop_rate_limit))
        {
            estop_triggered = gpio_get(estop_pin); // estop triggered by disconnecting the switch
            canbus_transmit_int(estop_triggered_id, estop_triggered);

            if (estop_triggered || !allowed_to_motor(led_strip.state))
            {
                for (int i = 0; i < NUM_PINS; i++)
                {
                    // printf("disable motor %d\n", i);
                    pwm_write(pwm_pins[i], throttle_to_pwm_us(0.0));
                }
            }
        }
    }
}
