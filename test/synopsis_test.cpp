#include <gtest/gtest.h>
#include <cmath>

#include <synopsis.hpp>
#include <SqliteASDPDB.hpp>
#include <StdLogger.hpp>
#include <LinuxClock.hpp>
#include <Timer.hpp>
#include <RuleAST.hpp>
#include <MaxMarginalRelevanceDownlinkPlanner.hpp>


class TestASDS : public Synopsis::ASDS {

    public:

        int invocations;

        Synopsis::Status init(size_t bytes, void* memory, Synopsis::Logger *logger) override {
            this->_logger = logger;
            this->invocations = 0;
            return Synopsis::SUCCESS;
        }

        Synopsis::Status deinit() override {
            this->invocations = 0;
            return Synopsis::SUCCESS;
        }

        Synopsis::Status process_data_product(Synopsis::DpMsg msg) override {
            this->invocations++;
            return Synopsis::SUCCESS;
        }

        size_t memory_requirement(void) override {
            return 123;
        }
};


std::string get_absolute_data_path(std::string relative_path_str) {
    char sep = '/';
    const char* env_p = std::getenv("SYNOPSIS_TEST_DATA");
    EXPECT_NE(nullptr, env_p);
    std::string base_path(env_p);
    if (base_path[base_path.length()] != sep) {
        return base_path + sep + relative_path_str;
    } else {
        return base_path + relative_path_str;
    }
}


// Test high-level Synopsis Application interfaces
/*
 * Tests:
 * SYNOPSIS-IR-01
 * SYNOPSIS-IR-02
 * SYNOPSIS-IR-03
 * SYNOPSIS-IR-04
 */
TEST(SynopsisTest, TestApplicationInterface) {
    // Test initialization
    Synopsis::SqliteASDPDB db(":memory:");
    Synopsis::StdLogger logger;
    Synopsis::LinuxClock clock;
    Synopsis::MaxMarginalRelevanceDownlinkPlanner planner;

    Synopsis::Application app(&db, &planner, &logger, &clock);
    Synopsis::Status status;

    TestASDS asds;
    status = app.add_asds("test_instrument", &asds);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    Synopsis::PassthroughASDS pt_asds;
    status = app.add_asds("test_instrument", "type", &pt_asds);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    size_t mem_req;
    mem_req = app.memory_requirement();
    EXPECT_EQ(128, mem_req);

    status = app.init(0, NULL);
    EXPECT_EQ(Synopsis::Status::FAILURE, status);

    void* mem = malloc(128);
    status = app.init(128, mem);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    Synopsis::DpMsg msg(
        "test_instrument", "test_type",
        "file::///data/file.dat",
        "file::///data/meta.dat",
        true
    );

    EXPECT_EQ(0, asds.invocations);
    status = app.accept_dp(msg);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);
    EXPECT_EQ(1, asds.invocations);

    status = app.deinit();
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);
    EXPECT_EQ(0, asds.invocations);
}


// Test DpMsg interfaces
TEST(SynopsisTest, TestDpMsg) {

    Synopsis::DpMsg msg;
    EXPECT_EQ(msg.get_instrument_name(), "");
    EXPECT_EQ(msg.get_type(), "");
    EXPECT_EQ(msg.get_uri(), "");
    EXPECT_EQ(msg.get_metadata_uri(), "");
    EXPECT_EQ(msg.get_metadata_usage(), false);

    Synopsis::DpMsg msg2(
        "cntx", "flat",
        "file::///data/file.dat",
        "file::///data/meta.dat",
        true
    );
    EXPECT_EQ(msg2.get_instrument_name(), "cntx");
    EXPECT_EQ(msg2.get_type(), "flat");
    EXPECT_EQ(msg2.get_uri(), "file::///data/file.dat");
    EXPECT_EQ(msg2.get_metadata_uri(), "file::///data/meta.dat");
    EXPECT_EQ(msg2.get_metadata_usage(), true);

}


// Test ASDPDB interfaces and functionality
/*
 * Tests SYNOPSIS-IR-05
 */
TEST(SynopsisTest, TestASDPDB) {

    Synopsis::Status status;
    std::vector<int> asdp_ids;
    int asdp_id;

    Synopsis::DpDbMsg msg(
        -1, "test_instr", "test_type", "file:///data/file.dat",
        101, 0.12345, 7, Synopsis::DownlinkState::UNTRANSMITTED,
        {
            {"test_int", Synopsis::DpMetadataValue(123)},
            {"test_float", Synopsis::DpMetadataValue(123.456)},
            {"test_string", Synopsis::DpMetadataValue("test")}
        }
    );

    Synopsis::DpDbMsg msg2;

    Synopsis::SqliteASDPDB db(":memory:");
    Synopsis::StdLogger logger;

    status = db.init(0, NULL, &logger);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    status = db.insert_data_product(msg);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    // Check that a nonzero ID was assigned
    EXPECT_GT(msg.get_dp_id(), 0);

    asdp_ids = db.list_data_product_ids();
    EXPECT_EQ(asdp_ids.size(), 1);

    status = db.get_data_product(asdp_ids[0], msg2);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    // Copy ASDP ID
    asdp_id = asdp_ids[0];

    // Check all fields equal to inserted value
    EXPECT_EQ(msg.get_dp_id(), msg2.get_dp_id());
    EXPECT_EQ(msg.get_instrument_name(), msg2.get_instrument_name());
    EXPECT_EQ(msg.get_type(), msg2.get_type());
    EXPECT_EQ(msg.get_uri(), msg2.get_uri());
    EXPECT_EQ(msg.get_dp_size(), msg2.get_dp_size());
    EXPECT_EQ(msg.get_science_utility_estimate(),
        msg2.get_science_utility_estimate());
    EXPECT_EQ(msg.get_priority_bin(), msg2.get_priority_bin());
    EXPECT_EQ(msg.get_downlink_state(), msg2.get_downlink_state());

    // Check equality of all metadata retrieved
    auto meta1 = msg.get_metadata();
    auto meta2 = msg.get_metadata();
    EXPECT_EQ(3, meta1.size());
    EXPECT_EQ(meta1.size(), meta2.size());
    for (auto const& pair : meta1) {
        std::string key = pair.first;
        Synopsis::DpMetadataValue value1 = pair.second;
        Synopsis::DpMetadataValue value2 = meta2[key];
        EXPECT_EQ(value1.get_type(), value2.get_type());
        EXPECT_EQ(value1.get_int_value(), value2.get_int_value());
        EXPECT_EQ(value1.get_float_value(), value2.get_float_value());
        EXPECT_EQ(value1.get_string_value(), value2.get_string_value());
    }


    // Test Update SUE
    double new_sue = 0.5;
    status = db.update_science_utility(asdp_id, new_sue);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    status = db.get_data_product(asdp_ids[0], msg2);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    EXPECT_EQ(new_sue, msg2.get_science_utility_estimate());

    status = db.update_science_utility(-1, new_sue);
    EXPECT_EQ(Synopsis::Status::FAILURE, status);


    // Test Update priority bin
    int new_bin = 17;
    status = db.update_priority_bin(asdp_id, new_bin);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    status = db.get_data_product(asdp_ids[0], msg2);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    EXPECT_EQ(new_bin, msg2.get_priority_bin());

    status = db.update_priority_bin(-1, new_bin);
    EXPECT_EQ(Synopsis::Status::FAILURE, status);


    // Test update downlink_state
    Synopsis::DownlinkState new_state = Synopsis::DownlinkState::TRANSMITTED;
    status = db.update_downlink_state(asdp_id, new_state);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    status = db.get_data_product(asdp_ids[0], msg2);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    EXPECT_EQ(new_state, msg2.get_downlink_state());

    status = db.update_downlink_state(-1, new_state);
    EXPECT_EQ(Synopsis::Status::FAILURE, status);


    // Test no ASDP found in DB
    status = db.get_data_product(-1, msg2);
    EXPECT_EQ(Synopsis::Status::FAILURE, status);

    status = db.deinit();
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

}


// Test ASDPDB interfaces and functionality
/*
 * Tests:
 * SYNOPSIS-IR-06
 * SYNOPSIS-FR-02
 */
TEST(SynopsisTest, TestApplicationASDPDBInterfaces) {

    Synopsis::SqliteASDPDB db(":memory:");
    Synopsis::StdLogger logger;
    Synopsis::LinuxClock clock;
    Synopsis::MaxMarginalRelevanceDownlinkPlanner planner;

    Synopsis::Application app(&db, &planner, &logger, &clock);
    Synopsis::Status status;
    std::vector<int> asdp_ids;
    int asdp_id;

    // Initialize the application
    status = app.init(0, NULL);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);


    // Use low-level interface to insert ASDP entry
    Synopsis::DpDbMsg msg(
        -1, "test_instr", "test_type", "file:///data/file.dat",
        101, 0.12345, 7, Synopsis::DownlinkState::UNTRANSMITTED,
        {
            {"test_int", Synopsis::DpMetadataValue(123)},
            {"test_float", Synopsis::DpMetadataValue(123.456)},
            {"test_string", Synopsis::DpMetadataValue("test")}
        }
    );

    status = db.insert_data_product(msg);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    // Get ID of inserted ASDP
    asdp_ids = app.list_data_product_ids();
    EXPECT_EQ(asdp_ids.size(), 1);
    asdp_id = asdp_ids[0];

    // Test interfaces
    double new_sue = 0.5, new_float = 123.456;
    int new_bin = 17, new_int = 456;
    std::string new_string = "new_test";
    Synopsis::DownlinkState new_state = Synopsis::DownlinkState::TRANSMITTED;

    status = app.update_science_utility(asdp_id, new_sue);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);
    status = app.update_science_utility(-1, new_sue);
    EXPECT_EQ(Synopsis::Status::FAILURE, status);

    status = app.update_priority_bin(asdp_id, new_bin);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);
    status = app.update_priority_bin(-1, new_bin);
    EXPECT_EQ(Synopsis::Status::FAILURE, status);

    status = app.update_downlink_state(asdp_id, new_state);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);
    status = app.update_downlink_state(-1, new_state);
    EXPECT_EQ(Synopsis::Status::FAILURE, status);

    status = app.update_asdp_metadata(asdp_id, "test_int", new_int);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);
    status = app.update_asdp_metadata(-1, "test_int", new_int);
    EXPECT_EQ(Synopsis::Status::FAILURE, status);

    status = app.update_asdp_metadata(asdp_id, "test_float", new_float);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);
    status = app.update_asdp_metadata(-1, "test_float", new_float);
    EXPECT_EQ(Synopsis::Status::FAILURE, status);

    status = app.update_asdp_metadata(asdp_id, "test_string", new_string);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);
    status = app.update_asdp_metadata(-1, "test_string", new_string);
    EXPECT_EQ(Synopsis::Status::FAILURE, status);

    status = app.update_asdp_metadata(asdp_id, "bad_field", 0);
    EXPECT_EQ(Synopsis::Status::FAILURE, status);

    // Check values
    status = app.get_data_product(asdp_id, msg);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);
    EXPECT_EQ(new_sue, msg.get_science_utility_estimate());
    EXPECT_EQ(new_bin, msg.get_priority_bin());
    EXPECT_EQ(new_state, msg.get_downlink_state());

    auto meta = msg.get_metadata();
    EXPECT_EQ(Synopsis::MetadataType::INT, meta["test_int"].get_type());
    EXPECT_EQ(Synopsis::MetadataType::FLOAT, meta["test_float"].get_type());
    EXPECT_EQ(Synopsis::MetadataType::STRING, meta["test_string"].get_type());
    EXPECT_EQ(new_int, meta["test_int"].get_int_value());
    EXPECT_EQ(new_float, meta["test_float"].get_float_value());
    EXPECT_EQ(new_string, meta["test_string"].get_string_value());

    status = app.deinit();
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

}


// Test empty ASDS meta data in JSON file
TEST(SynopsisTest, TestEmptyMetaData) {

    // Construct DP message for test data
    std::string data_path = get_absolute_data_path("example_dp.dat");
    std::string metadata_path = get_absolute_data_path("example_metadata_empty_md.json");

    Synopsis::DpMsg msg(
        "test_instrument", "test_type",
        data_path,
        metadata_path,
        true  // TODO: a false here would trigger the msg.get_metadata_usage if statement to fail
    );

    // Test initialization
    Synopsis::SqliteASDPDB db(":memory:");
    Synopsis::StdLogger logger;
    Synopsis::LinuxClock clock;
    Synopsis::MaxMarginalRelevanceDownlinkPlanner planner;

    Synopsis::Application app(&db, &planner, &logger, &clock);
    Synopsis::Status status;

    Synopsis::PassthroughASDS pt_asds;
    status = app.add_asds("test_instrument", "test_type", &pt_asds);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    status = pt_asds.process_data_product(msg);
    EXPECT_EQ(Synopsis::Status::FAILURE, status);

    size_t mem_req;
    mem_req = app.memory_requirement();
    EXPECT_EQ(0, mem_req);

    status = app.init(0, NULL);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    status = app.accept_dp(msg);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    // Check DB entry
    std::vector<int> asdp_ids;
    int asdp_id;

    // Get ID of inserted ASDP
    asdp_ids = db.list_data_product_ids();
    EXPECT_EQ(asdp_ids.size(), 1);
    asdp_id = asdp_ids[0];

    // Get inserted ASDP
    Synopsis::DpDbMsg db_msg;
    status = db.get_data_product(asdp_id, db_msg);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    // Check ASDP values
    EXPECT_EQ("test_instrument", db_msg.get_instrument_name());
    EXPECT_EQ("test_type", db_msg.get_type());
    EXPECT_EQ(data_path, db_msg.get_uri());
    // EXPECT_EQ(53, db_msg.get_dp_size());
    EXPECT_EQ(0.123, db_msg.get_science_utility_estimate());
    EXPECT_EQ(7, db_msg.get_priority_bin());
    EXPECT_EQ(Synopsis::DownlinkState::UNTRANSMITTED, db_msg.get_downlink_state());

    auto meta = db_msg.get_metadata();
    EXPECT_EQ(0, meta.size());

    status = app.deinit();
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);
}


// Test bad ASDS meta data in JSON file
TEST(SynopsisTest, TestBadMetaData) {

    // Construct DP message for test data
    std::string data_path = get_absolute_data_path("example_dp.dat");
    std::string metadata_path = get_absolute_data_path("example_metadata_bad_md.json");

    Synopsis::DpMsg msg(
        "test_instrument", "test_type",
        data_path,
        metadata_path,
        true  // TODO: a false here would trigger the msg.get_metadata_usage if statement to fail
    );

    // Test initialization
    Synopsis::SqliteASDPDB db(":memory:");
    Synopsis::StdLogger logger;
    Synopsis::LinuxClock clock;
    Synopsis::MaxMarginalRelevanceDownlinkPlanner planner;

    Synopsis::Application app(&db, &planner, &logger, &clock);
    Synopsis::Status status;

    Synopsis::PassthroughASDS pt_asds;
    status = app.add_asds("test_instrument", "test_type", &pt_asds);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    status = pt_asds.process_data_product(msg);
    EXPECT_EQ(Synopsis::Status::FAILURE, status);

    size_t mem_req;
    mem_req = app.memory_requirement();
    EXPECT_EQ(0, mem_req);

    status = app.init(0, NULL);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    status = app.accept_dp(msg);
    EXPECT_EQ(Synopsis::Status::FAILURE, status);

    // Check DB entry
    std::vector<int> asdp_ids;
    int asdp_id;

    // Get ID of inserted ASDP
    asdp_ids = db.list_data_product_ids();
    EXPECT_EQ(asdp_ids.size(), 0);

    status = app.deinit();
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);
}

// Test bad ASDS SUE value in JSON file
TEST(SynopsisTest, TestBadSUE) {

    // Construct DP message for test data
    std::string data_path = get_absolute_data_path("example_dp.dat");
    std::string metadata_path = get_absolute_data_path("example_metadata_bad_sue.json");

    Synopsis::DpMsg msg(
        "test_instrument", "test_type",
        data_path,
        metadata_path,
        true  // TODO: a false here would trigger the msg.get_metadata_usage if statement to fail
    );

    // Test initialization
    Synopsis::SqliteASDPDB db(":memory:");
    Synopsis::StdLogger logger;
    Synopsis::LinuxClock clock;
    Synopsis::MaxMarginalRelevanceDownlinkPlanner planner;

    Synopsis::Application app(&db, &planner, &logger, &clock);
    Synopsis::Status status;

    Synopsis::PassthroughASDS pt_asds;
    status = app.add_asds("test_instrument", "test_type", &pt_asds);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    status = pt_asds.process_data_product(msg);
    EXPECT_EQ(Synopsis::Status::FAILURE, status);

    size_t mem_req;
    mem_req = app.memory_requirement();
    EXPECT_EQ(0, mem_req);

    status = app.init(0, NULL);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    status = app.accept_dp(msg);
    EXPECT_EQ(Synopsis::Status::FAILURE, status);

    // Check DB entry
    std::vector<int> asdp_ids;
    int asdp_id;

    // Get ID of inserted ASDP
    asdp_ids = db.list_data_product_ids();
    EXPECT_EQ(asdp_ids.size(), 0);

    status = app.deinit();
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);
}


// Test bad ASDS SUE value in JSON file
TEST(SynopsisTest, TestBadPriorityBin) {

    // Construct DP message for test data
    std::string data_path = get_absolute_data_path("example_dp.dat");
    std::string metadata_path = get_absolute_data_path("example_metadata_bad_pb.json");

    Synopsis::DpMsg msg(
        "test_instrument", "test_type",
        data_path,
        metadata_path,
        true  // TODO: a false here would trigger the msg.get_metadata_usage if statement to fail
    );

    // Test initialization
    Synopsis::SqliteASDPDB db(":memory:");
    Synopsis::StdLogger logger;
    Synopsis::LinuxClock clock;
    Synopsis::MaxMarginalRelevanceDownlinkPlanner planner;

    Synopsis::Application app(&db, &planner, &logger, &clock);
    Synopsis::Status status;

    Synopsis::PassthroughASDS pt_asds;
    status = app.add_asds("test_instrument", "test_type", &pt_asds);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    status = pt_asds.process_data_product(msg);
    EXPECT_EQ(Synopsis::Status::FAILURE, status);

    size_t mem_req;
    mem_req = app.memory_requirement();
    EXPECT_EQ(0, mem_req);

    status = app.init(0, NULL);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    status = app.accept_dp(msg);
    EXPECT_EQ(Synopsis::Status::FAILURE, status);

    // Check DB entry
    std::vector<int> asdp_ids;
    int asdp_id;

    // Get ID of inserted ASDP
    asdp_ids = db.list_data_product_ids();
    EXPECT_EQ(asdp_ids.size(), 0);

    status = app.deinit();
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);
}


// Test pass-through ASDS
TEST(SynopsisTest, TestPassThroughASDS) {

    // Construct DP message for test data
    std::string data_path = get_absolute_data_path("example_dp.dat");
    std::string metadata_path = get_absolute_data_path("example_metadata.json");

    Synopsis::DpMsg msg(
        "test_instrument", "test_type",
        data_path,
        metadata_path,
        true
    );

    // Test initialization
    Synopsis::SqliteASDPDB db(":memory:");
    Synopsis::StdLogger logger;
    Synopsis::LinuxClock clock;
    Synopsis::MaxMarginalRelevanceDownlinkPlanner planner;

    Synopsis::Application app(&db, &planner, &logger, &clock);
    Synopsis::Status status;

    Synopsis::PassthroughASDS pt_asds;
    status = app.add_asds("test_instrument", "test_type", &pt_asds);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    size_t mem_req;
    mem_req = app.memory_requirement();
    EXPECT_EQ(0, mem_req);

    status = app.init(0, NULL);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    status = app.accept_dp(msg);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    // Check DB entry
    std::vector<int> asdp_ids;
    int asdp_id;

    // Get ID of inserted ASDP
    asdp_ids = db.list_data_product_ids();
    EXPECT_EQ(asdp_ids.size(), 1);
    asdp_id = asdp_ids[0];

    // Get inserted ASDP
    Synopsis::DpDbMsg db_msg;
    status = db.get_data_product(asdp_id, db_msg);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    // Check ASDP values
    EXPECT_EQ("test_instrument", db_msg.get_instrument_name());
    EXPECT_EQ("test_type", db_msg.get_type());
    EXPECT_EQ(data_path, db_msg.get_uri());
    EXPECT_EQ(53, db_msg.get_dp_size());
    EXPECT_EQ(0.123, db_msg.get_science_utility_estimate());
    EXPECT_EQ(7, db_msg.get_priority_bin());
    EXPECT_EQ(Synopsis::DownlinkState::UNTRANSMITTED, db_msg.get_downlink_state());

    auto meta = db_msg.get_metadata();
    EXPECT_EQ(3, meta.size());
    auto int_val = meta["metadata_field_int"];
    auto float_val = meta["metadata_field_float"];
    auto string_val = meta["metadata_field_string"];
    EXPECT_EQ(Synopsis::MetadataType::INT, int_val.get_type());
    EXPECT_EQ(Synopsis::MetadataType::FLOAT, float_val.get_type());
    EXPECT_EQ(Synopsis::MetadataType::STRING, string_val.get_type());
    EXPECT_EQ(123, int_val.get_int_value());
    EXPECT_EQ(1.23, float_val.get_float_value());
    EXPECT_EQ("hello world", string_val.get_string_value());

    status = app.deinit();
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);
}


TEST(SynopsisTest, TestStdLogger) {

    Synopsis::StdLogger logger, *logger_ptr;
    logger_ptr = &logger;
    LOG(logger_ptr, Synopsis::LogType::INFO, "Test log info: %ld", 5);
    LOG(logger_ptr, Synopsis::LogType::WARN, "Test log warn: %ld", 5);
    LOG(logger_ptr, Synopsis::LogType::ERROR, "Test log error: %ld", 5);

    // TODO: try capturing stdout/stderr streams and checking if the text is as expected
}

TEST(SynopsisTest, TestLinuxClock) {

    Synopsis::LinuxClock clock;
    EXPECT_GT(clock.get_time(), 0.0);

    Synopsis::Timer timer(&clock, 0.0);
    EXPECT_FALSE(timer.is_expired());

    timer.start();
    EXPECT_TRUE(timer.is_expired());

    Synopsis::Timer timer_long(&clock, 1e9);
    timer_long.start();
    EXPECT_FALSE(timer_long.is_expired());

}







































TEST(SynopsisTest, TestExpressionsManually){
    Synopsis::RuleSet rs;

    Synopsis::StdLogger logger;
    std::map<int, Synopsis::RuleList> rule_map;
    std::map<int, Synopsis::ConstraintList> constraint_map;
    Synopsis::RuleList default_rules;
    Synopsis::ConstraintList default_constraints;

    Synopsis::RuleSet rs2(
                rule_map,
                constraint_map,
                default_rules,
                default_constraints,
                &logger);

    // Synopsis::RuleSet rs(std::map<int, Synopsis::RuleList> rule_map, std::map<int, Synopsis::ConstraintList> constraint_map, Synopsis::RuleList default_rules, Synopsis::ConstraintList default_constraints, Synopsis::Logger *logger)
    // Synopsis::Status status;
    // status = app.add_asds("test_instrument", "test_type", &pt_asds);
    // EXPECT_EQ(Synopsis::Status::SUCCESS, status);
}




TEST(SynopsisTest, TestBadRules) {
    std::string db_path = get_absolute_data_path("dd_example.db");
    std::string dd_config_path = get_absolute_data_path("dd_example_similarity_config.json");
    std::string dd_rules_path = get_absolute_data_path("example_simple_rule.json");

    // Test initialization
    Synopsis::SqliteASDPDB db(db_path);
    Synopsis::StdLogger logger;
    Synopsis::LinuxClock clock;
    Synopsis::MaxMarginalRelevanceDownlinkPlanner planner;

    Synopsis::Application app(&db, &planner, &logger, &clock);
    Synopsis::Status status;

    status = app.init(0, NULL);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    std::vector<int> prioritized_list;
    status = app.prioritize(dd_rules_path, dd_config_path, 100, prioritized_list);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);
    EXPECT_EQ(3, prioritized_list.size());
    EXPECT_EQ(1, prioritized_list[0]);
    EXPECT_EQ(3, prioritized_list[1]);

    status = app.deinit();
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

}





// TEST(SynopsisTest, TestRuleASTv2) {

   
//     Synopsis::LogicalConstant true_expr(true);
//     // Synopsis::ConstExpression adj_expr(1.0);
//     Synopsis::ConstExpression adj_expr("notnumeric");
//     int max_applications = 1;
//     Synopsis::StdLogger logger;

//     // Test two variable case
//      std::vector<std::string> variables = {"x"};  // {"x", "y"};
//     Synopsis::Rule rule(variables, &true_expr, &adj_expr, max_applications, &logger);
//     std::vector<std::map<std::string, Synopsis::DpMetadataValue>> asdps = {
//         {
//             {"asdp_id", Synopsis::DpMetadataValue(1)}
//         },
//         {
//             {"asdp_id", Synopsis::DpMetadataValue(2)}
//         },
//     };
//     EXPECT_EQ(1.0, rule.apply(asdps));

//     // // Test more than two variable case
//     // std::vector<std::string> variables3 = {"x", "y", "z"};
//     // Synopsis::Rule rule3(variables3, &true_expr, &adj_expr, max_applications, &logger);
//     // EXPECT_EQ(0.0, rule3.apply(asdps));

// }

TEST(SynopsisTest, TestRuleAST) {

    std::vector<std::string> variables = {"x"};
    Synopsis::LogicalConstant true_expr(true);
    Synopsis::LogicalConstant false_expr(false);
    Synopsis::LogicalNot not_true_expr(&true_expr);
    Synopsis::LogicalNot not_false_expr(&false_expr);
    Synopsis::ConstExpression adj_expr(1.0);
    Synopsis::ConstExpression zero_expr(0.0);
    Synopsis::ConstExpression one_expr(1.0);
    Synopsis::StringConstant a_str_expr("a");
    Synopsis::StringConstant b_str_expr("b");
    int max_applications = 1;
    Synopsis::StdLogger logger;

    Synopsis::Rule rule(variables, &true_expr, &adj_expr, max_applications, &logger);

    std::vector<std::map<std::string, Synopsis::DpMetadataValue>> asdps = {
        {
            {"asdp_id", Synopsis::DpMetadataValue(1)}
        },
        {
            {"asdp_id", Synopsis::DpMetadataValue(2)}
        },
    };

    EXPECT_TRUE(true_expr.get_value({}, {}));
    EXPECT_FALSE(false_expr.get_value({}, {}));
    EXPECT_EQ(1.0, adj_expr.get_value({}, {}).get_numeric());
    EXPECT_EQ(1.0, rule.apply(asdps));

    EXPECT_FALSE(not_true_expr.get_value({}, {}));
    EXPECT_TRUE(not_false_expr.get_value({}, {}));

    Synopsis::BinaryLogicalExpression true_and_true_expr("AND", &true_expr, &true_expr);
    Synopsis::BinaryLogicalExpression true_and_false_expr("AND", &true_expr, &false_expr);
    Synopsis::BinaryLogicalExpression false_and_false_expr("AND", &false_expr, &false_expr);
    EXPECT_TRUE(true_and_true_expr.get_value({}, {}));
    EXPECT_FALSE(true_and_false_expr.get_value({}, {}));
    EXPECT_FALSE(false_and_false_expr.get_value({}, {}));

    Synopsis::BinaryLogicalExpression true_or_true_expr("OR", &true_expr, &true_expr);
    Synopsis::BinaryLogicalExpression true_or_false_expr("OR", &true_expr, &false_expr);
    Synopsis::BinaryLogicalExpression false_or_false_expr("OR", &false_expr, &false_expr);
    EXPECT_TRUE(true_or_true_expr.get_value({}, {}));
    EXPECT_TRUE(true_or_false_expr.get_value({}, {}));
    EXPECT_FALSE(false_or_false_expr.get_value({}, {}));


    Synopsis::ComparatorExpression one_eq_one_expr("==", &one_expr, &one_expr);
    Synopsis::ComparatorExpression one_eq_zero_expr("==", &one_expr, &zero_expr);
    Synopsis::ComparatorExpression one_gt_zero_expr(">", &one_expr, &zero_expr);
    EXPECT_TRUE(one_eq_one_expr.get_value({}, {}));
    EXPECT_FALSE(one_eq_zero_expr.get_value({}, {}));
    EXPECT_TRUE(one_gt_zero_expr.get_value({}, {}));


    Synopsis::ComparatorExpression a_eq_a_expr("==", &a_str_expr, &a_str_expr);
    Synopsis::ComparatorExpression a_eq_b_expr("==", &a_str_expr, &b_str_expr);
    Synopsis::ComparatorExpression a_ne_a_expr("!=", &a_str_expr, &a_str_expr);
    Synopsis::ComparatorExpression a_ne_b_expr("!=", &a_str_expr, &b_str_expr);
    EXPECT_TRUE(a_eq_a_expr.get_value({}, {}));
    EXPECT_FALSE(a_eq_b_expr.get_value({}, {}));
    EXPECT_FALSE(a_ne_a_expr.get_value({}, {}));
    EXPECT_TRUE(a_ne_b_expr.get_value({}, {}));

    Synopsis::ComparatorExpression a_eq_one_expr("==", &a_str_expr, &one_expr);
    EXPECT_FALSE(a_eq_one_expr.get_value({}, {}));

    Synopsis::MinusExpression minus_one_expr(&one_expr);
    EXPECT_EQ(-1.0, minus_one_expr.get_value({}, {}).get_numeric());

    Synopsis::MinusExpression minus_a_expr(&a_str_expr);
    EXPECT_TRUE(std::isnan(minus_a_expr.get_value({}, {}).get_numeric()));


    Synopsis::BinaryExpression one_plus_one_expr("+", &one_expr, &one_expr);
    Synopsis::BinaryExpression one_times_one_expr("*", &one_expr, &one_expr);
    Synopsis::BinaryExpression one_minus_one_expr("-", &one_expr, &one_expr);
    EXPECT_EQ(2.0, one_plus_one_expr.get_value({}, {}).get_numeric());
    EXPECT_EQ(1.0, one_times_one_expr.get_value({}, {}).get_numeric());
    EXPECT_EQ(0.0, one_minus_one_expr.get_value({}, {}).get_numeric());

    Synopsis::BinaryExpression one_plus_a_expr("+", &one_expr, &a_str_expr);
    Synopsis::BinaryExpression one_div_one_expr("/", &one_expr, &one_expr);
    EXPECT_TRUE(std::isnan(one_plus_a_expr.get_value({}, {}).get_numeric()));
    EXPECT_TRUE(std::isnan(one_div_one_expr.get_value({}, {}).get_numeric()));

    std::map<std::string, std::map<std::string, Synopsis::DpMetadataValue>> assignments = {
        {"x", asdps[0]},
        {"y", asdps[1]}
    };

    Synopsis::Field x_id_field("x", "asdp_id");
    Synopsis::Field y_id_field("y", "asdp_id");
    Synopsis::Field z_id_field("z", "asdp_id");
    Synopsis::Field x_missing_field("x", "missing");

    EXPECT_EQ(1, x_id_field.get_value(assignments, {}).get_numeric());
    EXPECT_EQ(2, y_id_field.get_value(assignments, {}).get_numeric());

    EXPECT_TRUE(std::isnan(z_id_field.get_value(assignments, {}).get_numeric()));
    EXPECT_TRUE(std::isnan(x_missing_field.get_value(assignments, {}).get_numeric()));


    Synopsis::ComparatorExpression ex_cond_expr_zero("==", &zero_expr, &x_id_field);
    Synopsis::ComparatorExpression ex_cond_expr_one("==", &one_expr, &x_id_field);
    Synopsis::ExistentialExpression ex_expr_zero("x", &ex_cond_expr_zero);
    Synopsis::ExistentialExpression ex_expr_one("x", &ex_cond_expr_one);

    EXPECT_FALSE(ex_expr_zero.get_value({}, asdps));
    EXPECT_TRUE(ex_expr_one.get_value({}, asdps));

    Synopsis::Constraint constraint_nosum_sat(variables, &ex_cond_expr_zero, NULL, 1.0, &logger);
    Synopsis::Constraint constraint_nosum_unsat(variables, &ex_cond_expr_one, NULL, 1.0, &logger);
    Synopsis::Constraint constraint_sum_sat(variables, &true_expr, &x_id_field, 4.0, &logger);
    Synopsis::Constraint constraint_sum_unsat(variables, &true_expr, &x_id_field, 3.0, &logger);

    EXPECT_TRUE(constraint_sum_sat.apply(asdps));
    EXPECT_FALSE(constraint_sum_unsat.apply(asdps));

    // Test two variable case
     std::vector<std::string> variables2 = {"x", "y"};
    Synopsis::Rule rule2(variables2, &true_expr, &adj_expr, max_applications, &logger);
    EXPECT_EQ(1.0, rule2.apply(asdps));

    // Test more than two variable case
    std::vector<std::string> variables3 = {"x", "y", "z"};
    Synopsis::Rule rule3(variables3, &true_expr, &adj_expr, max_applications, &logger);
    EXPECT_EQ(0.0, rule3.apply(asdps));


    // std::map<int, Synopsis::RuleList> rule_map;
    // std::map<int, Synopsis::ConstraintList> constraint_map;
    // Synopsis::RuleList default_rules;
    // Synopsis::ConstraintList default_constraints;
    // Synopsis::RuleSet rs;
    // Synopsis::RuleSet rs2(
    //             rule_map,
    //             constraint_map,
    //             default_rules,
    //             default_constraints,
    //             &logger);

    // pass string to adj_expr
    // pass non numeric to value expr
    // inst empty ruleset
    // bad input to Synopsis::BinaryLogicalExpression true_and_true_expr("AND", &true_expr, &true_expr);
    // bad input to Synopsis::ComparatorExpression ex_cond_expr_zero("==", &zero_expr, &x_id_field);
    // Make a bad rules config json file to trigger rules tests 
}


TEST(SynopsisTest, TestPrioritizeInstPair) {
    std::string db_path = get_absolute_data_path("instrument_pair.db");
    std::string rules_path = get_absolute_data_path("instrument_pair_rules.json");

    // Test initialization
    Synopsis::SqliteASDPDB db(db_path);
    Synopsis::StdLogger logger;
    Synopsis::LinuxClock clock;
    Synopsis::MaxMarginalRelevanceDownlinkPlanner planner;

    Synopsis::Application app(&db, &planner, &logger, &clock);
    Synopsis::Status status;

    status = app.init(0, NULL);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    std::vector<int> prioritized_list;
    status = app.prioritize(rules_path, "", 100, prioritized_list);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);
    EXPECT_EQ(2, prioritized_list.size());
    EXPECT_EQ(3, prioritized_list[0]);
    EXPECT_EQ(4, prioritized_list[1]);

    status = app.deinit();
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

}


TEST(SynopsisTest, TestPrioritizeDD) {
    std::string db_path = get_absolute_data_path("dd_example.db");
    std::string dd_config_path = get_absolute_data_path("dd_example_similarity_config.json");
    std::string dd_rules_path = get_absolute_data_path("dd_example_rules.json");

    // Test initialization
    Synopsis::SqliteASDPDB db(db_path);
    Synopsis::StdLogger logger;
    Synopsis::LinuxClock clock;
    Synopsis::MaxMarginalRelevanceDownlinkPlanner planner;

    Synopsis::Application app(&db, &planner, &logger, &clock);
    Synopsis::Status status;

    status = app.init(0, NULL);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

    std::vector<int> prioritized_list;
    status = app.prioritize(dd_rules_path, dd_config_path, 100, prioritized_list);
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);
    EXPECT_EQ(2, prioritized_list.size());
    EXPECT_EQ(1, prioritized_list[0]);
    EXPECT_EQ(3, prioritized_list[1]);

    status = app.deinit();
    EXPECT_EQ(Synopsis::Status::SUCCESS, status);

}
