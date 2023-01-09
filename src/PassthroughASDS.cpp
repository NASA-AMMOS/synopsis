/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * @see PassthroughASDS.hpp
 */
#include "PassthroughASDS.hpp"


namespace Synopsis {


    Status PassthroughASDS::init(size_t bytes, void* memory, Logger *logger) {
        this->_logger = logger;
        return SUCCESS;
    }

    Status PassthroughASDS::deinit() {
        return SUCCESS;
    }

    size_t PassthroughASDS::memory_requirement(void) {
        return 0;
    }

    Status PassthroughASDS::process_data_product(DpMsg msg) {
        return this->submit_data_product(msg);
    }


};
