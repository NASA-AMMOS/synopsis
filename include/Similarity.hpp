/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * Provides similarity functionality for the maximum marginal relevance
 * downlink planner to use for diversity-aware prioritization. Includes
 * similarity function implementations, configuration, and parsing
 * capabilities.
 *
 * @see: MaxMarginalRelevanceDownlinkPlanner.hpp
 */
#ifndef JPL_SYNOPSIS_Similarity
#define JPL_SYNOPSIS_Similarity

#include <vector>
#include <map>
#include <utility>

#include "synopsis_types.hpp"
#include "DpDbMsg.hpp"
#include "Logger.hpp"


namespace Synopsis {


    /**
     * Calculates the squared Euclidean distance between two diversity
     * descriptors.
     *
     * @param[in] dd1: first diversity descriptor
     * @param[in] dd2: second diversity descriptor
     *
     * @return: squared Euclidean distance
     */
    double _sq_euclidean_dist(
        std::vector<double> dd1, std::vector<double> dd2
    );


    /**
     * Calculates the Gaussian similarity between two diversity descriptors:
     *
     *                          / || DD1 - DD2 ||^2 \
     *                          | ----------------- |
     * similarity(DD1, DD2) = e^\      sigma^2      /
     *
     * @param[in] sigma: Gaussian scale parameter
     * @param[in] dd1: first diversity descriptor
     * @param[in] dd2: second diversity descriptor
     *
     * @return: Gaussian similarity
     */
    double _gaussian_similarity(
        double sigma,
        std::vector<double> dd1, std::vector<double> dd2
    );


    /**
     * Type alias for a map of named parameters used by a similarity function
     */
    using SimParamMap = std::map<std::string, double>;


    /**
     * Implements a generic, configurable similarity function
     */
    class SimilarityFunction {


        public:

            /**
             * Constructs a similarity function from a list of diversity
             * descriptor elements, weight factors, a similarity type, and
             * parameters.
             *
             * @param[in] diversity_descriptors: a list of field names that
             * will be used to construct a diversity descriptor
             * @param[in] dd_factors: a list of weight factors that will be
             * multiplied by each raw diversity descriptor value for rescaling;
             * should be the same length as `diversity_descriptors`
             * @param[in] similarity_type: the type of similarity function to
             * compute (options: "gaussian")
             * @param[in] similarity_params: the parameters required by the
             * similarity function type:
             *    - Gaussian: {"sigma"}
             */
            SimilarityFunction(
                std::vector<std::string> diversity_descriptors,
                std::vector<double> dd_factors,
                std::string similarity_type,
                SimParamMap similarity_params, 
                Logger *logger // if log messages need to come from SimilarityFunction then pass a logger here
            );

            /**
             * Default destructor
             */
            ~SimilarityFunction() = default;

            /**
             * Extracts a diversity descriptors from an ASDP entry
             *
             * @param[in] asdp: ASDP entry from which values will be selected
             *
             * @return: vector diversity descriptor, with weights applied
             */
            std::vector<double> _extract_dd(AsdpEntry asdp);

            /**
             * Compute the similarity function between two ASDPs.
             *
             * @param[in] asdp1: first ASDP
             * @param[in] asdp2: second ASDP
             *
             * @return: similarity value between 0.0 and 1.0
             */
            double get_similarity(AsdpEntry asdp1, AsdpEntry asdp2);


        private:

            /**
             * Stores field names to be used for extracting a diversity
             * descriptor
             */
            std::vector<std::string> _diversity_descriptors;

            /**
             * Stores diversity descriptor weight factors
             */
            std::vector<double> _dd_factors;

            /**
             * Stores similarity type name
             */
            std::string _similarity_type;

            /**
             * Stores similarity parameter map
             */
            SimParamMap _similarity_params;

            /**
             * Reference to the logger instance to be used by this module
             */
            Logger *_logger = nullptr;


    };


    /**
     * Type alias to define a key for mapping instrument/type pairs to
     * functions.
     */
    using SimKey = std::pair<std::string, std::string>;

    /**
     * Type alias to define a mapping of instrument/type pairs to functions.
     */
    using SimFuncMap = std::map<SimKey, SimilarityFunction>;


    /**
     * Holds similarity configuration across priority bins
     */
    class Similarity {


        public:

            /**
             * Construct a similarity configuration.
             *
             * @param[in] alpha: a mapping of alpha values for the MMR
             * algorithm to use for each priority bin; alpha values must be
             * between 0.0 and 1.0
             * @param[in] default_alpha: the default alpha value to use for the
             * MMR algorithm for any bin not specified in `alpha`
             * @param[in] functions: a mapping of similarity functions to use
             * for priority bin
             * @param[in] default_functions: default similarity functions to
             * use for any bin not specified in `functions`
             */
            Similarity(
                std::map<int, double> alpha,
                double default_alpha,
                std::map<int, SimFuncMap> functions,
                SimFuncMap default_functions,
                Logger *logger
            );

            /**
             * Default destructor
             */
            ~Similarity() = default;

            /**
             * Utility function to retrieve a cached similarity function value.
             * If the value has not yet been computed, it is computed and
             * cached. The cached value is returned on all subsequent
             * invocations.
             *
             * @param[in] similarity_function: similarity function reference to
             * compute a similarity value
             * @param[in] asdp1: first ASDP
             * @param[in] asdp2: second ASDP
             *
             * @return: similarity function value
             */
            double _get_cached_similarity(
                SimilarityFunction &similarity_function,
                AsdpEntry asdp1,
                AsdpEntry asdp2
            );

            /**
             * Computes the maximum similarity value between a given candidate
             * ASDP and a list of ASDPs already queued for downlink, using the
             * appropriate similarity function for the specified priority bin.
             *
             * @param[in] bin: priority bin
             * @param[in] queue: list of already-queued ASDPs
             * @param[in] asdp: candidate ASDP
             *
             * @return: maximum similarity function value
             */
            double get_max_similarity(
                int bin,
                AsdpList queue,
                AsdpEntry asdp
            );

            /**
             * Computes the discount factor for given candidate ASDP with
             * respect to a  list of ASDPs already queued for downlink, using
             * the appropriate similarity function and alpha for the specified
             * priority bin.
             *
             * The base discount factor (DF) is 1.0 minus the maximum
             * similarity between the candidate ASDP and ASDPs within the
             * queue. The alpha-adjusted discount factor is:
             *
             *          DF_adj = (1.0 - alpha) + alpha * DF_base
             *
             * @see: Similarity::get_max_similarity
             *
             * @param[in] bin: priority bin
             * @param[in] queue: list of already-queued ASDPs
             * @param[in] asdp: candidate ASDP
             *
             * @return: ASDP discount factor
             */
            double get_discount_factor(
                int bin,
                AsdpList queue,
                AsdpEntry asdp
            );


        private:

            /**
             * Stores a mapping of priority bins to similarity function maps
             */
            std::map<int, SimFuncMap> _functions;

            /**
             * Stores a list of default similarity function maps
             */
            SimFuncMap _default_functions;

            /**
             * Stores a mapping of priority bins to alpha parameter values
             */
            std::map<int, double> _alpha;

            /**
             * Stores a default alpha parameter value
             */
            double _default_alpha;

            /**
             * A cache to store previously computed pairwise similarities,
             * indexed by ASDP ID pairs sorted in increasing order
             */
            std::map<std::pair<int, int>, double> _cache;

            /**
             * Reference to the logger instance to be used by this module
             */
            Logger *_logger = nullptr;


    };


    /**
     * Parse a similarity configuration file.
     *
     * @param[in] config_file: path to similarity configuration file (JSON)
     *
     * @return: parsed similarity configuration
     */
    Similarity parse_similarity_config(std::string config_file, Logger *logger);


};


#endif
