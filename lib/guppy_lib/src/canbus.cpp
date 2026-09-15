#include <cstring>

#include "guppylib/canbus.hpp"

#include "pico/stdlib.h"

extern "C"
{
#include "can2040.h"
}

namespace guppylib::canbus
{

/* ---------------CAN stuff--------------- */

// Internal storage for can2040 module
static struct can2040 cbus;

// PIO interrupt handler
static void PIOx_IRQHandler()
{
    can2040_pio_irq_handler(&cbus);
}

// TODO: allow for custom filters?
static void can2040_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg)
{
    if (notify == CAN2040_NOTIFY_RX) {
        // uint32_t id = msg->id;
        // if (id < 0x101 || id > 0x201)
        //     return;

        // Add to queue
        uint32_t push_pos = MessageQueue.push_pos;
        uint32_t pull_pos = MessageQueue.pull_pos;
        if (push_pos + 1 == pull_pos)
            // No space in queue
            return;
        MessageQueue.queue[push_pos % QUEUE_SIZE] = *msg;
        MessageQueue.push_pos = push_pos + 1;
    }
}

void setup()
{
    uint32_t pio_num = 2;
    uint32_t sys_clock = SYS_CLK_HZ, bitrate = 500000;
    uint32_t gpio_rx = 8, gpio_tx = 9;

    // Setup canbus
    can2040_setup(&cbus, pio_num);
    can2040_callback_config(&cbus, can2040_cb);

    // Enable irqs
    irq_set_exclusive_handler(PIO2_IRQ_0, PIOx_IRQHandler);
    irq_set_priority(PIO2_IRQ_0, 1);
    irq_set_enabled(PIO2_IRQ_0, 1);

    // Start canbus
    can2040_start(&cbus, sys_clock, bitrate, gpio_rx, gpio_tx);
}

bool read(struct can2040_msg *msg)
{
    const uint32_t push_pos = MessageQueue.push_pos;
    const uint32_t pull_pos = MessageQueue.pull_pos;

    if (pull_pos == push_pos) return false;

    (*msg) = MessageQueue.queue[pull_pos % QUEUE_SIZE];
    MessageQueue.pull_pos++;
}

int transmit_float(uint32_t id, float value)
{
    struct can2040_msg tmsg;
    tmsg.id = id; // TODO: isn't id 11 bits, why does this take 32bit?
    tmsg.dlc = sizeof(float);
    uint32_t data;
    memcpy(&data, &value, sizeof(float));
    tmsg.data32[0] = data;
    int sts = can2040_transmit(&cbus, &tmsg);

    return sts;
}

int transmit_int(uint32_t id, int32_t value)
{
    struct can2040_msg tmsg;
    tmsg.id = id; // TODO: isn't id 11 bits, why does this take 32bit?
    tmsg.dlc = sizeof(int32_t);
    uint32_t data;
    memcpy(&data, &value, sizeof(int32_t));
    tmsg.data32[0] = data;
    int sts = can2040_transmit(&cbus, &tmsg);

    return sts;
}

// int canbus_transmit_ints(uint32_t id, int32_t value1, int32_t value2)
// {
//     struct can2040_msg tmsg = {
//         .id = id,
//         .dlc = 2*sizeof(int32_t)
//     };
//     memcpy(&tmsg.data32[0], &value1, sizeof(int32_t));
//     memcpy(&tmsg.data32[1], &value2, sizeof(int32_t));
//
//     return can2040_transmit(&cbus, &tmsg);
// }

float read_float(struct can2040_msg msg) // TODO: is it possible to have a type generic for what to parse to?
{                                            // second TODO: make this memory safe?
    float value;
    memcpy(&value, msg.data, sizeof(float));

    return value;
}

int32_t read_int(struct can2040_msg msg)
{
    int32_t value;
    memcpy(&value, msg.data, sizeof(int32_t));

    return value;
}

}