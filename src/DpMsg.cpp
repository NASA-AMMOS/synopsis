/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * @see DpMsg.hpp
 */
#include "synopsis.hpp"

namespace Synopsis {


    DpMsg::DpMsg() :
        instrument_name(""),
        dp_type(""),
        dp_uri(""),
        meta_uri(""),
        meta_usage(false)
    {

    }

    DpMsg::DpMsg(
        std::string instrument_name,
        std::string dp_type,
        std::string dp_uri,
        std::string meta_uri,
        bool meta_usage
        ) :
        instrument_name(instrument_name),
        dp_type(dp_type),
        dp_uri(dp_uri),
        meta_uri(meta_uri),
        meta_usage(meta_usage)
    {

    }

    std::string DpMsg::get_instrument_name(void) {
        return this->instrument_name;
    }

    std::string DpMsg::get_type(void) {
        return this->dp_type;
    }

    std::string DpMsg::get_uri(void) {
        return this->dp_uri;
    }

    std::string DpMsg::get_metadata_uri(void) {
        return this->meta_uri;
    }

    bool DpMsg::get_metadata_usage(void) {
        return this->meta_usage;
    }


};
