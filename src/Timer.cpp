/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * @see: Timer.hpp
 */
#include "Timer.hpp"


namespace Synopsis {


    Timer::Timer(Clock *clock, double duration) :
        _clock(clock),
        duration(duration),
        start_time(-1.0)
    {

    }

    void Timer::start(void) {
        this->start_time = this->_clock->get_time();
    }

    bool Timer::is_expired(void) {

        // Not expired if it hasn't been started
        if (this->start_time < 0) {
            return false;
        }

        // Check if elapsed time exceeds duration
        double now = this->_clock->get_time();
        double elapsed = now - this->start_time;
        return (elapsed >= duration);

    }


};
