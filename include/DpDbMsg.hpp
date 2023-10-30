/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * Defines a class to hold Autonomous Science Data Product Database (ASDPDB)
 * messages. This message format is used internally by SYNOPSIS for inserting
 * new ASDPDB entries, or querying information from the ASDPDB.
 */
#ifndef JPL_SYNOPSIS_DpDbMsg
#define JPL_SYNOPSIS_DpDbMsg
#include <string>
#include <map>
#include <vector>

#include "synopsis_types.hpp"


namespace Synopsis {


    /**
     * Class for storing an ASDPDB metadata value that can take one of three
     * types:
     *   - INTEGER (int)
     *   - FLOAT (double)
     *   - STRING (std::string)
     */
    class DpMetadataValue {


        public:

            /**
             * Default constructor creates a value
             */
            DpMetadataValue();

            /**
             * Default destructor
             */
            ~DpMetadataValue() = default;

            /**
             * INTEGER value constructor
             *
             * @param[in] integer_value
             */
            DpMetadataValue(int integer_value);

            /**
             * FLOAT value constructor
             *
             * @param[in] float_value
             */
            DpMetadataValue(double float_value);

            /**
             * STRING value constructor
             *
             * @param[in] string_value
             */
            DpMetadataValue(std::string string_value);

            /**
             * Generic value constructor. Each instance can store all three
             * types; only the type corresponding to the `type` argument is
             * valid.
             *
             * @param[in] type: metadata value type
             * @param[in] integer_value
             * @param[in] float_value
             * @param[in] string_value
             */
            DpMetadataValue(MetadataType type,
                int integer_value, double float_value, std::string string_value);

            /**
             * @return: metadata value type
             */
            MetadataType get_type(void);

            /**
             * Returns integer metadata value.
             *
             * Warning: the user must check the type of this value before
             * calling this function. If the value does not have an integer
             * type, the behavior of this function is undefined.
             *
             * @return: integer metadata value
             */
            int get_int_value(void);

            /**
             * Returns float metadata value.
             *
             * Warning: the user must check the type of this value before
             * calling this function. If the value does not have a float type,
             * the behavior of this function is undefined.
             *
             * @return: float metadata value
             */
            double get_float_value(void);

            /**
             * Returns string metadata value.
             *
             * Warning: the user must check the type of this value before
             * calling this function. If the value does not have a string type,
             * the behavior of this function is undefined.
             *
             * @return: string metadata value
             */
            std::string get_string_value(void);

            /**
             * Checks if the metadata value has a numeric type; that is,
             * INTEGER or FLOAT.
             *
             * @return: whether the metadata has a numeric type
             */
            bool is_numeric(void);

            /**
             * Returns numeric metadata value; integer values are cast to
             * double-precision floating point.
             *
             * Warning: the user must check the type of this value before
             * calling this function. If the value does not have a numeric
             * type, the behavior of this function is undefined.
             *
             * @return: numeric value as a double
             */
            double get_numeric(void);


        private:

            /**
             * Stores the metadata value type
             */
            MetadataType type;

            /**
             * Stores integer-valued metadata
             */
            int int_value;

            /**
             * Stores float-valued metadata
             */
            double float_value;

            /**
             * Stores string-valued metadata
             */
            std::string string_value;


    };


    /**
     * Alias for an ASDP entry, with a map to hold metadata values indexed by
     * field name.
     */
    using AsdpEntry = std::map<std::string, DpMetadataValue>;

    /**
     * Alias for a list (vector) of ASDP entries.
     */
    using AsdpList = std::vector<AsdpEntry>;

    /**
     * Alias for an assignment of ASDP entries to variable names
     */
    using AsdpAssignments = std::map<std::string, AsdpEntry>;


    /**
     * Class for storing messages used to interact with an ASDPDB
     */
    class DpDbMsg {


        public:


            /**
             * Constructs an empty message
             */
            DpDbMsg();

            /**
             * Default destructor
             */
            ~DpDbMsg() = default;

            /**
             * Constructs an ASDPDB message
             *
             * @param[in] dp_id: unique ASDP identifier
             * @param[in] instrument_name: ASDP instrument name
             * @param[in] dp_type: ASDP type
             * @param[in] dp_uri: location of the ASDP on the filesystem or
             * other storage system
             * @param[in] dp_size: size of the ASDP in bytes
             * @param[in] science_utility_estimate: ASDP utility estimate
             * @param[in] priority_bin: ASDP priority bin
             * @param[in] downlink_state: ASDP downlink state
             * @param[in] metadata: mapping of ASDP metadata field names to
             * values
             */
            DpDbMsg(int dp_id, std::string instrument_name, std::string dp_type,
                std::string dp_uri, size_t dp_size, double science_utility_estimate,
                int priority_bin, DownlinkState downlink_state,
                AsdpEntry metadata);

            /**
             * @return: ASDP identifier
             */
            int get_dp_id(void);

            /**
             * @return: ASDP instrument name
             */
            std::string get_instrument_name(void);

            /**
             * @return: ASDP type
             */
            std::string get_type(void);

            /**
             * @return: ASDP URI
             */
            std::string get_uri(void);

            /**
             * @return: ASDP size in bytes
             */
            size_t get_dp_size(void);

            /**
             * @return: ASDP utility estimate
             */
            double get_science_utility_estimate(void);

            /**
             * @return: ASDP priority bin
             */
            int get_priority_bin(void);

            /**
             * @return: ASDP downlink state
             */
            DownlinkState get_downlink_state(void);

            /**
             * @return: mapping of ASDP metadata field names to values
             */
            AsdpEntry get_metadata(void);


            /**
             * Set ASDP identifier
             *
             * @param[in] id: ASDP identifier
             */
            void set_dp_id(int id);

            /**
             * Set ASDP instrument name
             *
             * @param[in] name: ASDP instrument name
             */
            void set_instrument_name(std::string name);

            /**
             * Set ASDP type
             *
             * @param[in] type: ASDP type
             */
            void set_type(std::string type);

            /**
             * Set ASDP URI
             *
             * @param[in] uri: ASDP URI
             */
            void set_uri(std::string uri);

            /**
             * Set ASDP size
             *
             * @param[in] size: ASDP size in bytes
             */
            void set_dp_size(size_t size);

            /**
             * Set ASDP utility estimate
             *
             * @param[in] sue: ASDP utility estimate
             */
            void set_science_utility_estimate(double sue);

            /**
             * Set ASDP priority bin
             *
             * @param[in] bin: ASDP priority bin
             */
            void set_priority_bin(int bin);

            /**
             * Set ASDP downlink state
             *
             * @param[in] state: ASDP downlink state
             */
            void set_downlink_state(DownlinkState state);

            /**
             * Set ASDP metadata values
             *
             * @param[in] metadata: mapping of ASDP metadata field names to
             * values
             */
            void set_metadata(AsdpEntry metadata);


        private:

            /**
             * Stores ASDP identifier
             */
            int dp_id;

            /**
             * Stores ASDP instrument name
             */
            std::string instrument_name;

            /**
             * Stores ASDP type
             */
            std::string dp_type;

            /**
             * Stores ASDP URI
             */
            std::string dp_uri;

            /**
             * Stores ASDP size in bytes
             */
            size_t dp_size;

            /**
             * Stores ASDP utility estimate
             */
            double science_utility_estimate;

            /**
             * Stores ASDP priority bin
             */
            int priority_bin;

            /**
             * Stores ASDP downlink state
             */
            DownlinkState downlink_state;

            /**
             * Stores mapping of ASDP metadata field names to values
             */
            AsdpEntry metadata;


    };


};


#endif
