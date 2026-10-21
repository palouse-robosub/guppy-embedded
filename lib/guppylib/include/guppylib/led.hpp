#ifndef GUPPY_EMBEDDED_LED_HPP
#define GUPPY_EMBEDDED_LED_HPP

#include "guppylib/canbus.hpp"
#include "adafruit/Adafruit_NeoPixel.hpp"
#include "guppylib/state.hpp"
#include "guppylib/timer.hpp"

namespace guppylib
{

template <size_t LED_GROUP_COUNT>
class LEDController
{
public:
    State state_;
    /// Groups of LEDs in a row of a continuous color
    size_t led_groups_[LED_GROUP_COUNT];
    uint8_t brightness_ = 35;

    /**
     * @brief Construct a new LEDController object in the Startup state initially.
     * 
     * @param pin Pin number of the LED strip.
     * @param groups Array of the lengths of each section in the LED strip. Used in some patterns.
     */
    explicit LEDController(int pin, const size_t groups[LED_GROUP_COUNT]);

    /**
     * @brief Updates LED strip pattern
     * 
     */
    void tick();
    
    /**
     * @brief Set the state object
     * 
     * @param new_state 
     */
    void set_state(State new_state);

    /**
     * @brief Set the brightness of the LEDs.
     * 
     * @param new_brightness int from 0 to 255.
     */
    void set_brightness(uint8_t new_brightness);
private:
    int tick_count_;
    Adafruit_NeoPixel led_strip_;
    Timer rate_limit_;
    void two_color(uint32_t color1, uint32_t color2);
    void startup();
    void holding();
    void nav();
    void task();
    void teleop();
    void disabled();
    void fault();
    [[nodiscard]] constexpr uint32_t color_white()  const { return Adafruit_NeoPixel::Color(brightness_,  brightness_,  brightness_  ); }
    [[nodiscard]] constexpr uint32_t color_grey()   const { return Adafruit_NeoPixel::Color(brightness_/2,brightness_/2,brightness_/2); }
    [[nodiscard]] constexpr uint32_t color_red()    const { return Adafruit_NeoPixel::Color(brightness_,  0,            0            ); }
    [[nodiscard]] constexpr uint32_t color_green()  const { return Adafruit_NeoPixel::Color(0,            brightness_,  0            ); }
    [[nodiscard]] constexpr uint32_t color_blue()   const { return Adafruit_NeoPixel::Color(0,            0,            brightness_  ); }
    [[nodiscard]] constexpr uint32_t color_yellow() const { return Adafruit_NeoPixel::Color(brightness_,  brightness_,  0            ); }
    [[nodiscard]] constexpr uint32_t orange()       const { return Adafruit_NeoPixel::Color(brightness_,  brightness_/2,0            ); }
    [[nodiscard]] constexpr uint32_t purple()       const { return Adafruit_NeoPixel::Color(brightness_/2,0,            brightness_  ); }
    [[nodiscard]] constexpr uint32_t color_off()    const { return Adafruit_NeoPixel::Color(0,            0,            0            ); }
};

template <size_t LED_GROUP_COUNT>
LEDController<LED_GROUP_COUNT>::LEDController(int pin, const size_t groups[LED_GROUP_COUNT])
: rate_limit_(250)
{
    uint16_t total_leds = 0;
    for (size_t i = 0; i < LED_GROUP_COUNT; i++) {
        led_groups_[i] = groups[i];
        total_leds += groups[i];
    }
    // led_strip = Adafruit_NeoPixel(total_leds, pin, NEO_GRB + NEO_KHZ800);
    led_strip_ = Adafruit_NeoPixel(total_leds, pin, NEO_GRB + NEO_KHZ800);
    led_strip_.begin();
    //for (int i = 0; i < 122; i++) led_strip.setPixelColor(i, this->color_green());

    tick_count_ = 0;
    state_ = State::Startup;
}

template <size_t LED_GROUP_COUNT>
void LEDController<LED_GROUP_COUNT>::two_color(uint32_t color1, uint32_t color2)
{
    int led_index = 0;
    for (int i = 0; i < LED_GROUP_COUNT; i++)
    {
        for (int j = 0; j < led_groups_[i]; j++) {
            if (j < led_groups_[i] / 2)
                led_strip_.setPixelColor(led_index + j, color1);
            else 
                led_strip_.setPixelColor(led_index + j, color2);
        }
        led_index += led_groups_[i];
    }
}

template <size_t LED_GROUP_COUNT>
void LEDController<LED_GROUP_COUNT>::set_state(const State new_state)
{
    if (new_state == state_) return;
    state_ = new_state;
    tick_count_ = 0;
}

template <size_t LED_GROUP_COUNT>
void LEDController<LED_GROUP_COUNT>::set_brightness(const uint8_t new_brightness)
{
    brightness_ = new_brightness;
}

template <size_t LED_GROUP_COUNT>
void LEDController<LED_GROUP_COUNT>::tick()
{
    if (!rate_limit_.check())
        return;

    switch (state_)
    {
        case State::Startup:  this->startup();  break;
        case State::Holding:  this->holding();  break;
        case State::Nav:      this->nav();      break;
        case State::Task:     this->task();     break;
        case State::Teleop:   this->teleop();   break;
        case State::Disabled: this->disabled(); break;
        case State::Fault:    this->fault();    break;
        default:              this->fault();    break;
    }
    led_strip_.show();
    ++tick_count_;
}

template <size_t LED_GROUP_COUNT>
void LEDController<LED_GROUP_COUNT>::startup()
{
    // bouncing white dot going back and forth across the groups
    int led_index = 0;
    for (int i = 0; i < LED_GROUP_COUNT; i++)
    {
        for (int j = 0; j < led_groups_[i]; j++) {
            size_t n = led_groups_[i];
            // creates patterns like 1 2 3 4 3 2 1 2 3 4 3 ...
            size_t center = tick_count_ % (2 * n - 2) + 1;
            if (center > n) center = 2 * n - center;
            center -= 1; // convert from 1..=n to  0..n

            size_t dot_radius = 2;
            if (j - center < dot_radius || center - j < dot_radius)
                led_strip_.setPixelColor(led_index+j, this->color_white());
            else if (j - center < dot_radius + 1 || center - j < dot_radius + 1)
                led_strip_.setPixelColor(led_index+j, this->color_grey());
            else
                led_strip_.setPixelColor(led_index+j, this->color_off());
        }
        led_index += led_groups_[i];
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
    if (this->tick_count_ % 2)
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
    if (this->tick_count_ % 2)
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
    if (this->tick_count_ % 2)
        two_color(this->color_blue(), this->color_blue());
    else
        two_color(this->color_red(), this->color_red());
}

}

#endif //GUPPY_EMBEDDED_LED_HPP