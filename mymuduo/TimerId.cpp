#include "TimerId.h"


TimerId::TimerId(): timerPtr_(nullptr), sequence_(0) {}

TimerId::TimerId(TimerPtr timerPtr, int64_t seq): timerPtr_(timerPtr), sequence_(seq) {}


