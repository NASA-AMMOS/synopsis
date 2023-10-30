// #ifndef __ITC_SYNOPSIS_BRIDGE_H__
// #define __ITC_SYNOPSIS_BRIDGE_H__

#ifdef __cplusplus
extern "C" {
#endif

//#include <gtest/gtest.h>
///#include <cmath>

#include <stdlib.h>

struct itc_dpmsg;
typedef struct itc_dpmsg itc_dpmsg_t;

struct itc_dbdpmsg;
typedef struct itc_dbdpmsg itc_dbdpmsg_t;


struct itc_dpmetavalue;
typedef struct itc_dpmetavalue itc_dpmetavalue_t;

struct itc_node;
typedef struct itc_node itc_node_t;

struct itc_dpids;
typedef struct itc_dpids itc_dpids_t;

struct itc_dpids{
    int* values;
    int size;
};

void itc_test_get_message();

int itc_setup_testasds();
void itc_setup_ptasds();

size_t itc_app_get_memory_requiremennt();
void itc_app_init(size_t bytes, void* memory);
void itc_app_deinit(void* memory);
int itc_app_get_invocations();
void itc_app_accept_dumb_dpmsg();
void itc_app_accept_dpmsg(itc_dpmsg_t* msg);

itc_dpmsg_t* itc_create_dpmsg(char* instrument_name, char* dp_type, char* dp_uri, char* meta_uri, bool meta_usage);
int itc_msg_get_dp_id(itc_dbdpmsg_t * msg);
void itc_assert_equality_dbdpmsg(itc_dbdpmsg_t* msg1, itc_dbdpmsg_t* msg2);
char* itc_msg_get_instrument_name(itc_dbdpmsg_t* msg);
char* itc_msg_get_type(itc_dbdpmsg_t* msg);

itc_dpmetavalue_t* itc_create_dpmetadatavalue_int(int int_value);
itc_dpmetavalue_t* itc_create_dpmetadatavalue_float(double float_value);
itc_dpmetavalue_t* itc_create_dpmetadatavalue_string(char* string_value);

//Not In Use
void itc_map_put(void* map, char* dp_string, itc_dpmetavalue_t* dpmetavalue);
void itc_node_push(itc_node_t* head, char* word, itc_dpmetavalue_t* metavalue);

itc_dbdpmsg_t* itc_create_dbdpmsg(int dp_id, const char* instrument_name, const char* dp_type, const char* dp_uri, size_t dp_size, double science_utility_estimate, int priority_bin, int downlink_state, itc_node_t* meta_node);

void itc_db_init(size_t bytes, void* memory);
void itc_db_insert_dumb_data_product();
void itc_db_insert_data_product(itc_dbdpmsg_t *msg);

itc_dpids_t *itc_db_list_data_product_ids();
itc_dbdpmsg_t* itc_db_get_data_product(int id);

void itc_db_destroy_data_product_ids(itc_dpids_t* dpids);
void itc_db_destroy_data_product(itc_dbdpmsg_t *msg);
void itc_destroy_dpmsg(itc_dpmsg_t *msg);
void itc_destroy_dpmetadatavalue(itc_dpmetavalue_t *metavalue);
void itc_destroy_nodes(itc_node_t *head);
void itc_destroy_dbdpbmsg(itc_dbdpmsg_t *msg);


int owls_test();//char* asdpdb_file, char* rule_config_file, char* similarity_config_file);
int owls_display_prioritized_data();
char* owls_get_prioritized_data(int index);
void owls_update_downlink_status(char* dpname);
void owls_destroy_prioritized_data_string(char* deleteme);
int owls_add_dpmsg();
int owls_prioritize_data();
void owls_set_sigma(double sigma);
void reset_dp_counter();
int get_dp_counter();

#ifdef __cplusplus
}
#endif

//#endif /* __ITC_SYNOPSIS_BRIDGE_HPP__*/