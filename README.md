SYNOPSIS: Science Yield improvemeNt via Onboard Prioritization and Summary of Information System
================================================================================================

SYNOPSIS is a system to prioritize the downlink of data products, including
both raw data products as well as "autonomous science data products" that are
extracted onboard the spacecraft to summarize data.

## Team
 - Mark Wronkiewicz
 - Gary Doran
 - Jack Lightholder
 - Zaki Hasnain
 - Rex Stockham
 - Robert Brown (JSTAR team at NASA IV&V)
 - Scott Zemerick (JSTAR team at NASA IV&V)
 - Lukas Mandrake

## Building SYNOPSIS

1. Run `cmake -S . -B build` to set up the build directory
2. Run `cmake --build build` to build the code
3. Run `cd build && ctest` to execute the tests

## SYNOPSIS Integration into cFS
See the [core Flight Software (cFS) README file](cfs_integration/README.md) for instructions on building SYNOPSIS in support of a cFS app.

### YouTube: Integration demo with [NOS3](http://www.stf1.com/NOS3Website/Nos3MainTab.php)
Click the below image to see a demo simulating the use of SYNOPSIS (integrated into cFS) for data prioritization. Commands are sent to exercise SYNOPSIS's functionality and a simple dashboard tracks and visualizes data from the [OWLS-Autonomy project](https://iopscience.iop.org/article/10.3847/PSJ/ad0227) as it moves through the onboard processing steps.
[![YouTube](http://i.ytimg.com/vi/41cvM5iSn2Y/hqdefault.jpg)](https://www.youtube.com/watch?v=41cvM5iSn2Y)

## Testing

To test from scratch, run the following commands:

1. `ssh itlinux-b` (or equivalent machine)
2. `git clone git@github.jpl.nasa.gov:SYNOPSIS/synopsis.git`
3. `cd synopsis`
4. Proceed with the Setup steps above

## For development only: code testing & coverage stats

Note this requires `lcov`

1. `cmake --build build_cov`
2. `cd build_cov`
3. `ctest`
4. `lcov --capture --directory CMakeFiles/synopsis.dir/src --exclude "*sqlite3*" --output-file coverage.info`
5. `genhtml coverage.info --output-directory out`
6. View stats HTML `build_cov/out/index.html`

## Compiling rule files
The rules files are defined in a human readable form via the `.srd` files. To prepare these rules for use within SYNOPSIS, they must be converted to a .json representation. To do that, use the `srd_to_json.py` script as follows:
```
python synopsis/python/synopsis/srd_to_json.py \
/path/to/my/synposis_rules.srd \
/path/to/my/synposis_rules.json
```

## Architecture Overview

SYNOPSIS is designed to be a modular framework for downlink prioritization to
be incorporated into flight software frameworks such as
[F'](https://nasa.github.io/fprime/) or [cFS](https://cfs.gsfc.nasa.gov/).
Users of the SYNOPSIS library instantiate an `Application` (see `synopsis.hpp`)
along with a number of modules. Modules include:

 - Autonomous Science Data Systems (see `ASDS.hpp`)
 - An Autonomous Science Data Product Database (see `ASDPDB.hpp`)
 - A Downlink Planner (see `DownlinkPlanner.hpp`)
 - A Logger (see `Logger.hpp`)
 - A Clock (see `Clock.hpp`)

The ASDSs are registered with an application instance after construction (see
`Application::add_asds`), whereas the other modules are used during application
construction. After registering ASDSs, the application instance should be
initialized using `Application::init`. A helper function
`Application::memory_requirement` specifies how much pre-allocated memory
should be passed to the application for initialization.

After initialization, new data products can be ingested into SYNOPSIS using the
`Application::accept_dp` function. Data product messages passed to this
function contain information about the data product's origin (i.e., which
instrument produced it), type, location on a filesystem, and metadata location.
Invoking this function will either synchronously or asynchronously (depending
on the design of the ASDS) process the data product to produce an autonomous
science data product (ASDP), which will then be inserted into the ASDP
database (ASDPDB).

SYNOPSIS provides an interface to manually update information in the ASDP DB,
which can be invoked via ground commands. These functions are:

 - `Application::update_science_utility`
 - `Application::update_priority_bin`
 - `Application::update_downlink_state`
 - `Application::update_asdp_metadata`

At any time, the user of SYNOPSIS can also invoke the `Application::prioritize`
function, which will return a prioritized list of ASDPs using a set of
prioritization rules and constraints, along with similarity configuration for
diversity-aware prioritization. This functionality should be invoked prior to
a downlink pass, and the information associated with unique ASDP identifier can
be queried from the ASDP DB to enable transmission to the ground. After
transmission, the downlink state of each transmitted ASDP should be updated so
that subsequent invocations of the `Application::prioritize` function exclude
these data products.

Finally, when SYNOPSIS functionality is no longer required, the
`Application::deinit` function can be used to relinquish the use of the memory
provided during application initialization. Initialization and
de-initialization can be performed as often as necessary (i.e., immediately
prior to and after the invocation of one of the functions above that requires
SYNOPSIS to be initialized). One exception is for asynchronous ASDSs; SYNOPSIS
must be initialized when an ASDS returns its result to be submitted to the ASDP
DB.

## Acknowledgements

Funding and support for this work was provided by the Multi-mission Ground
Systems and Services (MGSS) program office.

Copyright 2023, by the California Institute of Technology. ALL RIGHTS RESERVED. United States Government Sponsorship acknowledged. Any commercial use must be negotiated with the Office of Technology Transfer at the California Institute of Technology.

This software may be subject to U.S. export control laws. By accepting this software, the user agrees to comply with all applicable U.S. export laws and regulations. User has the responsibility to obtain export licenses, or other export authority as may be required before exporting such information to foreign countries or providing access to foreign persons.
