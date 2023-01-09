#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <synopsis.hpp>
#include <SqliteASDPDB.hpp>
#include <StdLogger.hpp>
#include <LinuxClock.hpp>
#include <MaxMarginalRelevanceDownlinkPlanner.hpp>

int main(int argc, char** argv) {

    if (argc != 5) {
        std::cout << "Usage: synopsis_cli asdpdb_file rule_config_file similarity_config_file output_file" << std::endl;
        return 0;
    }

    std::string asdpdb_file(argv[1]);
    std::string rule_config_file(argv[2]);
    std::string similarity_config_file(argv[3]);
    std::string output_file(argv[4]);

    Synopsis::SqliteASDPDB db(asdpdb_file);
    Synopsis::StdLogger logger;
    Synopsis::LinuxClock clock;
    Synopsis::MaxMarginalRelevanceDownlinkPlanner planner;

    Synopsis::Application app(&db, &planner, &logger, &clock);
    Synopsis::Status status;

    status = app.init(0, NULL);
    if (status != Synopsis::Status::SUCCESS) { return status; }

    std::vector<int> prioritized_list;
    status = app.prioritize(
        rule_config_file, similarity_config_file, 1e9, prioritized_list
    );
    if (status != Synopsis::Status::SUCCESS) { return status; }

    std::vector<std::string> prioritized_uris;
    for (auto i : prioritized_list) {
        Synopsis::DpDbMsg msg;
        app.get_data_product(i, msg);
        prioritized_uris.push_back(msg.get_uri());
    }

    status = app.deinit();
    if (status != Synopsis::Status::SUCCESS) { return status; }

    std::ofstream out(output_file);
    if (out.is_open()) {
        for (auto uri : prioritized_uris) {
            out << uri << std::endl;
        }
        out.close();
    } else {
        return Synopsis::Status::FAILURE;
    }

    return Synopsis::Status::SUCCESS;
}
