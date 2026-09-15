#ifndef GUPPY_EMBEDDED_CANBUS_H
#define GUPPY_EMBEDDED_CANBUS_H

#include <pico/stdlib.h>

namespace guppylib::canbus
{

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