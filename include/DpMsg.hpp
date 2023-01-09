/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * Defines a class to hold data product messages. This message format is used
 * to add new data products to SYNOPSIS.
 */
#ifndef JPL_SYNOPSIS_DpMsg
#define JPL_SYNOPSIS_DpMsg
#include <string>


namespace Synopsis {


    class DpMsg {


        public:

            /**
             * Default constructor creates an empty message
             */
            DpMsg();

            /**
             * Default destructor
             */
            ~DpMsg() = default;

            /**
             * Constructs a data product message
             *
             * @param[in] instrument_name: data product instrument name
             * @param[in] dp_type: data product type
             * @param[in] dp_uri: location of the data product on the
             * filesystem or other storage system
             * @param[in] meta_uri: location of the data product metadata on
             * the filesystem or other storage system (optional)
             * @param[in] meta_usage: indicates whether the optional meta_uri
             * field should be used
             */
            DpMsg(std::string instrument_name, std::string dp_type,
                std::string dp_uri, std::string meta_uri, bool meta_usage);

            /**
             * @return: data product instrument name
             */
            std::string get_instrument_name(void);

            /**
             * @return: data product type
             */
            std::string get_type(void);

            /**
             * @return: data product URI
             */
            std::string get_uri(void);

            /**
             * @return: data product metadata URI
             */
            std::string get_metadata_uri(void);

            /**
             * @return: data product metadata URI usage flag
             */
            bool get_metadata_usage(void);


        private:

            /**
             * Stores data product instrument name
             */
            std::string instrument_name;

            /**
             * Stores data product type
             */
            std::string dp_type;

            /**
             * Stores data product URI
             */
            std::string dp_uri;

            /**
             * Stores data product metadata URI
             */
            std::string meta_uri;

            /**
             * Stores data product metadata URI usage flag
             */
            bool meta_usage;


    };


};


#endif
