#ifndef GUPPY_EMBEDDED_CANBUS_HPP
#define GUPPY_EMBEDDED_CANBUS_HPP

#include <pico/stdlib.h>
#include <cstring>
#include <type_traits>

extern "C"
{
#include "can2040.h"
}

namespace guppylib
{

// Simple example of irq safe queue (this is not multi-core safe)
#define QUEUE_SIZE 128 // Must be power of 2

class CanBus
{
private:

    struct MessageQueue {
        uint32_t pull_pos{};
        volatile uint32_t push_pos{};
        struct can2040_msg queue[QUEUE_SIZE]{};
    };

    can2040 can_bus_;
    MessageQueue message_queue_;

    static inline CanBus* instance_{};
    
public:

    /**
     * @brief Construct a new Can Bus object. Must run `setup()` first! 
     * 
     */
    CanBus()
    {
        // TODO: error if already an instance
        instance_ = this; 
    }

    /**
     * @brief Sets up the can bus
     * 
     * @param gpio_rx pin number for receiving (rx)
     * @param gpio_tx pin number for transmitting (tx)
     */
    void setup(uint32_t gpio_rx = 8, uint32_t gpio_tx = 9);

    /**
     * @brief Tries to read an incoming message, and sets `msg` to the incoming message
     * 
     * @param msg the message struct to be written to
     * @return true if there is an incoming message
     */
    bool read(can2040_msg *msg);

    /**
     * @brief Transmits a value over CAN. 
     * 
     * @tparam T Type of data to send, must be at most 4 bytes.
     * @param id CAN id to send
     * @param value Value to send over CAN
     * @return Status code, 0 is good
     */
    template <typename T>
    int transmit(uint32_t id, T value);

    /**
     * @brief Parses a `can2040_msg` as a float
     * 
     * @param msg Message to parse.
     * @return float 
     */
    static float parse_float(const can2040_msg& msg);

    /**
     * @brief Parses a `can2040_msg` as an int
     * 
     * @param msg Message to parse.
     * @return int32_t 
     */
    static int32_t parse_int(const can2040_msg& msg);

private:
    // callback whenever a can frame is received
    static void can_callback(can2040 *cd, uint32_t notify, can2040_msg *msg);

    // PIO interrupt handler
    static void PIOx_IRQHandler()
    {
        can2040_pio_irq_handler(&instance_->can_bus_);
    }
};

template <typename T>
int CanBus::transmit(const uint32_t id, const T value)
{
    static_assert(std::is_trivially_copyable_v<T>, "Type is not trivially copyable");
    static_assert(sizeof(T) <= 4, "Type must be at most 4 bytes");
    can2040_msg msg;
    msg.id = id;
    msg.dlc = sizeof(T);
    std::memcpy(&msg.data32[0], &value, sizeof(T));
    int status = can2040_transmit(&can_bus_, &msg);

    return status;
}

}

#endif //GUPPY_EMBEDDED_CANBUS_HPP