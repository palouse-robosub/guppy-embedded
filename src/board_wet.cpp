#include <pico/stdlib.h>
#include <iostream>
#include <memory>
#include <cstdio>
#include <optional>
#include <bluerobotics/barometer.h>

#include <guppylib/core.hpp>
#include <guppylib/gpio.hpp>
#include <guppylib/canbus.hpp>
#include <guppylib/led.hpp>
#include <guppylib/timer.hpp>
#include <guppylib/core.hpp>

#include "board_wet.hpp"
#include "firmware_version.hpp"



#define PICO_I2C_INSTANCE   i2c0
#define PICO_I2C_SDA_PIN    16 // white
#define PICO_I2C_SCL_PIN    17 // green

constexpr uint16_t switch_one_pin = 26;
constexpr uint16_t switch_two_pin = 19;
constexpr uint16_t led_pin = 20;
// TODO: assign a dedicated CAN ID. Currently just a placeholder.
constexpr std::optional<uint16_t> wet_board_version_id = 0x112;

using namespace guppylib;



int main()
{
    stdio_init_all();

    board_wet_loop();
}

static void init_pins()
{
    // From pico_examples
    i2c_init(PICO_I2C_INSTANCE, 400 * 1000);
    gpio_set_function(PICO_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_I2C_SCL_PIN);
    gpio_pull_up(PICO_I2C_SDA_PIN);

    // init switches
    gpio::init_input(switch_one_pin, gpio::Pull::PullDown);
    gpio::init_input(switch_two_pin, gpio::Pull::PullDown);
}

void board_wet_loop()
{
    MS5837 sensor;

    init_pins();

    // set up sensor with I2C pin
    sensor.init(PICO_I2C_INSTANCE);
    if (!sensor.isInitialized())
    {
        std::cout << "Initializing barometer failed!" << std::endl;
        std::cout << "Are SDA/SCL connected correctly?" << std::endl;
        std::cout << "Blue Robotics Bar30: White=SDA, Green=SCL" << std::endl;
    }
    sensor.setFluidDensity(997);

    constexpr size_t led_group_sizes[3] = { 42, 40, 40 };
    auto led_controller = std::make_unique<LEDController<3>>(led_pin, led_group_sizes);

    Timer publish_timer(50);

    Core<3> context(8, 9, 0x020, std::move(led_controller));

    // Parse once at startup; report malformed tags without stopping board work.
    const uint32_t packed_version = firmware_version::parse(PICO_PROGRAM_VERSION_STRING);
    if (packed_version == 0xFFFFFFFFu)
    {
        std::printf("Invalid firmware version format, expected v.XX.XX.XX\n");
    }

    Timer version_timer(1000);
    version_timer.reset();
    if (packed_version != 0xFFFFFFFFu && wet_board_version_id)
    {
        context.can_bus().transmit(*wet_board_version_id, packed_version);
    }

    while (true)
    {
        context.tick();

        if (packed_version != 0xFFFFFFFFu && wet_board_version_id && version_timer.check())
        {
            context.can_bus().transmit(*wet_board_version_id, packed_version);
        }

        if (publish_timer.check())
        {
            /* transmit sensor values */
            if (sensor.read())
            {
                float depth = sensor.depth();
                float temp = sensor.temperature();

                context.can_bus().transmit(0x026, depth);
                context.can_bus().transmit(0x025, temp);
            }

            /* transmit switch values */
            const bool switch_one = gpio::read(switch_one_pin);
            const bool switch_two = gpio::read(switch_two_pin); 

            context.can_bus().transmit(0x022, static_cast<int32_t>(switch_one));
            context.can_bus().transmit(0x023, static_cast<int32_t>(switch_two));

            /* set up switch if it's not set up */ // TODO: sensor init is slow i think and blocking? anyways this stuff isn't good
            if (!sensor.isInitialized())
            {
                if (!sensor.init(PICO_I2C_INSTANCE))
                {
                    context.can_bus().transmit(0x029, static_cast<int32_t>(0));
                }
                else
                {
                    context.can_bus().transmit(0x029, static_cast<int32_t>(1));
                }
            }
        }
    }
}
