#ifndef GUPPY_EMBEDDED_STATE_HPP
#define GUPPY_EMBEDDED_STATE_HPP

typedef enum {
    STARTUP = 0,
    HOLDING = 1,
    NAV = 2,
    TASK = 3,
    TELEOP = 4,
    DISABLED = 5,
    FAULT = 6
} State;

#endif //GUPPY_EMBEDDED_STATE_HPP