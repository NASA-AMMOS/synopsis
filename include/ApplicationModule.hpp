/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * Provides the generic interface for a SYNOPSIS application module. Example
 * modules include ASDSs, the ASDP DB, downlink planner, etc.
 */
#ifndef JPL_SYNOPSIS_ApplicationModule
#define JPL_SYNOPSIS_ApplicationModule

#include "synopsis_types.hpp"
#include "Logger.hpp"


namespace Synopsis {


    /**
     * Provides the generic interface for SYNOPSIS application modules.
     */
    class ApplicationModule {


        public:

            /**
             * Default virtual destructor
             */
            virtual ~ApplicationModule() = default;

            /**
             * Returns the number of bytes of memory required by the module.
             * This is called by the application to determine the memory needed
             * by each of its modules.
             *
             * @return: memory required in bytes
             */
            virtual size_t memory_requirement(void) = 0;

            /**
             * Initializes the SYNOPSIS application module using the provided
             * memory block containing the specified number of bytes. The
             * number of bytes provided should be at least the number returned
             * by the `memory_requirement` function. This function is called by
             * the application at the time the application is initialized.
             *
             * @param[in] bytes: number of bytes in memory block
             * @param[in] memory: pointer to memory block
             * @param[in] logger: pointer to a logger instance to be used by
             * the module
             *
             * @return: SUCCESS if initialization was successful, or error code
             */
            virtual Status init(size_t bytes, void* memory, Logger *logger) = 0;

            /**
             * De-initializes the SYNOPSIS application module; the provided
             * memory block will no longer be used by the module.
             *
             * @return: SUCCESS if de-initialization was successful, or error
             * code
             */
            virtual Status deinit() = 0;


        protected:

            /**
             * Reference to the logger instance to be used by this module
             */
            Logger *_logger = nullptr;


    };


};


#endif
