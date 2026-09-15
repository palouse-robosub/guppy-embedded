#include "board_wet.h"
#include "bluerobotics/barometer.h"
#include "guppylib/led.hpp"

extern "C" {
#include "pico/stdlib.h"
#include "can2040.h"
}
#include <guppylib/guppy_lib.h>
#include <guppylib/canbus.hpp>
#include <iostream>

#define PICO_I2C_INSTANCE   i2c0
#define PICO_I2C_SDA_PIN    16 // white
#define PICO_I2C_SCL_PIN    17 // green

#define SWITCH_PIN_ONE      26
#define SWITCH_PIN_TWO      19 

#define LEDS_PIN            20

using namespace guppylib;

static void init_pins()
{
    // From pico_examples
    i2c_init(PICO_I2C_INSTANCE, 400 * 1000);
    gpio_set_function(PICO_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_I2C_SCL_PIN);
    gpio_pull_up(PICO_I2C_SDA_PIN);

    // init switches
    gpio_init(SWITCH_PIN_ONE);
    gpio_init(SWITCH_PIN_TWO);
    gpio_set_dir(SWITCH_PIN_ONE, GPIO_IN); // TODO: turn to guppy_lib function
    gpio_set_dir(SWITCH_PIN_TWO, GPIO_IN);
    gpio_pull_down(SWITCH_PIN_ONE);
    gpio_pull_down(SWITCH_PIN_TWO);
}

void board_wet_loop()
{
    MS5837 sensor;

    init_pins();

    // make available for picotool
    //bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    sensor.init(PICO_I2C_INSTANCE);
    if (!sensor.isInitialized())
    {
        std::cout << "Initializing barometer failed!" << std::endl;
        std::cout << "Are SDA/SCL connected correctly?" << std::endl;
        std::cout << "Blue Robotics Bar30: White=SDA, Green=SCL" << std::endl;
    }

    sensor.setFluidDensity(997); // kg/m^3 (freshwater) TODO: change to actual density


    size_t led_groups[3] = {42, 40, 40};
    led::LEDController<3> led_strip(LEDS_PIN, led_groups);

    struct can2040_msg msg = { 0 };

    RateLimit publish = new_rate_limit(50);

    while (true)
    {
        if (canbus::read(&msg))
        {
            led_strip.update(msg);
        }

        do_heartbeat(0x020);
        led_strip.tick();

        float depth{};
        float temp{};

        if (check_rate(&publish)) // can bus is getting full so im going to only send stuff like every 100ms
        {
            if (sensor.read())
            {
                depth = sensor.depth();
                temp = sensor.temperature();

                canbus::transmit_float(0x026, depth);
                canbus::transmit_float(0x025, temp);

                //printf("\nPressure: %f\n", sensor.pressure());
                // printf("Altitude: %f\n", sensor.altitude());
                // printf("Depth: %f\n", depth);
                // printf("Temperature: %f\n", temp);
            }
            

            canbus::transmit_int(0x022, gpio_get(SWITCH_PIN_ONE));
            canbus::transmit_int(0x023, gpio_get(SWITCH_PIN_TWO));
            // Barometer
            if (!sensor.isInitialized())
            {
                if (!sensor.init(PICO_I2C_INSTANCE))
                {
                    canbus::transmit_int(0x029, 0);
                }
                else
                {
                    canbus::transmit_int(0x029, 1);
                }
            }

        }
    }
}