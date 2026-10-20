#include "guppylib/canbus.hpp"

namespace guppylib
{

void CanBus::setup(const uint32_t gpio_rx, const uint32_t gpio_tx)
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

bool CanBus::read(can2040_msg *msg)
{
    const uint32_t push_pos = message_queue_.push_pos;
    const uint32_t pull_pos = message_queue_.pull_pos;

    if (pull_pos == push_pos) return false;

    (*msg) = message_queue_.queue[pull_pos % QUEUE_SIZE];
    message_queue_.pull_pos++;

    return true;
}

float CanBus::parse_float(const can2040_msg& msg)
{
    float value;
    std::memcpy(&value, msg.data, sizeof(float));

    return value;
}

int32_t CanBus::parse_int(const can2040_msg& msg)
{
    int32_t value;
    std::memcpy(&value, msg.data, sizeof(int32_t));

    return value;
}

void CanBus::can_callback(can2040 *cd, uint32_t notify, can2040_msg *msg) // TODO: filter only ids we fricking want
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

}