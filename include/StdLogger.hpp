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
             * Constructs an StdLogger instance
             *
             * @param[in] output_all_to_stderr: output logs to stderr stream
             */
            StdLogger(bool output_all_to_stderr = false);

            /**
             * Default virtual destructor
             */
            virtual ~StdLogger() = default;

            /**
             * @see: Logger:log
             */
            void log(LogType type, const char* file, const int line, const char* fmt,  ...);

        private:

            /**
             * output all logs to stderr stream
             */
            bool output_all_to_stderr;

    };

};


#endif
