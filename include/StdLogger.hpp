/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * Provides a logger implementation using standard out/standard error.
 */
#ifndef JPL_SYNOPSIS_StdLogger
#define JPL_SYNOPSIS_StdLogger

#include "Logger.hpp"


namespace Synopsis {


    class StdLogger : public Logger {


        public:

            /**
             * Default virtual destructor
             */
            virtual ~StdLogger() = default;


            /**
             * @see: Logger:log
             */
            void log(LogType type, const char* fmt, ...);


    };


};


#endif
