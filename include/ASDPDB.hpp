/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * Provides a generic abstract interface for an Autonomous Science Data Product
 * Database (ASDPDB).
 */
#ifndef JPL_SYNOPSIS_ASDPDB
#define JPL_SYNOPSIS_ASDPDB
#include <vector>

#include "synopsis_types.hpp"
#include "ApplicationModule.hpp"
#include "DpDbMsg.hpp"


namespace Synopsis {


    /**
     * Base class for Autonomous Science Data Product Database (ASDPDB)
     * implementations
     */
    class ASDPDB : public ApplicationModule {


        public:

            /**
             * Default virtual destructor
             */
            virtual ~ASDPDB() = default;

            /**
             * Inserts data product information into the database.
             *
             * @param[in] msg: message containing data product information
             *
             * @return: SUCCESS if successfully inserted, or error code
             */
            virtual Status insert_data_product(DpDbMsg& msg) = 0;

            /**
             * Fetches data product information from the database corresponding
             * to a specific ASDP.
             *
             * @param[in] asdp_id: ASDP for which information should be fetched
             * @param[out] msg: message that will be populated with ASDP
             * information
             *
             * @return: SUCCESS if successfully fetched, or error code
             */
            virtual Status get_data_product(int asdp_id, DpDbMsg& msg) = 0;

            /**
             * Returns a list of all ASDPs within the database.
             *
             * @return: list of ASDP identifiers
             */
            virtual std::vector<int> list_data_product_ids(void) = 0;

            /**
             * Manually update the science utility estimate of a specific data
             * product.
             *
             * @param[in] asdp_id: ASDP for which the science utility estimate
             * should be udpated
             * @param[in] sue: new science utility estimate value
             *
             * @return: SUCCESS if the update succeeded, or error code
             */
            virtual Status update_science_utility(int asdp_id, double sue) = 0;

            /**
             * Manually update the priority bin of a specific data product.
             *
             * @param[in] asdp_id: ASDP for which the priority bin should be
             * udpated
             * @param[in] bin: new priority bin
             *
             * @return: SUCCESS if the update succeeded, or error code
             */
            virtual Status update_priority_bin(int asdp_id, int bin) = 0;

            /**
             * Manually update the downlink state of a specific data product.
             *
             * @param[in] asdp_id: ASDP for which the downlink state should be
             * udpated
             * @param[in] bin: new downlink state
             *
             * @return: SUCCESS if the update succeeded, or error code
             */
            virtual Status update_downlink_state(
                int asdp_id, DownlinkState state) = 0;

            /**
             * Manually update an arbitrary metadata field of a specific data
             * product. The specified field must already exist.
             *
             * @param[in] asdp_id: ASDP for which the metadata field should be
             * udpated
             * @param[in] fieldname: the name of the field that should be updated
             * @param[in] value: new value of the field
             *
             * @return: SUCCESS if the update succeeded, or error code
             */
            virtual Status update_metadata(
                int asdp_id, std::string fieldname, DpMetadataValue value) = 0;


            /**
             * Check if DB has been initialized
             *
             *
             * @return: True if the DB has been initialized
             */
            virtual bool is_initialized(void) = 0;


    };


};


#endif
