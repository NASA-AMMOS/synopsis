/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * @see DownlinkPlanner.hpp
 */
#include "DownlinkPlanner.hpp"

namespace Synopsis {


    void DownlinkPlanner::set_database(ASDPDB *db) {
        this->_db = db;
    }


    void DownlinkPlanner::set_clock(Clock *clock) {
        this->_clock = clock;
    }


};
