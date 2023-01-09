/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * Provides the generic interface for a system-specific clock.
 */
#ifndef JPL_SYNOPSIS_Clock
#define JPL_SYNOPSIS_Clock

#include "synopsis_types.hpp"


namespace Synopsis {


    /**
     * Generic clock interface
     */
    class Clock {


        public:

            /**
             * Default virtual destructor
             */
            virtual ~Clock() = default;

            /**
             * Returns time in (fractional) seconds since the beginning of some
             * arbitrary but consistent epoch. Must return a positive value.
             *
             * @return: time in seconds since some epoch
             */
            virtual double get_time(void) = 0;


    };


};


#endif
