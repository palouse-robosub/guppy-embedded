#ifndef _GUPPY_LIB_H
#define _GUPPY_LIB_H

#include "pico/stdlib.h"
#include "can2040.h"

#define MS_BETWEEN_HEARTBEATS 1000

#define ROS_TIMEOUT_DELAY_MS 10000
// no heartbeat ID assigned yet so set to 0xAAA
#define ROS_HEARTBEAT_ID 0xAAA
typedef enum {
    STARTUP = 0,
    HOLDING = 1,
    NAV = 2,
    TASK = 3,
    TELEOP = 4,
    DISABLED = 5,
    FAULT = 6
} State;

// returns if motors are allowed to run in the state
bool allowed_to_motor(State state);

// RateLimit is for things that should only run so often (after a minimum delay)
typedef struct {
    absolute_time_t time;
    int min_delay_ms;
} RateLimit;

// constructs a RateLimit with time=current-min_delay_ms and min_delay_ms
RateLimit new_rate_limit(int min_delay_ms);

// returns true if it's been more than min_delay_ms since it last returned true
bool check_rate(RateLimit *r);

// set up the can bus. TODO: add parameter support? shouldn't it all be the same?
void canbus_setup();

// reads a frame. Returns whether or not there is a frame to read
bool canbus_read(struct can2040_msg *msg);

// sends a float over CAN
int canbus_transmit_float(uint32_t id, float value); // TODO: should all return values for success be a bool or int?
int canbus_transmit_int(uint32_t id, int32_t value);

// parses can frame as a float. (assumes data is sent as little endian)
float can_read_float(struct can2040_msg msg);
int32_t can_read_int(struct can2040_msg msg);

// handles heartbeat code
void do_heartbeat(uint32_t id);

// sets up a pin for PWM and gives it an initial signal of 1500 microseconds.
void add_pwm_pin(uint pin_num);

// writes a pwm signal to a pin. It must first be set up using add_pwm_pin()
void pwm_write(uint pin_num, uint16_t level);

// converts a float from -1.0 to 1.0 into a pwm signal
int throttle_to_pwm_us(float value);

#endif // _GUPPY_LIB_H