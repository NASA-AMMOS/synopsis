/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * The main public SYNOPSIS interface. SYNOPSIS is instantiated as an
 * `Application`, which must be initialized prior to use, and de-initialized
 * after use.
 */
#ifndef JPL_SYNOPSIS_PUBLIC
#define JPL_SYNOPSIS_PUBLIC
#include <string>
#include <tuple>
#include <vector>

#include "synopsis_types.hpp"
#include "DpMsg.hpp"
#include "DpDbMsg.hpp"
#include "ApplicationModule.hpp"
#include "ASDS.hpp"
#include "ASDPDB.hpp"
#include "PassthroughASDS.hpp"
#include "Logger.hpp"
#include "Clock.hpp"
#include "DownlinkPlanner.hpp"

/**
 * Maximum number of ASDSs that can be registered to an application instance
 */
#define MAX_SYNOPSIS_APP_ASDS 32

/**
 * Word size for memory alignment
 */
#define MEM_ALIGN_SIZE 8

namespace Synopsis {


    /**
     * Defines an instance of the SYNOPSIS `Application`
     */
    class Application {


        public:

            /**
             * Application constructor
             *
             * @param[in] db: pointer to an ASDP database instance
             * @param[in] planner: pointer to a downlink planner instance
             * @param[in] logger: pointer to a system-specific logger instance
             * @param[in] clock: pointer to a system-specific clock instance
             */
            Application(ASDPDB *db, DownlinkPlanner *planner, Logger *logger, Clock *clock);

            /**
             * Default destructor
             */
            ~Application() = default;

            /**
             * Returns the number of bytes of memory required by the
             * application and its modules. Should be called prior to
             * initialization to provided an appropriately sized memory block
             * to the application.
             *
             * @return: memory required in bytes
             */
            size_t memory_requirement(void);

            /**
             * Register an ASDS with SYNOPSIS. The ASDS will process any data
             * product associated with the specified instrument. This function
             * should be called prior to application initialization.
             *
             * @param[in] instrument_name: process all data products from this
             * instrument
             * @param[in] asds: pointer to an ASDS instance
             *
             * @return: SUCCESS if registration was successful, or error code
             */
            Status add_asds(std::string instrument_name, ASDS *asds);

            /**
             * Register an ASDS with SYNOPSIS. The ASDS will process any data
             * product associated with the specified instrument and type pair.
             * This function should be called prior to application
             * initialization.
             *
             * @param[in] instrument_name: process all data products from this
             * instrument
             * @param[in] dp_type: process all data products from the specified
             * instrument with this type
             * @param[in] asds: pointer to an ASDS instance
             *
             * @return: SUCCESS if registration was successful, or error code
             */
            Status add_asds(std::string instrument_name, std::string dp_type,
                ASDS *asds);

            /**
             * Initializes the SYNOPSIS application using the provided memory
             * block containing the specified number of bytes. The number of
             * bytes provided should be at least the number returned by the
             * `memory_requirement` function.
             *
             * @param[in] bytes: number of bytes in memory block
             * @param[in] memory: pointer to memory block
             *
             * @return: SUCCESS if initialization was successful, or error code
             */
            Status init(size_t bytes, void* memory);

            /**
             * De-initializes the SYNOPSIS application; the provided memory
             * block will no longer be used by the application.
             *
             * @return: SUCCESS if de-initialization was successful, or error
             * code
             */
            Status deinit(void);

            /**
             * This function is called to accept an incoming data product
             * message to be ingested and processed by an ASDS. The instrument
             * name and data product type in the message will be used to route
             * the data product to the appropriate ASDS.
             *
             * @param[in] msg: data product message instance
             *
             * @return: SUCCESS if message was successfully accepted, or error
             */
            Status accept_dp(DpMsg msg);

            /**
             * Updates the science utility estimate of an ASDP, to be called in
             * response to a ground-commanded update.
             *
             * @param[in] asdp_id: id of the ASDP to update
             * @param[in] sue: new science utility estimate value
             *
             * @return: SUCCESS if successfully updated, or error
             */
            Status update_science_utility(int asdp_id, double sue);

            /**
             * Updates the priority bin of an ASDP, to be called in response to
             * a ground-commanded update.
             *
             * @param[in] asdp_id: id of the ASDP to update
             * @param[in] bin: new priority bin
             *
             * @return: SUCCESS if successfully updated, or error
             */
            Status update_priority_bin(int asdp_id, int bin);

            /**
             * Updates the downlink state of an ASDP, to be called in response to
             * a ground-commanded update or after an ASDP is transmitted by the
             * downlink system.
             *
             * @param[in] asdp_id: id of the ASDP to update
             * @param[in] state: new downlink state
             *
             * @return: SUCCESS if successfully updated, or error
             */
            Status update_downlink_state(int asdp_id, DownlinkState state);

            /**
             * Updates an arbitrary metadata field of an ASDP, to be called in
             * response to a ground-commanded update.
             *
             * @param[in] asdp_id: id of the ASDP to update
             * @param[in] fieldname: metadata field name
             * @param[in] vaule: new metadata field value
             *
             * @return: SUCCESS if successfully updated, or error
             */
            template <typename T>
            Status update_asdp_metadata(int asdp_id, std::string fieldname, T value);

            /**
             * Returns a list of all ASDPs within the database.
             *
             * @return: list of ASDP identifiers
             */
            std::vector<int> list_data_product_ids(void);

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
            Status get_data_product(int asdp_id, DpDbMsg& msg);

            /**
             * Prioritize the data products in the ASDP DB, given a set of
             * rules and configuration for similarity-based discounts.
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
            Status prioritize(
                std::string rule_configuration_id,
                std::string similarity_configuration_id,
                double max_processing_time_sec,
                std::vector<int> &prioritized_list
            );


        private:

            /**
             * Size of the memory buffer held by this application.
             */
            size_t buffer_size;

            /**
             * Reference to memory buffer held by this application.
             */
            void *memory_buffer;

            /**
             * Reference to the ASDP database instance to be used by this
             * application.
             */
            ASDPDB *_db;

            /**
             * Reference to the downlink planner instance to be used by this
             * application.
             */
            DownlinkPlanner *_planner;

            /**
             * Reference to the logger instance to be used by this application.
             */
            Logger *_logger;

            /**
             * Reference to the clock instance to be used by this application.
             */
            Clock *_clock;

            /**
             * The number of ASDS instances registered to the application.
             */
            int n_asds;

            /**
             * An array of tuples holding pointers to the ASDSs associated with
             * each instrument and data type pair.
             */
            std::tuple<std::string, std::string, ASDS*> asds[MAX_SYNOPSIS_APP_ASDS];

            /**
             * Returns the number of padding bytes needed to word-align
             * requests for memory blocks.
             *
             * @param[in] block_size: requested block size
             *
             * @return: padding size in bytes
             */
            static size_t padding_nbytes(size_t block_size);


    };


};


#endif
