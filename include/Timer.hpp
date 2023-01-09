/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * Implements a timer using a generic Clock implementation.
 *
 * @see: Clock.hpp
 */
#ifndef JPL_SYNOPSIS_Timer
#define JPL_SYNOPSIS_Timer

#include "synopsis_types.hpp"
#include "Clock.hpp"


namespace Synopsis {


    class Timer {


        public:

            /**
             * Constructor based on a clock instance and timer duration
             *
             * @param[in] clock: clock instance to use for timing
             * @param[in] duration: desired duration of the timer
             */
            Timer(Clock *clock, double duration);

            /**
             * Default destructor
             */
            ~Timer() = default;

            /**
             * Starts or restarts the timer.
             */
            void start(void);

            /**
             * Check whether the timer has expired, given the specified
             * duration during construction. Returns `false` if the timer has
             * not been started.
             *
             * @return: `true` if `duration` has elapsed since the last time
             * the timer was started, or `false` otherwise
             */
            bool is_expired(void);


        private:

            /**
             * Holds a pointer to the clock instance
             */
            Clock *_clock;

            /**
             * The desired duration of the timer
             */
            double duration;

            /**
             * The most recent start time of the timer, as provided by the
             * clock instance
             */
            double start_time;


    };


};


#endif
