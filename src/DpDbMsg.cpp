/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * @see DpDbMsg.hpp
 */
#include "synopsis.hpp"
#include "DpDbMsg.hpp"

namespace Synopsis {


    DpMetadataValue::DpMetadataValue() :
        type(INT),
        int_value(0),
        float_value(0.0),
        string_value("")
    {

    }

    DpMetadataValue::DpMetadataValue(int value) :
        type(INT),
        int_value(value),
        float_value(0.0),
        string_value("")
    {

    }

    DpMetadataValue::DpMetadataValue(double value) :
        type(FLOAT),
        int_value(0),
        float_value(value),
        string_value("")
    {

    }

    DpMetadataValue::DpMetadataValue(std::string value) :
        type(STRING),
        int_value(0),
        float_value(0.0),
        string_value(value)
    {

    }

    DpMetadataValue::DpMetadataValue(
        MetadataType type,
        int int_value,
        double float_value,
        std::string string_value
    ) :
        type(type),
        int_value(int_value),
        float_value(float_value),
        string_value(string_value)
    {

    }

    MetadataType DpMetadataValue::get_type(void) {
        return this->type;
    }

    int DpMetadataValue::get_int_value(void) {
        return this->int_value;
    }

    double DpMetadataValue::get_float_value(void) {
        return this->float_value;
    }

    std::string DpMetadataValue::get_string_value(void) {
        return this->string_value;
    }


    bool DpMetadataValue::is_numeric(void) {
        return (this->type == INT) || (this->type == FLOAT);
    }


    double DpMetadataValue::get_numeric(void) {
        switch (this->type) {
            case INT:
                return (double)this->int_value;
            default:
                return this->float_value;
        }
    }


    DpDbMsg::DpDbMsg() :
        dp_id(-1),
        instrument_name(""),
        dp_type(""),
        dp_uri(""),
        dp_size(0),
        science_utility_estimate(0.0),
        priority_bin(0),
        downlink_state(DownlinkState::UNTRANSMITTED),
        metadata({})
    {

    }

    DpDbMsg::DpDbMsg(
        int dp_id,
        std::string instrument_name,
        std::string dp_type,
        std::string dp_uri,
        size_t dp_size,
        double science_utility_estimate,
        int priority_bin,
        DownlinkState downlink_state,
        AsdpEntry metadata
        ) :
        dp_id(dp_id),
        instrument_name(instrument_name),
        dp_type(dp_type),
        dp_uri(dp_uri),
        dp_size(dp_size),
        science_utility_estimate(science_utility_estimate),
        priority_bin(priority_bin),
        downlink_state(downlink_state),
        metadata(metadata)
    {

    }

    int DpDbMsg::get_dp_id(void) {
        return this->dp_id;
    }

    std::string DpDbMsg::get_instrument_name(void) {
        return this->instrument_name;
    }

    std::string DpDbMsg::get_type(void) {
        return this->dp_type;
    }

    std::string DpDbMsg::get_uri(void) {
        return this->dp_uri;
    }

    size_t DpDbMsg::get_dp_size(void) {
        return this->dp_size;
    }

    double DpDbMsg::get_science_utility_estimate(void) {
        return this->science_utility_estimate;
    }

    int DpDbMsg::get_priority_bin(void) {
        return this->priority_bin;
    }

    DownlinkState DpDbMsg::get_downlink_state(void) {
        return this->downlink_state;
    }

    AsdpEntry DpDbMsg::get_metadata(void) {
        return this->metadata;
    }

    void DpDbMsg::set_dp_id(int id) {
        this->dp_id = id;
    }

    void DpDbMsg::set_instrument_name(std::string name) {
        this->instrument_name = name;
    }

    void DpDbMsg::set_type(std::string type) {
        this->dp_type = type;
    }

    void DpDbMsg::set_uri(std::string uri) {
        this->dp_uri = uri;
    }

    void DpDbMsg::set_dp_size(size_t size) {
        this->dp_size = size;
    }

    void DpDbMsg::set_science_utility_estimate(double sue) {
        this->science_utility_estimate = sue;
    }

    void DpDbMsg::set_priority_bin(int bin) {
        this->priority_bin = bin;
    }

    void DpDbMsg::set_downlink_state(DownlinkState state) {
        this->downlink_state = state;
    }

    void DpDbMsg::set_metadata(AsdpEntry metadata) {
        this->metadata = metadata;
    }


};
