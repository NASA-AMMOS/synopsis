#include "itc_synopsis_bridge.h"

#include "synopsis.hpp"
#include "SqliteASDPDB.hpp"
#include "StdLogger.hpp"
#include "LinuxClock.hpp"
#include "MaxMarginalRelevanceDownlinkPlanner.hpp"
#include "Timer.hpp"
#include "RuleAST.hpp"
#include "StdLogger.hpp"
#include <cstring>
#include <vector>
#include <algorithm>

#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

 typedef enum {
    E_SUCCESS = 0,
    E_FAILURE = 1,
    E_TIMEOUT = 2
 }ITC_STATUS_MESSAGE;

//Owls Test


// This needs to be modified in order utilize a database, rather than build as we have been.
// Perhaps it will be swapped back, but it is trivial to do so.
//Synopsis::SqliteASDPDB db(":memory:");
Synopsis::SqliteASDPDB db("/home/nos3/Desktop/github-nos3/fsw/build/exe/cpu1/data/owls/owls_asdpdb_20230815_copy.db");

Synopsis::StdLogger logger;
Synopsis::LinuxClock clock2;
Synopsis::MaxMarginalRelevanceDownlinkPlanner planner;

Synopsis::Application app(&db, &planner, &logger, &clock2);
Synopsis::PassthroughASDS pt_asds;

std::vector<std::string> prioritized_uris;
std::vector<int> prioritized_list;

int dp_counter = 0;

/**
 * ITC Function to reset the value of the DP MSG Counter to 0
*/
void reset_dp_counter(){
    dp_counter = 0;
    //printf("  reset_dp_counter: dp_counter set to %d \n", reset_dp_counter);
}

/**
 * ITC Function to reset the value of the DP MSG Counter to 0
*/
void set_dp_counter(int value){
    dp_counter = value;
    //printf("  reset_dp_counter: dp_counter set to %d \n", reset_dp_counter);
}

/**
 * ITC Function to get the value of the DP MSG Counter
*/
int get_dp_counter(){
    //printf("  get_dp_counter: dp_counter is %d \n", dp_counter);
    return dp_counter;
}


std::string get_absolute_data_path(std::string relative_path_str) {
    char sep = '/';
    const char* env_p = std::getenv("SYNOPSIS_TEST_DATA");
    //EXPECT_NE(nullptr, env_p);
    std::string base_path(env_p);
    if (base_path[base_path.length()] != sep) {
        return base_path + sep + relative_path_str;
    } else {
        return base_path + relative_path_str;
    }
}

/**
 * ITC Function for creation of dpmsg based on Owls pre-canned data
 * Note:  This requires some asset directory setup and the existence of data or some erroneous behavior will occur.
 * @return: Synopsis::Status resulting from the app.accept_dp wrapped within
*/
int owls_add_dpmsg(){ 
    Synopsis::Status status = Synopsis::Status::FAILURE;

    if (dp_counter > 7){
        printf("*! Unable to add additional Data!\n");
        status = Synopsis::Status::FAILURE;
    }
    else{
        printf("** SYN_APP: Adding Data Product %d\n", dp_counter);
        std::string data_path("/home/nos3/Desktop/github-nos3/fsw/build/exe/cpu1/data/owls/bundle/asdp00000000" + std::to_string(dp_counter) + ".tgz");
        std::string metadata_path("/home/nos3/Desktop/github-nos3/fsw/build/exe/cpu1/data/owls/bundle/asdp00000000" + std::to_string(dp_counter) + "_meta.json");
        

        // printf("STRINGS: \n\t%s\n\t%s\n", data_path.c_str(), metadata_path.c_str());
        // std::string owls_string = "owls";
        // std::string helm_string = "helm";
        
        Synopsis::DpMsg msg("owls", "helm", data_path, metadata_path, true);

        status = app.accept_dp(msg);
    }    
    
    if (status != Synopsis::Status::SUCCESS){
        printf("*! Error Adding DPMSG to DB!\n");
    }
    else{
        printf("** Adding DPMSG to DB!\n");
        dp_counter++;
    }

    return status;
}

/**
 * ITC Function to update the Sigma Value
 * Modifies the SImilarity Config
*/
void owls_set_sigma(double sigma)
{
    //printf("* INCOMING ALPHA: %f\n", sigma);
    std::string similarity("/home/nos3/Desktop/github-nos3/fsw/build/exe/cpu1/data/owls/owls_similarity_config.json");
    std::ifstream json_in(similarity);
    json j_sigma;
    json_in >> j_sigma;
    j_sigma["alphas"]["default"] = sigma;

    std::ofstream json_out(similarity);
    json_out << std::setw(4) << j_sigma << std::endl; 
}

/**
 * ITC Function for prioritization of pre-canned OWLs data
 * Note:  This requires some setup and the existence of files or some erroneous behavior will occur.
 * @return: Synopsis::Status resulting from the app.prioritize wrapped within
*/
int owls_prioritize_data(){
    
    // printf("LIST SIZE: %d\n", prioritized_list.size());
    // if(prioritized_list.size() != 0) {prioritized_list.clear();}
    prioritized_list.clear();
    //printf("LIST SIZE: %d\n", prioritized_list.size());
    //std::string asdpdb("/home/nos3/Desktop/github-nos3/fsw/build/exe/cpu1/data/owls/owls_bundle_20221223T144226.db");
    std::string rule_file("/home/nos3/Desktop/github-nos3/fsw/build/exe/cpu1/data/owls/empty_rules.json");
    std::string similarity("/home/nos3/Desktop/github-nos3/fsw/build/exe/cpu1/data/owls/owls_similarity_config.json");
    
    Synopsis::Status status;
    
    
    status = app.prioritize(rule_file, similarity, 1e9, prioritized_list);
    //printf("LIST SIZE: %d\n", prioritized_list.size());
    if (status != Synopsis::Status::SUCCESS) { return status; }

    // printf("URI SIZE: %d\n", prioritized_uris.size());
    // if(prioritized_uris.size() != 0) {prioritized_uris.clear();}
    prioritized_uris.clear();
    
    printf("URI SIZE: %d\n", prioritized_uris.size());
    
    for (auto i : prioritized_list) {
        Synopsis::DpDbMsg temp_msg;
        //printf("I: %d\n", i);
        app.get_data_product(i, temp_msg);
        prioritized_uris.push_back(temp_msg.get_uri());
        //printf("NAME: %s\n", temp_msg.get_uri().c_str());
    }
    for (auto uri : prioritized_uris){
        printf("** SYNOPSIS URI: %s\n", uri.c_str());
    }

    return Synopsis::Status::SUCCESS;
}

/**
 * ITC Function displaying current prioritized data
 * Note:  This requires some setup and the existence of data or some erroneous behavior will occur.
 * @return: Synopsis::Status::SUCCESS
*/
int owls_display_prioritized_data(){
    for (auto uri: prioritized_uris){
        printf("URI: %s\n", uri.c_str());
    }
    return Synopsis::Status::SUCCESS;
}

/**
 * ITC Function for retreiving the asdp_id from global lists
 * @param: char* uri: The URI of the DpDbMsg from which to retreive the asdp_id
 * @return: int: asdp_id
*/
int get_aspd_id(char* uri){
    int returnval = -1;
    
    std::string orig(uri);
    //printf("ORIG: %s\n", orig.c_str());
    for (auto i : prioritized_list){
        Synopsis::DpDbMsg temp_msg;
        app.get_data_product(i, temp_msg);
        //printf("INT I: %d\n", i);
        std::string comp(temp_msg.get_uri().c_str());
        //printf("COMP: %s\n", comp.c_str());
        int match = orig.compare(temp_msg.get_uri());
        //printf("MATCH: %d\n", match);
        if( match == 0){
            returnval = temp_msg.get_dp_id();
            break;
        }
    }
    return returnval;
}

/**
 * ITC Function for the simulation of dowlinking prioritized files
 * Note:  This function retreives the highest priority file, and uses cFS OS calls
 * to OS_mv the data to a simulated downlink location.  It then updates the downlink state
 * within the DB so that it is no longer part of the prioritization list.
 * @return: char*: The URI of the prioritized data to downlink
*/
char* owls_get_prioritized_data(int index){
    size_t uri_size = prioritized_uris.size();
    //printf("URI SIZE: %d\n", uri_size);
    if((uri_size != 0) && (0 <= index <= 7) && (index < uri_size) ){
        //printf("INSIDE\n");
        size_t len = strlen(prioritized_uris[index].c_str())+1;
        char* returnval = new char[len];
        strcpy(returnval, (char*)prioritized_uris[index].c_str());
        
        return returnval;
        }
    else{
        //printf("RETURNING NULL\n");
        return NULL;
    }
}

/**
 * ITC Function to update dataproduct status on successful downlink
 * Modifies the DLState to DOWNLINKED
*/
void owls_update_downlink_status(char* dpname){
    auto itr = std::find(prioritized_uris.begin(), prioritized_uris.end(), dpname);
    if(itr != prioritized_uris.end()){
        int aspd_id = get_aspd_id(dpname);
        Synopsis::Status status = db.update_downlink_state(aspd_id, Synopsis::DownlinkState::DOWNLINKED);
        //prioritized_uris.erase(itr);
        printf("** SYNOPSIS DP UPDATED\n");
    }   
    else{
        printf("*! SYNOPSIS DP NOT FOUND!\n");
    }
}        

/**
 * ITC Function for the destruction of data string
 * Deletes the char array created by owls_get_prioritized_data
*/
void owls_destroy_prioritized_data_string(char* deleteme){
    delete[] deleteme;
}

/**
 * ITC Function for setup of the PassThrough ASDS
 * @return: VOID
*/
void itc_setup_ptasds(){
    Synopsis::Status status;
    status = app.add_asds("owls", "helm", &pt_asds);
    ITC_STATUS_MESSAGE result = (ITC_STATUS_MESSAGE)status;
    if(result == E_SUCCESS){
        printf("** SYNOPSIS PTASDS SETUP SUCCESSFUL!\n");
    }
    else{
        printf("*! SYNOPSIS PTASDS SETUP UNSUCCESSFUL!\n");
    }
}

/**
 * ITC Function for setup of the TEST ASDS Class
 * Note:  This requires some setup or some erroneous behavior will occur.
 * @return: size_t memory requirement size required by app
*/
size_t itc_app_get_memory_requiremennt(){
    size_t mem_req_bytes = 0;
    mem_req_bytes = app.memory_requirement();        
    printf("** SYNOPSIS REQ Bytes: %d\n", mem_req_bytes);
    return mem_req_bytes;
}

/**
 * ITC Function for Initializing the App.  
 * Makes use of a memory object that will need to be freed.
 * @return: VOID
*/
void itc_app_init(size_t bytes, void* memory){ 
    Synopsis::Status status;
    status = app.init(bytes, memory);
    ITC_STATUS_MESSAGE result = (ITC_STATUS_MESSAGE)status;
    if(result == E_SUCCESS){
        printf("** SYNOPSIS APP INIT SUCCESSFUL!\n");
    }
    else{
        printf("*! SYNOPSIS APP INIT UNSUCCESSFUL!\n");
    }
}

/**
 * ITC Function for De-Initialization of the App
 * @return: VOID
*/
void itc_app_deinit(void* memory){
    Synopsis::Status status;
    status = app.deinit();

    // sqlite3_clear(db);
    // See if sqlite3 has a parameter for number of entries 
    // Use that to delete all
    // Then confirm deleted via same number of entries
 
    // db.deinit();
    // pt_asds.deinit();
    ITC_STATUS_MESSAGE result = (ITC_STATUS_MESSAGE)status;
    if(result == E_SUCCESS){
        printf("** SYNOPSIS DEINIT SUCCESSFUL!\n");
    }
    else{
        printf("*! SYNOPSIS DEINIT UNSUCCESSFUL!\n");
    }
}