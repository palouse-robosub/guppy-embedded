#include <pico/stdlib.h>
#include <array>
#include <algorithm>
#include <cstdio>
#include <optional>

#include <guppylib/guppylib.hpp>

#include "board_motor.hpp"
#include "firmware_version.hpp"

using namespace guppylib;


constexpr uint8_t num_motors = 8;
constexpr std::array<uint8_t, num_motors> motor_pins = { 16, 17, 18, 20, 19, 25, 26, 27 }; // motors 3 & 4 swapped in hardware

constexpr uint8_t estop_pin = 29;
constexpr uint8_t led_pin = 28;

constexpr uint16_t motor_board_heartbeat_id = 0x010;
constexpr uint16_t motor_board_version_id = 0x01C;
constexpr uint16_t motor_board_id = 0x410;
constexpr uint16_t estop_triggered_id = 0x01B;
// constexpr uint16_t torpedo_servo_id = 0x019;
// constexpr uint16_t claw_servo_id = 0x01A;

#define MOTOR_MULT 1.0



int main()
{
    stdio_init_all();

    board_motor_loop();
}

bool allowed_to_motor(State state)
{
    return state == State::Holding
        || state == State::Nav
        || state == State::Task
        || state == State::Teleop;
}

void board_motor_loop()
{
    // last time motors have been updated, used for stale motors
    Timer last_updates[num_motors];
    std::fill_n(last_updates, num_motors, Timer(500));

    // set up leds
    constexpr size_t led_group_sizes[] = { 42, 42, 42};
    auto led_controller = std::make_unique<LEDController<3>>(led_pin, led_group_sizes);

    // set up estop
    gpio::init_input(estop_pin, gpio::Pull::PullUp);
    Timer estop_rate_limit(20);
    bool estop_triggered = true; // to be safe, start with trigger being true

    Core context(8, 9, 0x010, std::move(led_controller));

    /* callback called on can message */
    /* updates motors */
    context.set_can_bus_message_callback([&](can2040_msg& msg, Core<3>& core)
    {
        if (msg.id >= motor_board_id + 1 && msg.id <= motor_board_id + num_motors)
        {
            float motor_value = CanBus::parse_float(msg) * MOTOR_MULT;
        
            const int pwm_value = pwm::from_float(motor_value);

            const int index = msg.id - motor_board_id - 1;
            if (!estop_triggered && allowed_to_motor(core.get_state()))
                pwm::write(motor_pins[index], pwm_value);
            last_updates[index].reset();
        }
    });
    
    // initialize pins
    for (int i = 0; i < num_motors; i++)
    {
        pwm::init_pin(motor_pins[i]);
    }

    // Parse once at startup; report malformed tags without stopping board work.
    constexpr uint32_t packed_version = firmware_version::parse(PICO_PROGRAM_VERSION_STRING);
    if ((packed_version & 0xFF000000u) == 0xFF000000u)
    {
        std::printf("Invalid firmware version format, expected v.XX.XX.XX\n");
    }

    // timer to send the version every second, if the version is valid and the CAN ID is set 
    Timer version_timer(1000);

    while (true)
    {
        context.tick();

        // transmit version value if the version is valid, the CAN ID is set, and the timer has elapsed
        if (version_timer.check())
        {
            context.can_bus().transmit(motor_board_version_id, packed_version);
        }

        if (estop_rate_limit.check())
        {
            estop_triggered = gpio::read(estop_pin);
            context.can_bus().transmit(estop_triggered_id, static_cast<int32_t>(estop_triggered)); 
            // TODO: should not be transmitting all the time
            // there is already a heartbeat so this should only send on change right? Will packets ever get dropped?
        }

        /* set stale motors to 0 */
        for (int i = 0; i < num_motors; i++)
        {
            if (last_updates[i].check()) 
            {
                pwm::write(motor_pins[i], pwm::from_float(0.0));
            }
        }

        if (estop_triggered || !allowed_to_motor(context.get_state()))
        {
            for (int i = 0; i < num_motors; i++)
            {
                pwm::write(motor_pins[i], pwm::from_float(0.0));
            }
        }
    }
}
