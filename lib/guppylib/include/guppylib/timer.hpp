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
    /**
     * @brief Construct a new Timer object, automatically reset to 0ms.
     * 
     * @param timeout_ms The ms for timer duration
     */
    explicit Timer(uint32_t timeout_ms = 0);

    /**
     * @brief Checks if the timer has reached a specified time (`timeout_ms` passed in constructor).
     * Also resets the timer if true.
     * @return boolean
     */
    bool check();

    /**
     * @brief Resets the timer to 0ms.
     * 
     */
    void reset();
};

}


#endif //GUPPY_EMBEDDED_Timer_HPP