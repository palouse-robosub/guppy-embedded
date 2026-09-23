#ifndef GUPPY_EMBEDDED_TIMER_HPP
#define GUPPY_EMBEDDED_TIMER_HPP

#include <pico/stdlib.h>

namespace guppylib
{

class Timer
{
private:
    absolute_time_t last_timeout_ms_;
    uint32_t timeout_ms_;
public:
    explicit Timer(uint32_t timeout_ms = 0);
    bool has_timed_out();
    void reset();
};

}


#endif //GUPPY_EMBEDDED_Timer_HPP