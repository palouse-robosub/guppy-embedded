#ifndef GUPPY_EMBEDDED_CORE_HPP
#define GUPPY_EMBEDDED_CORE_HPP

#include <functional>
#include <memory>

#include <pico/stdlib.h>

#include "guppylib/canbus.hpp"
#include "guppylib/led.hpp"
#include "guppylib/state.hpp"
#include "guppylib/ratelimit.hpp"

namespace guppylib
{

// Stores state, leds, and canbus. Basically anything that commonly used
template <size_t LEDGroups>
class Core
{
private:
    static constexpr int update_rate_ms_ = 20; // 50 times a second

    State state_;
    std::unique_ptr<LEDController<LEDGroups>> led_strip_;
    std::function<void(can2040_msg&, Core&)> can_bus_message_callback_; // called on receiving canbus msg
    int heartbeat_id_;
    CanBus can_bus_;
    RateLimit<1000> heartbeat_rate_limit_;

    void do_heartbeat();

public:
    Core(uint32_t can_bus_rx_pin, uint32_t can_bus_tx_pin, uint32_t heartbeat_id, std::unique_ptr<LEDController<LEDGroups>> led_controller);

    void tick();

    void set_can_bus_message_callback(std::function<void(can2040_msg&, Core&)> callback) { can_bus_message_callback_ = callback; }

    [[nodiscard]] State get_state() const { return state_; }
    CanBus& can_bus() { return can_bus_; }
};



template<size_t LEDGroups>
Core<LEDGroups>::Core(
    const uint32_t can_bus_rx_pin,
    const uint32_t can_bus_tx_pin,
    const uint32_t heartbeat_id,
    std::unique_ptr<LEDController<LEDGroups>> led_controller
)
: led_strip_(std::move(led_controller))
{
    can_bus_.setup(can_bus_rx_pin, can_bus_tx_pin);
    heartbeat_id_ = heartbeat_id;
}

template<size_t LEDGroups>
void Core<LEDGroups>::tick()
{
    do_heartbeat();
    led_strip_->tick();

    can2040_msg msg;
    if (can_bus_.read(&msg))
    {
        led_strip_->update(msg);
    }
}

template<size_t LEDGroups>
void Core<LEDGroups>::do_heartbeat()
{
    if (!heartbeat_rate_limit_.has_timeout()) return;

    int32_t cur_time = to_ms_since_boot(get_absolute_time());
    can_bus_.transmit(heartbeat_id_, cur_time);
}

}

#endif //GUPPY_EMBEDDED_CORE_HPP