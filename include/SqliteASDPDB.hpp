/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * Provides an Autonomous Science Data Product Database (ASDPDB) implementation
 * using a SQLite backend.
 */
#ifndef JPL_SYNOPSIS_SqliteASDPDB
#define JPL_SYNOPSIS_SqliteASDPDB
#include <string>
#include <sqlite3.h>

#include "ASDPDB.hpp"


namespace Synopsis {


    /**
     * SQLite ASDPDB Implementation
     */
    class SqliteASDPDB : public ASDPDB {


        public:

            /**
             * Constructs an ASDPDB instance
             *
             * @param[in] asdpdb_file: path to database file on disk (it will
             * be created if it does not exist)
             */
            SqliteASDPDB(std::string asdpdb_file);

            /**
             * Default virtual destructor
             */
            virtual ~SqliteASDPDB();

            /**
             * Returns the number of bytes of memory required by the module.
             *
             * @return: memory required in bytes
             */
            size_t memory_requirement(void);

            /**
             * Initializes the ASDPDB using the provided memory block
             * containing the specified number of bytes. The number of bytes
             * provided should be at least the number returned by the
             * `memory_requirement` function. This function is called by the
             * application at the time the application is initialized.
             *
             * During initialization, a connection to the database will be
             * opened, and the database schema will be created if the file did
             * not already exist.
             *
             * @param[in] bytes: number of bytes in memory block
             * @param[in] memory: pointer to memory block
             * @param[in] logger: pointer to a logger instance to be used by
             * the ASDPDB
             *
             * @return: SUCCESS if initialization was successful, or error code
             */
            Status init(size_t bytes, void* memory, Logger *logger);

            /**
             * De-initializes the ASDPDB; the provided memory block will no
             * longer be used by the module, and the connection to the database
             * will be closed.
             *
             * @return: SUCCESS if de-initialization was successful, or error
             * code
             */
            Status deinit(void);

            /**
             * @see ASDPDB::insert_data_product
             */
            Status insert_data_product(DpDbMsg& msg);

            /**
             * @see ASDPDB::get_data_product
             */
            Status get_data_product(int asdp_id, DpDbMsg& msg);

            /**
             * @see ASDPDB::list_data_product_ids
             */
            std::vector<int> list_data_product_ids(void);

            /**
             * @see ASDPDB::update_science_utility
             */
            Status update_science_utility(int asdp_id, double sue);

            /**
             * @see ASDPDB::update_priority_bin
             */
            Status update_priority_bin(int asdp_id, int bin);

            /**
             * @see ASDPDB::update_downlink_state
             */
            Status update_downlink_state(int asdp_id, DownlinkState state);

            /**
             * @see ASDPDB::update_metadata
             */
            Status update_metadata(
                int asdp_id, std::string fieldname, DpMetadataValue value);

            /**
             * Convenience function template to insert a typed value into the
             * database. This function wraps the provided value in a
             * `DpMetadataValue` instance.
             *
             * @see ASDPDB::update_metadata
             *
             * @param[in] asdp_id: ASDP for which the metadata field should be
             * udpated
             * @param[in] fieldname: the name of the field that should be updated
             * @param[in] value: new value of the field
             *
             * @return: SUCCESS if the update succeeded, or error code
             */
            template <typename T>
            Status update_metadata(int asdp_id, std::string fieldname, T value);


            /**
             * Check if DB has been initialized
             *
             *
             * @return: True if the DB has been initialized
             */
            bool is_initialized(void);


        private:

            /**
             * ASDPDB file path
             */
            std::string asdpdb_file;

            /**
             * Holds an open SQLite DB connection after initialization
             */
            sqlite3* _db;

            /**
             * Status of DB initialization
             */
            bool _initialized;

    };


};


#endif
