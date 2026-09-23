#include "guppylib/timer.hpp"

namespace guppylib
{

Timer::Timer(const uint32_t timeout_ms)
{
    timeout_ms_ = timeout_ms;
}

bool Timer::has_timed_out()
{
    const uint32_t cur_ms = to_ms_since_boot(get_absolute_time());
    if (cur_ms - last_timeout_ms_ < timeout_ms_) return false;

    last_timeout_ms_ = cur_ms;
    return true;
}

void Timer::reset()
{
    const uint32_t cur_ms = to_ms_since_boot(get_absolute_time());
    last_timeout_ms_ = cur_ms;
}

}