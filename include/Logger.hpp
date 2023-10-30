/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * Provides the generic interface for a system-specific logger.
 */
#ifndef JPL_SYNOPSIS_Logger
#define JPL_SYNOPSIS_Logger

#include <iostream>
#include "synopsis_types.hpp"

#define LOG_OBJECT(logger, type, fmt, ...) if(&logger != NULL){logger.log(type,  __FILE__, __LINE__, fmt, ## __VA_ARGS__);};
#define LOG(logger, type, fmt, ...) if(logger != NULL){logger->log(type,  __FILE__, __LINE__, fmt, ## __VA_ARGS__);};

namespace Synopsis {

    /**
     * Generic logging interface
     */
    class Logger {


        public:

            /**
             * Default virtual destructor
             */
            virtual ~Logger() = default;


            /**
             * Logs a message of the specified type, using a format string with
             * arguments.
             *
             * @param[in] type: log message type
             * @param[in] fmt: C-style format string
             * @param[in] ...: variable argument list for format string
             */
            // virtual void log(LogType type, const char* file, const int line, FILE *out, const char* fmt,  ...) = 0;
            virtual void log(LogType type, const char* file, const int line, const char* fmt,  ...) = 0;

    };


};


#endif
