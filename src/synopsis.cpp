/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * @see synopsis.hpp
 */
#include <cstddef>
#include <string>
#include <tuple>
#include <vector>

#include "synopsis.hpp"

namespace Synopsis {


    Application::Application(
        ASDPDB *db, DownlinkPlanner *planner, Logger *logger, Clock *clock
    ) :
        buffer_size(0),
        memory_buffer(NULL),
        _db(db),
        _planner(planner),
        _logger(logger),
        _clock(clock),
        n_asds(0)
    {

    }


    Status Application::add_asds(std::string instrument_name, ASDS *asds) {
        return this->add_asds(instrument_name, "", asds);
    }


    Status Application::add_asds(std::string instrument_name,
            std::string dp_type, ASDS *asds) {
        if (this->n_asds >= MAX_SYNOPSIS_APP_ASDS) {
            return FAILURE;
        }

        // Add ASDS to application list
        this->asds[this->n_asds] =
            std::make_tuple(instrument_name, dp_type, asds);
        this->n_asds += 1;

        // Set the DB instance to be used by the ASDS
        asds->set_database(this->_db);

        return SUCCESS;
    }


    Status Application::init(size_t bytes, void* memory) {
        Status status;
        void* pntr;
        size_t mem = 0;
        size_t offset = 0;

        // Init ASDPDB
        mem = _db->memory_requirement();
        mem += Application::padding_nbytes(mem);
        status = _db->init(
            mem, (void*)((char*)memory + offset), this->_logger
        );
        if (status != SUCCESS) {
            return status;
        }
        offset += mem;
        if (offset > bytes) {
            return FAILURE;
        }

        // Init ASDSs
        for (int i = 0; i < this->n_asds; i++) {
            ASDS *asds = std::get<2>(this->asds[i]);
            mem = asds->memory_requirement();
            mem += Application::padding_nbytes(mem);

            status = asds->init(
                mem, (void*)((char*)memory + offset), this->_logger
            );
            if (status != SUCCESS) {
                return status;
            }

            offset += mem;
            if (offset > bytes) {
                return FAILURE;
            }
        }

        // Init Planner
        mem = _planner->memory_requirement();
        mem += Application::padding_nbytes(mem);
        status = _planner->init(
            mem, (void*)((char*)memory + offset), this->_logger
        );
        if (status != SUCCESS) {
            return status;
        }
        offset += mem;
        if (offset > bytes) {
            return FAILURE;
        }
        this->_planner->set_database(this->_db);
        this->_planner->set_clock(this->_clock);

        return SUCCESS;
    }


    Status Application::deinit(void) {
        Status status;

        // De-initialize in the reverse order as initialized

        // De-init Planner
        status = _planner->deinit();
        if (status != SUCCESS) {
            return status;
        }

        // De-init ASDSs
        for (int i = 0; i < this->n_asds; i++) {
            ASDS *asds = std::get<2>(this->asds[i]);
            status = asds->deinit();
            if (status != SUCCESS) {
                return status;
            }
        }

        // De-init ASDPDB
        status = _db->deinit();
        if (status != SUCCESS) {
            return status;
        }

        return SUCCESS;
    }


    size_t Application::memory_requirement(void) {
        size_t base_memory_req = 0;
        size_t mem = 0;

        // Get ASDS memory requirement
        for (int i = 0; i < this->n_asds; i++) {
            ASDS *asds = std::get<2>(this->asds[i]);
            mem = asds->memory_requirement();
            mem += Application::padding_nbytes(mem);
            base_memory_req += mem;
        }

        // Get ASDPDB memory requirement
        mem = _db->memory_requirement();
        mem += Application::padding_nbytes(mem);
        base_memory_req += mem;

        return base_memory_req;
    }


    size_t Application::padding_nbytes(size_t block_size) {
        return (
            (MEM_ALIGN_SIZE - (block_size % MEM_ALIGN_SIZE))
            % MEM_ALIGN_SIZE
        );
    }

    Status Application::accept_dp(DpMsg msg) {

        std::string iname = msg.get_instrument_name();
        std::string dp_type = msg.get_type();

        Status status = SUCCESS;
        Status status_i;
        for (int i = 0; i < this->n_asds; i++) {

            std::string iname_i = std::get<0>(this->asds[i]);
            if (iname == iname_i) {

                std::string dp_type_i = std::get<1>(this->asds[i]);
                if ((dp_type_i == "") || (dp_type_i == dp_type)) {

                    ASDS *asds = std::get<2>(this->asds[i]);
                    status_i = asds->process_data_product(msg);
                    if (status_i != SUCCESS) {
                        status = status_i;
                        LOG(this->_logger, Synopsis::LogType::ERROR, "ASDS processing failed with status: %ld", status);
                    }

                }
            }
        }
        return status;
    }


    Status Application::update_science_utility(int asdp_id, double sue) {
        return _db->update_science_utility(asdp_id, sue);
    }


    Status Application::update_priority_bin(int asdp_id, int bin) {
        return _db->update_priority_bin(asdp_id, bin);
    }


    Status Application::update_downlink_state(
        int asdp_id, DownlinkState state
    ) {
        return _db->update_downlink_state(asdp_id, state);
    }


    template <>
    Status Application::update_asdp_metadata(
        int asdp_id, std::string fieldname, int value
    ) {
        return _db->update_metadata(asdp_id, fieldname, value);
    }


    template <>
    Status Application::update_asdp_metadata(
        int asdp_id, std::string fieldname, double value
    ) {
        return _db->update_metadata(asdp_id, fieldname, value);
    }


    template <>
    Status Application::update_asdp_metadata(
        int asdp_id, std::string fieldname, std::string value
    ) {
        return _db->update_metadata(asdp_id, fieldname, value);
    }

    std::vector<int> Application::list_data_product_ids(void) {
        return _db->list_data_product_ids();
    }

    Status Application::get_data_product(int asdp_id, DpDbMsg& msg) {
        return _db->get_data_product(asdp_id, msg);
    }

    Status Application::prioritize(
        std::string rule_configuration_id,
        std::string similarity_configuration_id,
        double max_processing_time_sec,
        std::vector<int> &prioritized_list
    ) {
        return _planner->prioritize(
            rule_configuration_id,
            similarity_configuration_id,
            max_processing_time_sec,
            prioritized_list
        );
    }


};
