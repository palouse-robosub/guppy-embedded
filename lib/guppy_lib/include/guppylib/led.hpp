#ifndef GUPPY_EMBEDDED_LED_HPP
#define GUPPY_EMBEDDED_LED_HPP

extern "C" {
    #include "can2040.h"
}
#include "guppylib/guppy_lib.h"
#include "guppylib/canbus.hpp"
#include "adafruit/Adafruit_NeoPixel.hpp"

#define BRIGHTNESS 50 // brightness of pixels out of 255

namespace guppylib
{

template <size_t LED_GROUP_COUNT>
class LEDController
{
public:
    State state;
    /// Groups of LEDs in a row of a continuous color
    size_t led_groups[LED_GROUP_COUNT];
    uint8_t brightness = 35;
    /// `pin` is the gpio pin the LED line data is connected to
    /// `groups` is a list of lengths of LED groups
    explicit LEDController(int pin, const size_t groups[LED_GROUP_COUNT]);
    /// Updates the LED strip
    void tick();
    /// Updates LEDController based on the provided CAN message
    /// (does care about CAN ID and will ignore messages from most IDs)
    bool update(const struct can2040_msg& msg);
private:
    int tick_count;
    Adafruit_NeoPixel led_strip;
    RateLimit rate_limit;
    void two_color(uint32_t color1, uint32_t color2);
    void startup();
    void holding();
    void nav();
    void task();
    void teleop();
    void disabled();
    void fault();
    [[nodiscard]] uint32_t color_white()  const { return Adafruit_NeoPixel::Color(brightness,  brightness,  brightness  ); }
    [[nodiscard]] uint32_t color_grey()   const { return Adafruit_NeoPixel::Color(brightness/2,brightness/2,brightness/2); }
    [[nodiscard]] uint32_t color_red()    const { return Adafruit_NeoPixel::Color(brightness,  0,           0           ); }
    [[nodiscard]] uint32_t color_green()  const { return Adafruit_NeoPixel::Color(0,           brightness,  0           ); }
    [[nodiscard]] uint32_t color_blue()   const { return Adafruit_NeoPixel::Color(0,           0,           brightness  ); }
    [[nodiscard]] uint32_t color_yellow() const { return Adafruit_NeoPixel::Color(brightness,  brightness,  0           ); }
    [[nodiscard]] uint32_t orange()       const { return Adafruit_NeoPixel::Color(brightness,  brightness/2,0           ); }
    [[nodiscard]] uint32_t purple()       const { return Adafruit_NeoPixel::Color(brightness/2,0,           brightness  ); }
    [[nodiscard]] uint32_t color_off()    const { return Adafruit_NeoPixel::Color(0,           0,           0           ); }
};

template <size_t LED_GROUP_COUNT>
LEDController<LED_GROUP_COUNT>::LEDController(int pin, const size_t groups[LED_GROUP_COUNT])
{
    uint16_t total_leds = 0;
    for (size_t i = 0; i < LED_GROUP_COUNT; i++) {
        led_groups[i] = groups[i];
        total_leds += groups[i];
    }
    // led_strip = Adafruit_NeoPixel(total_leds, pin, NEO_GRB + NEO_KHZ800);
    led_strip = Adafruit_NeoPixel(total_leds, pin, NEO_GRB + NEO_KHZ800);
    led_strip.begin();
    //for (int i = 0; i < 122; i++) led_strip.setPixelColor(i, this->color_green());

    tick_count = 0;
    rate_limit = new_rate_limit(250);
    state = State::STARTUP;
}

template <size_t LED_GROUP_COUNT>
void LEDController<LED_GROUP_COUNT>::two_color(uint32_t color1, uint32_t color2)
{
    int led_index = 0;
    for (int i = 0; i < LED_GROUP_COUNT; i++)
    {
        for (int j = 0; j < led_groups[i]; ++j) {
            if (j < led_groups[i]/2) led_strip.setPixelColor(led_index+j, color1);
            else led_strip.setPixelColor(led_index+j, color2);
        }
        led_index += led_groups[i];
    }
}

template <size_t LED_GROUP_COUNT>
bool LEDController<LED_GROUP_COUNT>::update(const can2040_msg& msg)
{
    if (msg.id == 0x201) // CAN ID Spreadsheet `current state`
    {
        State new_state = static_cast<State>(guppylib::canbus::read_int(msg));
        if (new_state != state)
        {
            // we want to synchronize the timing for each hull
            switch (new_state)
            {
            case State::STARTUP:
                rate_limit = new_rate_limit(10);
            default:
                rate_limit = new_rate_limit(250);
            }
            tick_count = 0;
            state = new_state;
        }
        return true;
    }
    if (msg.id == 0x021) // CAN ID Spreadsheet `led brightness`
    {
        float brightness_float = guppylib::canbus::read_float(msg);
        if (brightness_float > 1.0) brightness_float = 1.0;
        if (brightness_float < 0.0) brightness_float = 0.0;
        brightness = static_cast<uint8_t>(brightness_float * 255.0);
        return true;
    }
    else return false;
}

template <size_t LED_GROUP_COUNT>
void LEDController<LED_GROUP_COUNT>::tick()
{
    if (!check_rate(&rate_limit))
        return;

    switch (state)
    {
        case State::STARTUP:  this->startup();  break;
        case State::HOLDING:  this->holding();  break;
        case State::NAV:      this->nav();      break;
        case State::TASK:     this->task();     break;
        case State::TELEOP:   this->teleop();   break;
        case State::DISABLED: this->disabled(); break;
        case State::FAULT:    this->fault();    break;
        default:       this->fault();    break;
    }
    led_strip.show();
    ++tick_count;
}

template <size_t LED_GROUP_COUNT>
void LEDController<LED_GROUP_COUNT>::startup()
{
    // bouncing white dot going back and forth across the groups
    int led_index = 0;
    for (int i = 0; i < LED_GROUP_COUNT; i++)
    {
        for (int j = 0; j < led_groups[i]; ++j) {
            size_t n = led_groups[i];
            // creates patterns like 1 2 3 4 3 2 1 2 3 4 3 ...
            size_t center = tick_count % (2*n - 2) + 1;
            if (center > n) center = 2*n - center;
            center -= 1; // convert from 1..=n to  0..n

            size_t dot_radius = 2;
            if (j - center < dot_radius || center - j < dot_radius)
                led_strip.setPixelColor(led_index+j, this->color_white());
            else if (j - center < dot_radius + 1 || center - j < dot_radius + 1)
                led_strip.setPixelColor(led_index+j, this->color_grey());
            else
                led_strip.setPixelColor(led_index+j, this->color_off());
        }
        led_index += led_groups[i];
    }
}
template <size_t LED_GROUP_COUNT>
void LEDController<LED_GROUP_COUNT>::holding()
{
    this->two_color(this->color_red(), this->color_red());
}
template <size_t LED_GROUP_COUNT>
void LEDController<LED_GROUP_COUNT>::nav()
{
    if (this->tick_count % 2)
        two_color(this->color_red(), this->color_red());
    else
        two_color(this->color_off(), this->color_off());
}
template <size_t LED_GROUP_COUNT>
void LEDController<LED_GROUP_COUNT>::task()
{
    this->two_color(this->color_red(), this->color_red());
}
template <size_t LED_GROUP_COUNT>
void LEDController<LED_GROUP_COUNT>::teleop()
{
    if (this->tick_count % 2)
        two_color(this->color_green(), this->color_green());
    else
        two_color(this->color_off(), this->color_off());
}
template <size_t LED_GROUP_COUNT>
void LEDController<LED_GROUP_COUNT>::disabled()
{
    this->two_color(this->color_green(), this->color_green());
}
template <size_t LED_GROUP_COUNT>
void LEDController<LED_GROUP_COUNT>::fault()
{
    if (this->tick_count % 2)
        two_color(this->color_blue(), this->color_blue());
    else
        two_color(this->color_red(), this->color_red());
}

}

#endif //GUPPY_EMBEDDED_LED_HPP