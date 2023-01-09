/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * Provides a "pass through" Autonomous Science Data System (ASDS)
 * implementation. All data products provided to this ASDS are simply forwarded
 * to the ASDPDB without further processing. This ASDS is useful for scenarios
 * in which ASDPs are produced directly by an instrument.
 */
#ifndef JPL_SYNOPSIS_PassthroughASDS
#define JPL_SYNOPSIS_PassthroughASDS

#include "ASDS.hpp"


namespace Synopsis {


    class PassthroughASDS : public ASDS {


        public:

            /**
             * Default virtual destructor
             */
            virtual ~PassthroughASDS() = default;

            /**
             * @see ApplicationModule::memory_requirement
             */
            size_t memory_requirement(void);

            /**
             * @see ApplicationModule::init
             */
            Status init(size_t bytes, void* memory, Logger *logger);

            /**
             * @see ApplicationModule::deinit
             */
            Status deinit(void);

            /**
             * @see ASDS::process_data_product
             *
             * This implementation forwards the message along to the ASDPDB
             * without further processing.
             */
            Status process_data_product(DpMsg msg);


    };


};


#endif
