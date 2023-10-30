#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <assert.h>

#include <synopsis.hpp>
#include <SqliteASDPDB.hpp>
#include <StdLogger.hpp>
#include <LinuxClock.hpp>
#include <MaxMarginalRelevanceDownlinkPlanner.hpp>

#include <nlohmann/json.hpp>

void get_json_message(Synopsis::Application &app, std::vector<int>&prioritized_list, nlohmann::json &json_msg){

    // json_msg["prioritized_list"] = prioritized_list;
    for (auto i : prioritized_list) {
        Synopsis::DpDbMsg msg;
        app.get_data_product(i, msg);
        int asdp_id = msg.get_dp_id();
        std::string instrument_name = msg.get_instrument_name();
        std::string dp_type = msg.get_type();
        std::string dp_uri = msg.get_uri();
        size_t dp_size = msg.get_dp_size();
        double science_utility_estimate = msg.get_science_utility_estimate();
        int priority_bin = msg.get_priority_bin();
        Synopsis::DownlinkState downlink_state = msg.get_downlink_state();
        Synopsis::AsdpEntry metadata = msg.get_metadata();

        // json_msg[std::to_string(i)] = {
        //     {"dp_id", i},
        //     {"instrument_name", instrument_name},
        //     {"dp_type", dp_type},
        //     {"dp_uri", dp_uri},
        //     {"dp_size", dp_size},
        //     {"science_utility_estimate", science_utility_estimate},
        //     {"priority_bin", priority_bin},
        //     {"downlink_state", downlink_state}
        // };

        // for(const auto& elem : metadata)
        // {
        //     Synopsis::DpMetadataValue mdval = elem.second;
        //     if (mdval.is_numeric()){
        //         json_msg[std::to_string(i)]["metadata"][elem.first] = mdval.get_numeric();
        //     } else {
        //         json_msg[std::to_string(i)]["metadata"][elem.first] = mdval.get_string_value();
        //     }
        // }

        // Create a DP array with DP info and metadata
        nlohmann::json dp_array = {
            {"dp_id", i},
            {"instrument_name", instrument_name},
            {"dp_type", dp_type},
            {"dp_uri", dp_uri},
            {"dp_size", dp_size},
            {"science_utility_estimate", science_utility_estimate},
            {"priority_bin", priority_bin},
            {"downlink_state", downlink_state}
        };

        for(const auto& elem : metadata)
        {
            Synopsis::DpMetadataValue mdval = elem.second;
            if (mdval.is_numeric()){
                dp_array["metadata"][elem.first] = mdval.get_numeric();
            } else {
                dp_array["metadata"][elem.first] = mdval.get_string_value();
            }
        }
        // Add the DP array to the JSON message
        json_msg += dp_array;
    }
}

int main(int argc, char** argv) {
    // Example call: ./build/synopsis_cli test/data/dd_example.db test/data/dd_example_rules.json test/data/dd_example_similarity_config.json output

    bool output_all_to_stderr = true;
    Synopsis::StdLogger logger(output_all_to_stderr), *logger_ptr;
    logger_ptr = &logger;

    // TODO: see if we can do better argument parsing, e.g. rules and similarity config files could be optional, also the order shouldn't matter
    if (argc <4 ) {
        LOG(logger_ptr, Synopsis::LogType::ERROR, "Not enough arguments. Usage (output file is optional): synopsis_cli <asdpdb_file>.db <rule_config_file>.json <similarity_config_file>.json <output>.json");
        return 0;
    }

    std::string asdpdb_file(argv[1]);
    std::string rule_config_file(argv[2]);
    std::string similarity_config_file(argv[3]);
    std::string output_file="";
    bool save_outputs = false;
    if (argc ==5){
        save_outputs = true;
        output_file = argv[4];
    } 

    Synopsis::SqliteASDPDB db(asdpdb_file);
    Synopsis::LinuxClock clock;
    Synopsis::MaxMarginalRelevanceDownlinkPlanner planner;

    Synopsis::Application app(&db, &planner, &logger, &clock);
    Synopsis::Status status;

    status = app.init(0, NULL);
    if (status != Synopsis::Status::SUCCESS) { 
        LOG(logger_ptr, Synopsis::LogType::ERROR, "Initialization failed");
        return status; 
    }
    // LOG_OBJECT(logger, Synopsis::LogType::INFO, "Status after initalization: %ld", status);
    LOG(logger_ptr, Synopsis::LogType::INFO,   "Status after initalization: %ld", status);

    std::vector<int> prioritized_list;
    status = app.prioritize(
        rule_config_file, similarity_config_file, 1e9, prioritized_list
    );
    if (status != Synopsis::Status::SUCCESS) { 
        LOG(logger_ptr, Synopsis::LogType::ERROR, "Prioritization failed");
        return status; 
    }
    LOG(logger_ptr, Synopsis::LogType::INFO,   "Status after prioritization: %ld", status);
    nlohmann::json json_msg;
    get_json_message(app, prioritized_list, json_msg);
    std::cout << "BEGIN PRIORITIZATION RESULTS" << std::endl;
    std::cout << std::setw(4) << json_msg << std::endl;
    std::cout << "END PRIORITIZATION RESULTS" << std::endl;

    // std::vector<std::string> prioritized_uris;
    // for (auto i : prioritized_list) {
    //     Synopsis::DpDbMsg msg;
    //     app.get_data_product(i, msg);
    //     prioritized_uris.push_back(msg.get_uri());
    // }

    status = app.deinit();
    if (status != Synopsis::Status::SUCCESS) { 
        LOG(logger_ptr, Synopsis::LogType::ERROR, "De-initialization failed");
        return status; 
    }
    LOG(logger_ptr, Synopsis::LogType::INFO,   "Status after de-initialization: %ld", status);

    if (save_outputs){
        std::ofstream out(output_file);
        if (out.is_open()) {
            LOG(logger_ptr, Synopsis::LogType::INFO, "Writing to output file %s", output_file.c_str());
            // for (auto uri : prioritized_uris) {
            //     out << uri << std::endl;
            // }
            // out.close();

            out << std::setw(4) << json_msg << std::endl;
            out.close();
        } else {
            LOG(logger_ptr, Synopsis::LogType::ERROR, "Could not output to file %s", output_file.c_str());
            return Synopsis::Status::FAILURE;
        }
    }

    // return Synopsis::Status::SUCCESS;
    return 0;
}
