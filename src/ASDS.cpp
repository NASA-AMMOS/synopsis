/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * @see ASDS.hpp
 */
#include <nlohmann/json.hpp>
#include <fstream>

#include "ASDS.hpp"

namespace Synopsis {


    size_t get_file_size(std::string filename) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        return file.tellg();
    }


    void ASDS::set_database(ASDPDB *db) {
        this->_db = db;
    }


    Status ASDS::submit_data_product(DpDbMsg msg) {
        if(this->_db == nullptr) {
            // TODO: Log no DB instance available
            return FAILURE;
        }
        return this->_db->insert_data_product(msg);
    }


    Status ASDS::submit_data_product(DpMsg msg) {

        size_t dp_size = get_file_size(msg.get_uri());

        // Default values
        double sue = 0.0;
        int priority_bin = 0;
        AsdpEntry metadata;

        if (msg.get_metadata_usage()) {
            // Parse metadata JSON
            std::ifstream file_input(msg.get_metadata_uri());
            auto j = nlohmann::json::parse(file_input);

            // Extract SUE
            auto j_sue = j["science_utility_estimate"];
            if (j_sue.is_number()) {
                sue = j_sue.get<double>();
            } else {
                // TODO: Log bad SUE type
            }

            // Extract Priority Bin
            auto j_bin = j["priority_bin"];
            if (j_bin.is_number_integer()) {
                priority_bin = j_bin.get<int>();
            } else {
                // TODO: Log bad bin type
            }

            auto j_meta = j["metadata"];
            if (j_meta.is_object()) {
                for (auto& el : j_meta.items()) {

                    std::string key = el.key();
                    auto val = el.value();

                    if (val.is_number_integer()) {
                        DpMetadataValue value(val.get<int>());
                        metadata.insert({key, value});

                    } else if (val.is_number()) {
                        DpMetadataValue value(val.get<double>());
                        metadata.insert({key, value});

                    } else if (val.is_string()) {
                        DpMetadataValue value(val.get<std::string>());
                        metadata.insert({key, value});

                    } else {
                        // Log unsupported metadata type
                    }

                }
            } else {
                // TODO: Log bad metadata type
            }

        } else {
            // TODO: Log warning that no metadata provided
        }

        DpDbMsg db_msg(
            -1,
            msg.get_instrument_name(),
            msg.get_type(),
            msg.get_uri(),
            dp_size,
            sue,
            priority_bin,
            DownlinkState::UNTRANSMITTED,
            metadata
        );

        return this->submit_data_product(db_msg);
    }


};
