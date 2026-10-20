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
    void setup(const uint32_t gpio_rx = 8, const uint32_t gpio_tx = 9)
    {
        uint32_t pio_num = 2;
        uint32_t sys_clock = SYS_CLK_HZ, bitrate = 500000;

        // Setup canbus
        can2040_setup(&can_bus_, pio_num);
        can2040_callback_config(&can_bus_, can_callback); // TODO: when we get an actual controller, FIX THIS NOW (no more singleton)

        // Enable irqs
        irq_set_exclusive_handler(PIO2_IRQ_0, PIOx_IRQHandler);
        irq_set_priority(PIO2_IRQ_0, 1);
        irq_set_enabled(PIO2_IRQ_0, 1);

        // Start canbus
        can2040_start(&can_bus_, sys_clock, bitrate, gpio_rx, gpio_tx);
    }

    /**
     * @brief Tries to read an incoming message, and sets `msg` to the incoming message
     * 
     * @param msg the message struct to be written to
     * @return true if there is an incoming message
     */
    bool read(can2040_msg *msg)
    {
        const uint32_t push_pos = message_queue_.push_pos;
        const uint32_t pull_pos = message_queue_.pull_pos;

        if (pull_pos == push_pos) return false;

        (*msg) = message_queue_.queue[pull_pos % QUEUE_SIZE];
        message_queue_.pull_pos++;

        return true;
    }

    /**
     * @brief Transmits a value over CAN. 
     * 
     * @tparam T Type of data to send, must be at most 4 bytes.
     * @param id CAN id to send
     * @param value Value to send over CAN
     * @return Status code, 0 is good
     */
    template <typename T>
    int transmit(uint32_t id, T value)
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

    /**
     * @brief Parses a `can2040_msg` as a float
     * 
     * @param msg Message to parse.
     * @return float 
     */
    static float parse_float(const can2040_msg& msg)
    {
        float value;
        std::memcpy(&value, msg.data, sizeof(float));

        return value;
    }

    /**
     * @brief Parses a `can2040_msg` as an int
     * 
     * @param msg Message to parse.
     * @return int32_t 
     */
    static int32_t parse_int(const can2040_msg& msg)
    {
        int32_t value;
        std::memcpy(&value, msg.data, sizeof(int32_t));

        return value;
    }

private:
    // callback whenever a can frame is received
    static void can_callback(can2040 *cd, uint32_t notify, can2040_msg *msg) // TODO: filter only ids we fricking want
    {
        if (notify == CAN2040_NOTIFY_RX) {
            // uint32_t id = msg->id;
            // if (id < 0x101 || id > 0x201)
            //     return;

            // Add to queue
            uint32_t push_pos = instance_->message_queue_.push_pos;
            uint32_t pull_pos = instance_->message_queue_.pull_pos;
            if (push_pos + 1 == pull_pos) // TODO: should send warning that no space left!
                // No space in queue
                return;
            instance_->message_queue_.queue[push_pos % QUEUE_SIZE] = *msg;
            instance_->message_queue_.push_pos = push_pos + 1;
        }
    }

    // PIO interrupt handler
    static void PIOx_IRQHandler()
    {
        can2040_pio_irq_handler(&instance_->can_bus_);
    }
};

}

#endif //GUPPY_EMBEDDED_CANBUS_HPP