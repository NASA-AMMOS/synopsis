/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * Provides a definition of custom and common types used by the SYNOPSIS
 * library.
 */
#ifndef JPL_SYNOPSIS_TYPES
#define JPL_SYNOPSIS_TYPES

#include <cstddef>


namespace Synopsis {


    /**
     * Custom return status codes
     */
    typedef enum {
        SUCCESS = 0,
        FAILURE = 1,
        TIMEOUT = 2
    } Status;


    /**
     * ASDP Downlink States
     */
    typedef enum {
        UNTRANSMITTED = 0,
        TRANSMITTED = 1,
        DOWNLINKED = 2
    } DownlinkState;


    /**
     * ASDP Metadata Field Types
     */
    typedef enum {
        INT = 0,
        FLOAT = 1,
        STRING = 2
    } MetadataType;


    /**
     * SYNOPSIS Log Message Types
     */
    typedef enum {
        INFO = 0,
        WARN = 1,
        ERROR = 2
    } LogType;


};

#endif
