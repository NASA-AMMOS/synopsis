/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * Defines SQL queries used by the SqliteASDPDB implementation.
 *
 * @see SqliteASDPDB.hpp
 */
#ifndef JPL_SYNOPSIS_SQL
#define JPL_SYNOPSIS_SQL


namespace Synopsis {


    /**
     * ASDPDB schema; defines two tables to hold ASDPs and associated metadata
     *
     * @see SqliteASDPDB::init
     */
    static constexpr const char* SQL_SCHEMA = R"(

    CREATE TABLE IF NOT EXISTS ASDP (
        asdp_id INTEGER PRIMARY KEY,
        instrument_name TEXT,
        type TEXT,
        uri TEXT,
        size INTEGER,
        science_utility_estimate REAL,
        priority_bin INTEGER,
        downlink_state INTEGER
    );

    CREATE TABLE IF NOT EXISTS METADATA (
        asdp_id INTEGER,
        fieldname TEXT NOT NULL,
        type INTEGER,
        value_int INTEGER,
        value_float REAL,
        value_string TEXT,
        FOREIGN KEY(asdp_id) REFERENCES ASDP(asdp_id)
        CONSTRAINT UNIQUE_META UNIQUE (asdp_id,fieldname)
    );

    )";

    /**
     * Defines query to insert a new ASDP
     *
     * @see SqliteASDPDB::insert_data_product
     */
    static constexpr const char* SQL_ASDP_INSERT = R"(

    INSERT INTO ASDP (
        instrument_name, type, uri, size,
        science_utility_estimate, priority_bin, downlink_state
    ) VALUES (?, ?, ?, ?, ?, ?, ?);

    )";

    /**
     * Defines query to insert new ASDP metadata
     *
     * @see SqliteASDPDB::insert_data_product
     */
    static constexpr const char* SQL_ASDP_METADATA_INSERT = R"(

    INSERT INTO METADATA (
        asdp_id, fieldname, type, value_int, value_float, value_string
    ) VALUES (?, ?, ?, ?, ?, ?);

    )";

    /**
     * Defines query to fetch all ASDP ids
     *
     * @see SqliteASDPDB::list_data_product_ids
     */
    static constexpr const char* SQL_ASDP_SELECT = R"(

    SELECT asdp_id FROM ASDP;

    )";

    /**
     * Defines query to fetch a specific ASDP
     *
     * @see SqliteASDPDB::get_data_product
     */
    static constexpr const char* SQL_ASDP_GET = R"(

    SELECT
        asdp_id, instrument_name, type, uri, size,
        science_utility_estimate, priority_bin, downlink_state
    FROM ASDP WHERE asdp_id=?;

    )";

    /**
     * Defines query to fetch metadata for a specific ASDP
     *
     * @see SqliteASDPDB::get_data_product
     */
    static constexpr const char* SQL_ASDP_METADATA_GET = R"(

    SELECT
        fieldname, type, value_int, value_float, value_string
    FROM METADATA WHERE asdp_id=?;

    )";

    /**
     * Defines query to update the science utility estimate of an ASDP
     *
     * @see SqliteASDPDB::update_science_utility
     */
    static constexpr const char* SQL_UPDATE_SUE = R"(

    UPDATE ASDP
    SET science_utility_estimate=?
    WHERE asdp_id=?;

    )";

    /**
     * Defines query to update the priority bin of an ASDP
     *
     * @see SqliteASDPDB::update_priority_bin
     */
    static constexpr const char* SQL_UPDATE_BIN = R"(

    UPDATE ASDP
    SET priority_bin=?
    WHERE asdp_id=?;

    )";

    /**
     * Defines query to update the downlink state of an ASDP
     *
     * @see SqliteASDPDB::update_downlink_state
     */
    static constexpr const char* SQL_UPDATE_DL_STATE = R"(

    UPDATE ASDP
    SET downlink_state=?
    WHERE asdp_id=?;

    )";

    /**
     * Defines query to update the metadata field of an ASDP
     *
     * @see SqliteASDPDB::update_metadata
     */
    static constexpr const char* SQL_UPDATE_METADATA = R"(

    UPDATE METADATA
    SET type=?, value_int=?, value_float=?, value_string=?
    WHERE asdp_id=? AND fieldname=?;

    )";


};

#endif
