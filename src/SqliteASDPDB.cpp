/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * @see SqliteASDPDB.hpp
 */
#include "SqliteASDPDB.hpp"
#include "Sqlite3Statement.hpp"
#include "synopsis_sql.hpp"


namespace Synopsis {


    SqliteASDPDB::SqliteASDPDB(std::string asdpdb_file) :
        asdpdb_file(asdpdb_file),
        _db(NULL),
        _initialized(false)
    {

    }

    SqliteASDPDB::~SqliteASDPDB() {
        this->deinit();
    }


    Status SqliteASDPDB::init(size_t bytes, void* memory, Logger *logger) {
        int status;

        this->_logger = logger;

        // Open database
        status= sqlite3_open(this->asdpdb_file.c_str(), &this->_db);
        if (status != SQLITE_OK) {
            LOG(logger, Synopsis::LogType::ERROR, "SQLite DB not opened"); 
            return FAILURE;
        }

        // Initialize schema
        status = sqlite3_exec(this->_db, SQL_SCHEMA, NULL, NULL, NULL);
        if (status != SQLITE_OK) {
            LOG(logger, Synopsis::LogType::ERROR, "SQLite DB schema not initialized");
            return FAILURE;
        }

        this->_initialized = true;
        return SUCCESS;
    }


    Status SqliteASDPDB::deinit() {
        // Close database connection
        this->_initialized = false;
        sqlite3_close(this->_db);
        return SUCCESS;
    }

    bool SqliteASDPDB::is_initialized(){
        return this->_initialized;
    }

    size_t SqliteASDPDB::memory_requirement(void) {
        return 0;
    }


    Status SqliteASDPDB::insert_data_product(DpDbMsg& msg) {
        int dp_id;

        // Begin Transaction
        sqlite3_exec(this->_db, "BEGIN", NULL, NULL, NULL);

        try {

            // Insert ASDP
            Sqlite3Statement stmt(this->_db, SQL_ASDP_INSERT);
            stmt.bind(0, msg.get_instrument_name());
            stmt.bind(1, msg.get_type());
            stmt.bind(2, msg.get_uri());
            stmt.bind(3, msg.get_dp_size());
            stmt.bind(4, msg.get_science_utility_estimate());
            stmt.bind(5, msg.get_priority_bin());
            stmt.bind(6, (int) msg.get_downlink_state());
            stmt.step();

            // Get ID of inserted ASDP
            dp_id = sqlite3_last_insert_rowid(this->_db);
            msg.set_dp_id(dp_id);

            // Insert Metadata
            for (auto const& pair : msg.get_metadata()) {
                std::string key = pair.first;
                DpMetadataValue value = pair.second;
                Sqlite3Statement stmt2(this->_db, SQL_ASDP_METADATA_INSERT);
                stmt2.bind(0, dp_id);
                stmt2.bind(1, key);
                stmt2.bind(2, (int) value.get_type());
                stmt2.bind(3, value.get_int_value());
                stmt2.bind(4, value.get_float_value());
                stmt2.bind(5, value.get_string_value());
                stmt2.step();
            }

            // Commit transaction
            sqlite3_exec(this->_db, "COMMIT", NULL, NULL, NULL);

        } catch (...) {

            // Rollback transaction and return error
            sqlite3_exec(this->_db, "ROLLBACK", NULL, NULL, NULL);

            LOG(this->_logger, Synopsis::LogType::ERROR, "Error inserting data product"); 
            return FAILURE;
        }

        return SUCCESS;
    }


    Status SqliteASDPDB::get_data_product(int asdp_id, DpDbMsg& msg) {
        
        Sqlite3Statement stmt(this->_db, SQL_ASDP_GET);

        stmt.bind(0, asdp_id);

        if (stmt.step() == SQLITE_DONE) {
            LOG(this->_logger, Synopsis::LogType::ERROR, "Data product not found");
            return FAILURE;
        }

        // Populate message with database row values
        msg.set_dp_id(stmt.fetch<int>(0));
        msg.set_instrument_name(stmt.fetch<std::string>(1));
        msg.set_type(stmt.fetch<std::string>(2));
        msg.set_uri(stmt.fetch<std::string>(3));
        msg.set_dp_size(stmt.fetch<int>(4));
        msg.set_science_utility_estimate(stmt.fetch<double>(5));
        msg.set_priority_bin(stmt.fetch<int>(6));
        msg.set_downlink_state((DownlinkState)stmt.fetch<int>(7));


        Sqlite3Statement stmt2(this->_db, SQL_ASDP_METADATA_GET);
        stmt2.bind(0, asdp_id);

        AsdpEntry metadata;
        for (int rc = stmt2.step(); rc == SQLITE_ROW; rc = stmt2.step()) {
            std::string key = stmt2.fetch<std::string>(0);
            DpMetadataValue value(
                (MetadataType)stmt2.fetch<int>(1),
                stmt2.fetch<int>(2),
                stmt2.fetch<double>(3),
                stmt2.fetch<std::string>(4)
            );
            metadata.insert({key, value});
        }


        msg.set_metadata(metadata);


        return SUCCESS;
    }


    std::vector<int> SqliteASDPDB::list_data_product_ids(void) {

        Sqlite3Statement stmt(this->_db, SQL_ASDP_SELECT);

        std::vector<int> result;
        for (int rc = stmt.step(); rc == SQLITE_ROW; rc = stmt.step()) {
            result.push_back(stmt.fetch<int>(0));
        }

        return result;
    }


    Status SqliteASDPDB::update_science_utility(int asdp_id, double sue) {
        Sqlite3Statement stmt(this->_db, SQL_UPDATE_SUE);
        stmt.bind(0, sue);
        stmt.bind(1, asdp_id);

        if (stmt.step() != SQLITE_DONE) {
            LOG(this->_logger, Synopsis::LogType::ERROR, "SQLite operation not completed while updating science utility");
            return FAILURE;
        }

        if (sqlite3_changes(this->_db) == 0) {
            LOG(this->_logger, Synopsis::LogType::ERROR, "SQLite DB not found while updating science utility");
            return FAILURE;
        }

        return SUCCESS;
    }


    Status SqliteASDPDB::update_priority_bin(int asdp_id, int bin) {
        Sqlite3Statement stmt(this->_db, SQL_UPDATE_BIN);
        stmt.bind(0, bin);
        stmt.bind(1, asdp_id);

        if (stmt.step() != SQLITE_DONE) {
            LOG(this->_logger, Synopsis::LogType::ERROR, "SQLite operation not completed while updating priority bin");
            return FAILURE;
        }

        if (sqlite3_changes(this->_db) == 0) {
            LOG(this->_logger, Synopsis::LogType::ERROR,  "SQLite DB not found while updating priority bin");
            return FAILURE;
        }

        return SUCCESS;
    }


    Status SqliteASDPDB::update_downlink_state(int asdp_id, DownlinkState state) {
        Sqlite3Statement stmt(this->_db, SQL_UPDATE_DL_STATE);
        stmt.bind(0, (int)state);
        stmt.bind(1, asdp_id);

        if (stmt.step() != SQLITE_DONE) {
            LOG(this->_logger, Synopsis::LogType::ERROR, "SQLite operation not completed while updating downlink state");
            return FAILURE;
        }

        if (sqlite3_changes(this->_db) == 0) {
            LOG(this->_logger, Synopsis::LogType::ERROR, "SQLite DB not found while updating downlink state");
            return FAILURE;
        }

        return SUCCESS;
    }


    template <typename T>
    Status SqliteASDPDB::update_metadata(int asdp_id, std::string fieldname, T value) {
        return this->update_metadata(asdp_id, fieldname, DpMetadataValue(value));
    }


    Status SqliteASDPDB::update_metadata(
            int asdp_id, std::string fieldname, DpMetadataValue value) {

        Sqlite3Statement stmt(this->_db, SQL_UPDATE_METADATA);
        stmt.bind(0, value.get_type());
        stmt.bind(1, value.get_int_value());
        stmt.bind(2, value.get_float_value());
        stmt.bind(3, value.get_string_value());
        stmt.bind(4, asdp_id);
        stmt.bind(5, fieldname);

        if (stmt.step() != SQLITE_DONE) {
            LOG(this->_logger, Synopsis::LogType::ERROR, "SQLite operation not completed while updating metadata");
            return FAILURE;
        }

        if (sqlite3_changes(this->_db) == 0) {
            LOG(this->_logger, Synopsis::LogType::ERROR, "SQLite DB not found while updating metadata");
            return FAILURE;
        }

        return SUCCESS;
    }


};
