/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * Provides a clock implementation for Linux/Unix systems
 */
#ifndef JPL_SYNOPSIS_LinuxClock
#define JPL_SYNOPSIS_LinuxClock

#include "Clock.hpp"


namespace Synopsis {


    /**
     * Clock implementation for Linux/Unix systems
     */
    class LinuxClock : public Clock {


        public:

            /**
             * Default virtual destructor
             */
            virtual ~LinuxClock() = default;

            /**
             * @see: Clock::get_time
             */
            double get_time(void);


    };


};


#endif
