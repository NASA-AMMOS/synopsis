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


    StdLogger::StdLogger(
        bool output_all_to_stderr
    ) :
        output_all_to_stderr(output_all_to_stderr)
    {

    }

    void StdLogger::log(LogType type, const char* file, const int line, const char* fmt,  ...) {
        va_list args;
        va_start(args, fmt);

        FILE *out;
        std::string prefix;

        switch (type) {

            case LogType::INFO:
                out = stdout;
                prefix = "[INFO]";
                break;

            case LogType::WARN:
                out = stdout;
                prefix = "[WARN]";
                break;

            case LogType::ERROR:
            default:
                out = stderr;
                prefix = "[ERROR]";
                break;

        }
        if (this->output_all_to_stderr) {
            out = stderr;
        }

        // fprintf(out, "%s", prefix.c_str());
        // fprintf(out, " ");
        // fprintf(out, "%s", file);
        // fprintf(out, ", line ");
        // fprintf(out, "%d", line);
        fprintf(out, "%s %s, line %d: ", prefix.c_str(), file, line);
        fprintf(out, ": ");
        vfprintf(out, fmt, args);
        fprintf(out, "\n");

        va_end(args);
    }


};
