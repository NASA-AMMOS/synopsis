/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * Provides the generic interface for a SYNOPSIS downlink planner.
 */
#ifndef JPL_SYNOPSIS_DownlinkPlanner
#define JPL_SYNOPSIS_DownlinkPlanner

#include <vector>

#include "synopsis_types.hpp"
#include "ApplicationModule.hpp"
#include "ASDPDB.hpp"
#include "Clock.hpp"


namespace Synopsis {


    /**
     * Abstarct base class for a downlink planner algorithm
     */
    class DownlinkPlanner : public ApplicationModule {


        public:

            /**
             * Default virtual destructor
             */
            virtual ~DownlinkPlanner() = default;

            /**
             * Sets the database instance to be used by this downlink planner.
             * This function is called by the SYNOPSIS application during
             * initialization, with the application providing its instance.
             *
             * @param[in] db: pointer to an ASDPDB instance
             */
            void set_database(ASDPDB *db);

            /**
             * Sets the clock instance to be used by this downlink planner.
             * This function is called by the SYNOPSIS application during
             * initialization, with the application providing its instance.
             *
             * @param[in] clock: pointer to a clock instance
             */
            void set_clock(Clock *clock);

            /**
             * Abstract prioritization algorithm interface to be implemented by
             * a child class. This function is invoked by the SYNOPSIS
             * application during prioritization.
             *
             * @see Application::prioritize
             *
             * @param[in] rule_configuration_id: rule and constraint
             * configuration (e.g., URI of JSON on filesystem)
             * @param[in] similarity_configuration_id: similarity-based
             * discount configuration (e.g., URI of JSON on filesystem)
             * @param[in] max_processing_time_sec: the prioritization algorithm
             * should time-out after this amount of time has passed
             * @param[out] prioritized_list: this list will be populated with a
             * prioritized list of ASDPs, specified using their IDs
             *
             * @return: SUCCESS if prioritization completed, TIMEOUT if the
             * time expired, or other error code upon failure
             */
            virtual Status prioritize(
                std::string rule_configuration_id,
                std::string similarity_configuration_id,
                double max_processing_time_sec,
                std::vector<int> &prioritized_list
            ) = 0;


        protected:

            /**
             * Reference to the ASDP database instance to be used by this
             * downlink planner.
             */
            ASDPDB *_db = nullptr;

            /**
             * Reference to the clock instance to be used by this downlink
             * planner.
             */
            Clock *_clock = nullptr;


    };


};


#endif
