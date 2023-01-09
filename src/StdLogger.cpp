/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * @see StdLogger.hpp
 */
#include <iostream>
#include <cstdarg>

#include "StdLogger.hpp"


namespace Synopsis {


    void StdLogger::log(LogType type, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);

        FILE *out;
        std::string prefix;

        switch (type) {

            case LogType::INFO:
                out = stdout;
                prefix = "[INFO ]";
                break;

            case LogType::WARN:
                out = stdout;
                prefix = "[WARN ]";
                break;

            case LogType::ERROR:
            default:
                prefix = "[ERROR]";
                out = stderr;
                break;

        }

        fprintf(out, "%s", prefix.c_str());
        vfprintf(out, fmt, args);
        fprintf(out, "\n");

        va_end(args);
    }


};
