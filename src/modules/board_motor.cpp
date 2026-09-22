#include <pico/stdlib.h>
#include <array>
#include <algorithm>

#include <guppylib/led.hpp>
#include <guppylib/pwm.hpp>
#include <guppylib/canbus.hpp>
#include <guppylib/ratelimit.hpp>
#include <guppylib/state.hpp>
#include <guppylib/core.hpp>
#include <guppylib/gpio.hpp>

#include "board_motor.h"

using namespace guppylib;


constexpr uint8_t num_pins = 8;
constexpr std::array<uint8_t, num_pins> pwm_pins = { 16, 17, 18, 20, 19, 25, 26, 27 }; // motors 3 & 4 swapped in hardware

constexpr uint8_t estop_pin = 29;
constexpr uint8_t led_pin = 28;

constexpr uint16_t motor_board_heartbeat_id = 0x010;
constexpr uint16_t motor_board_id = 0x410;
constexpr uint16_t estop_triggered_id = 0x01B;
// constexpr uint16_t torpedo_servo_id = 0x019;
// constexpr uint16_t claw_servo_id = 0x01A;

#define MOTOR_MULT 1.0

void board_motor_loop()
{
    // last time motors have been updated, used for stale motors
    RateLimit<500> last_updates[num_pins]{};

    // set up leds
    constexpr size_t led_group_sizes[] = { 42, 42, 42};
    auto led_controller = std::make_unique<LEDController<3>>(led_pin, led_group_sizes);

    // set up estop
    gpio::init_input(estop_pin, gpio::Pull::PullUp);
    RateLimit<20> estop_rate_limit;
    bool estop_triggered = true; // to be safe, start with trigger being true

    Core context(8, 9, 0x010, std::move(led_controller));

    /* callback called on can message */
    /* updates motors */
    context.set_can_bus_message_callback([&](can2040_msg& msg, Core<3>& core)
    {
        if (msg.id >= motor_board_id + 1 && msg.id <= motor_board_id + num_pins)
        {
            float motor_value = CanBus::parse_float(msg) * MOTOR_MULT;
            motor_value = std::clamp(motor_value, -1.0f, 1.0f);
        
            const int pwm_value = pwm::float_to_signal(motor_value);

            const int index = msg.id - motor_board_id - 1;
            if (!estop_triggered && allowed_to_motor(core.get_state()))
                pwm::write(pwm_pins[index], pwm_value);
            last_updates[index].reset();
        }
    });
    
    // initialize pins
    for (int i = 0; i < num_pins; i++)
    {
        pwm::init_pin(pwm_pins[i]);
    }

    while (true)
    {
        context.tick();

        if (estop_rate_limit.has_timeout())
        {
            estop_triggered = gpio::read(estop_pin);
            context.can_bus().transmit(estop_triggered_id, static_cast<int32_t>(estop_triggered)); 
            // TODO: should not be transmitting all the time
            // there is already a heartbeat so this should only send on change right? Will packets ever get dropped?
        }

        /* set stale motors to 0 */
        for (int i = 0; i < num_pins; i++)
        {
            if (last_updates[i].has_timeout()) 
            {
                pwm::write(pwm_pins[i], pwm::float_to_signal(0.0));
            }
        }

        if (estop_triggered || !allowed_to_motor(context.get_state()))
        {
            for (int i = 0; i < num_pins; i++)
            {
                pwm::write(pwm_pins[i], pwm::float_to_signal(0.0));
            }
        }
    }
}