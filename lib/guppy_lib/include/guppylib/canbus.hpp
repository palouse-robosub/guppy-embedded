#ifndef GUPPY_EMBEDDED_CANBUS_H
#define GUPPY_EMBEDDED_CANBUS_H

#include <pico/stdlib.h>
extern "C"
{
#include "can2040.h"
}

namespace guppylib::canbus
{

// Simple example of irq safe queue (this is not multi-core safe)
#define QUEUE_SIZE 128 // Must be power of 2
static struct {
    uint32_t pull_pos;
    volatile uint32_t push_pos;
    struct can2040_msg queue[QUEUE_SIZE];
} MessageQueue;

// set up the can bus. TODO: add parameter support? shouldn't it all be the same?
void setup();

// reads a frame. Returns whether or not there is a frame to read
bool read(struct can2040_msg *msg);

// sends a float over CAN
int transmit_float(uint32_t id, float value); // TODO: should all return values for success be a bool or int?
int transmit_int(uint32_t id, int32_t value);

// parses can frame as a float. (assumes data is sent as little endian)
float read_float(struct can2040_msg msg);
int32_t read_int(struct can2040_msg msg);

}

#endif //GUPPY_EMBEDDED_CANBUS_H