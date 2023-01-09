/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * @see LinuxClock.hpp
 */
#include <ctime>

#include "LinuxClock.hpp"


namespace Synopsis {


    double LinuxClock::get_time(void) {
        std::time_t now;
        std::time(&now);
        return std::difftime(now, 0);
    }


};
