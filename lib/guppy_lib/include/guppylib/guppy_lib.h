#ifndef _GUPPY_LIB_H
#define _GUPPY_LIB_H

#include "pico/stdlib.h"
#include "guppylib/state.hpp"

#define MS_BETWEEN_HEARTBEATS 1000

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

// handles heartbeat code
void do_heartbeat(uint32_t id);



#endif // _GUPPY_LIB_H