#ifndef GUPPY_EMBEDDED_RATELIMIT_HPP
#define GUPPY_EMBEDDED_RATELIMIT_HPP

#include <pico/stdlib.h>

namespace guppylib
{
    template <size_t MS>
    class RateLimit
    {
    private:
        absolute_time_t last_timeout_ms_;
        static constexpr uint32_t timeout_ms_ = MS;
    public:
        bool has_timeout();
        void reset();
    };

    template <size_t MS>
    bool RateLimit<MS>::has_timeout()
    {
        const uint32_t cur_ms = to_ms_since_boot(get_absolute_time());
        if (cur_ms - last_timeout_ms_ < timeout_ms_) return false;

        last_timeout_ms_ = cur_ms;
        return true;
    }

    template <size_t MS>
    void RateLimit<MS>::reset()
    {
        const uint32_t cur_ms = to_ms_since_boot(get_absolute_time());
        last_timeout_ms_ = cur_ms;
    }
}


#endif //GUPPY_EMBEDDED_RATELIMIT_HPP