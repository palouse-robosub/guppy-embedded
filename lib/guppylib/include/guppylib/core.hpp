#ifndef GUPPY_EMBEDDED_CORE_HPP
#define GUPPY_EMBEDDED_CORE_HPP

#include <functional>
#include <memory>

#include <pico/stdlib.h>

#include "guppylib/canbus.hpp"
#include "guppylib/led.hpp"
#include "guppylib/state.hpp"
#include "guppylib/timer.hpp"

namespace guppylib
{

// Stores state, leds, and canbus. Basically anything that commonly used
template <size_t LEDGroups>
class Core
{
private:
    State state_ = State::Startup;
    std::unique_ptr<LEDController<LEDGroups>> led_strip_;
    std::function<void(can2040_msg&, Core&)> can_bus_message_callback_; // called on receiving canbus msg
    int heartbeat_id_;
    CanBus can_bus_;
    Timer heartbeat_timer_;

    void do_heartbeat();

public:
    Core(uint32_t can_bus_rx_pin, uint32_t can_bus_tx_pin, uint32_t heartbeat_id, std::unique_ptr<LEDController<LEDGroups>> led_controller);

    /**
     * @brief Should run every loop,
     * Updates CAN, LEDs, and state!
     * Also runs callback if CAN has incoming message
     */
    void tick();

    /**
     * @brief Sets the callback ran whenever a CAN message is received.
     * 
     * @param callback The callback to be ran with signature `void(can2040_msg&, Core&)`
     */
    void set_can_bus_message_callback(std::function<void(can2040_msg&, Core&)> callback) { can_bus_message_callback_ = callback; }

    /**
     * @brief Get the current state.
     * 
     * @return State 
     */
    [[nodiscard]] State get_state() const { return state_; }

    /**
     * @brief Get CAN bus instance.
     * 
     * @return CanBus& 
     */
    CanBus& can_bus() { return can_bus_; }

    void update_state(const can2040_msg& msg);
};



template<size_t LEDGroups>
Core<LEDGroups>::Core(
    const uint32_t can_bus_rx_pin,
    const uint32_t can_bus_tx_pin,
    const uint32_t heartbeat_id,
    std::unique_ptr<LEDController<LEDGroups>> led_controller
)
: led_strip_(std::move(led_controller)), heartbeat_timer_(500)
{
    can_bus_.setup(can_bus_rx_pin, can_bus_tx_pin);
    heartbeat_id_ = heartbeat_id;
}

template <size_t LEDGroups>
inline void Core<LEDGroups>::update_state(const can2040_msg &msg)
{
    if (msg.id == 0x201)
    {
        state_ = static_cast<State>(CanBus::parse_int(msg));
        led_strip_->set_state(state_);
    }
    else if (msg.id == 0x021)
    {
        float brightness_float = CanBus::parse_float(msg);
        if (brightness_float > 1.0) brightness_float = 1.0;
        if (brightness_float < 0.0) brightness_float = 0.0;
        led_strip_->set_brightness(static_cast<uint8_t>(brightness_float * 255.0));
    }
}

template<size_t LEDGroups>
void Core<LEDGroups>::tick()
{
    do_heartbeat();
    led_strip_->tick();

    can2040_msg msg;
    if (can_bus_.read(&msg))
    {
        update_state(msg);
        can_bus_message_callback_(msg, *this);
    }
}

template<size_t LEDGroups>
void Core<LEDGroups>::do_heartbeat()
{
    if (!heartbeat_timer_.check()) return;

    int32_t cur_time = to_ms_since_boot(get_absolute_time());
    can_bus_.transmit(heartbeat_id_, cur_time);
}

}

#endif //GUPPY_EMBEDDED_CORE_HPP
