/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * Provides a generic abstract interface for Autonomous Science Data System
 * (ASDS) implementations.
 */
#ifndef JPL_SYNOPSIS_ASDS
#define JPL_SYNOPSIS_ASDS

#include "synopsis_types.hpp"
#include "DpMsg.hpp"
#include "DpDbMsg.hpp"
#include "ApplicationModule.hpp"
#include "ASDPDB.hpp"


namespace Synopsis {


    /**
     * Utility function to get the size of an ASDP
     *
     * @param filename: ASDP file path
     *
     * @return: file size in bytes
     */
    size_t get_file_size(std::string filename);


    /*
     * Base class for Autonomous Science Data System (ASDS) implementations
     */
    class ASDS : public ApplicationModule {


        public:

            /**
             * Default virtual destructor
             */
            virtual ~ASDS() = default;

            /**
             * Submits a data product to the ASDS for processing. This function
             * is called by the SYNOPSIS application, and should be implemented
             * within the child class to process instrument- and type-specific
             * data.
             *
             * For time and resource-intensive processing, ASDS implementations
             * should return quickly from this function call and enqueue the
             * data product for asynchronous processing.
             *
             * @param[in] msg: message containing data product information
             *
             * @return: data processing status code
             */
            virtual Status process_data_product(DpMsg msg) = 0;

            /**
             * Sets the database instance to be used by this ASDS for
             * submission. This function is called by the SYNOPSIS application
             * when the ASDS is registered, prior to initialization.
             *
             * @param[in] db: pointer to an ASDP database instance
             */
            void set_database(ASDPDB *db);


        protected:

            /**
             * Submits a data product to the ASDP database after it has been
             * processed. This function handles data product messages in a raw
             * format, which may contain pointers to metadata on the filesystem.
             *
             * @param[in] msg: message containing ASDS information
             *
             * @return: SUCCESS if the data product was successfully inserted
             * into the database, or error code
             */
            Status submit_data_product(DpMsg msg);

            /**
             * Submits a data product to the ASDP database after it has been
             * processed. This function handles data product messages that have
             * already been parsed into a format suitable for direct insertion
             * into the database.
             *
             * @param[in] msg: message containing ASDS information
             *
             * @return: SUCCESS if the data product was successfully inserted
             * into the database, or error code
             */
            Status submit_data_product(DpDbMsg msg);


        private:

            /**
             * Reference to the database instance to be used by this ASDS
             */
            ASDPDB *_db = nullptr;


    };


};


#endif
