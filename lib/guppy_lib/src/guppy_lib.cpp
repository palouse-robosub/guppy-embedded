#include "hardware/pwm.h"
#include <string.h>

extern "C"
{
#include "can2040.h"
}

#include "guppylib/guppy_lib.h"
#include "guppylib/canbus.hpp"


bool allowed_to_motor(State state)
{
    return state == HOLDING
           || state == NAV
           || state == TASK
           || state == TELEOP;
}

RateLimit new_rate_limit(int min_delay_ms) {
    return (RateLimit) { .time = get_absolute_time() - min_delay_ms*1000, .min_delay_ms = min_delay_ms };
}

bool check_rate(RateLimit* r)
{
    absolute_time_t current_time = get_absolute_time();
    if (absolute_time_diff_us(r->time, current_time)/1000 > r->min_delay_ms) {
        r->time = current_time;
        return true;
    }
    return false;
}


static uint32_t last_heartbeat_time = 0;

void do_heartbeat(uint32_t id)
{
    uint32_t cur_time = to_ms_since_boot(get_absolute_time());
    
    if (cur_time - last_heartbeat_time > MS_BETWEEN_HEARTBEATS)
    {
        last_heartbeat_time = cur_time;
        guppylib::canbus::transmit_int(id, cur_time);
    }
}
