#ifndef GUPPY_EMBEDDED_STATE_HPP
#define GUPPY_EMBEDDED_STATE_HPP

#include <functional>

#include "guppylib/led.hpp"
#include "guppylib/ratelimit.hpp"

namespace guppylib
{

enum class State : int
{
    Startup = 0,
    Holding = 1, 
    Nav = 2,
    Task = 3,
    Teleop = 4,
    Disabled = 5,
    Fault = 6
};

// Stores state, leds, and canbus. Basically anything that commonly used
template <size_t LEDGroups>
class GuppyContext
{
private:
    constexpr int update_rate_ms_ = 20; // 50 times a second

    State state_;
    LEDController<LEDGroups> led_strip_;
    std::function<void(can2040_msg&)> can_bus_message_callback_; // called on receiving canbus msg
    int heartbeat_id_;
    CanBus can_bus_;
    RateLimit<1000> heartbeat_rate_limit_;

    void do_heartbeat();

public:
    explicit GuppyContext(uint32_t can_bus_rx_pin = 8, uint32_t can_bus_tx_pin = 9, uint32_t led_pin, const int groups[LEDGroups], uint32_t heartbeat_id);

    void tick();
};

template<size_t LEDGroups>
GuppyContext<LEDGroups>::GuppyContext(
    const uint32_t can_bus_rx_pin,
    const uint32_t can_bus_tx_pin,
    const uint32_t led_pin,
    const int groups[LEDGroups],
    const uint32_t heartbeat_id
)
: led_strip_(led_pin, groups)
{
    can_bus_.setup(can_bus_rx_pin, can_bus_tx_pin);
    heartbeat_id_ = heartbeat_id;
}

template<size_t LEDGroups>
void GuppyContext<LEDGroups>::tick()
{
    do_heartbeat();
    led_strip_.tick();

    can2040_msg msg;
    if (can_bus_.read(&msg))
    {
        led_strip_.update(msg);
    }
}

template<size_t LEDGroups>
void GuppyContext<LEDGroups>::do_heartbeat()
{
    if (!heartbeat_rate_limit_.has_timeout()) return;

    uint32_t cur_time = to_ms_since_boot(get_absolute_time());
    can_bus_.transmit(heartbeat_id_, cur_time);
}

}

#endif //GUPPY_EMBEDDED_STATE_HPP