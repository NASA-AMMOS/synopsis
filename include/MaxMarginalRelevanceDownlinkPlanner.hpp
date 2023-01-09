/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * Provides a downlink planner implementation based on the maximum marginal
 * relevance algorithm [1].
 *
 * [1] Carbonell and Goldstein, "The use of MMR, diversity-based reranking for
 *     reordering documents and producing summaries,” in Proceedings of the
 *     21st Annual International ACM SIGIR Conference on Research and
 *     Development in Information Retrieval, 1998.
 *     https://doi.org/10.1145/290941.291025
 */
#ifndef JPL_SYNOPSIS_MaxMarginalRelevanceDownlinkPlanner
#define JPL_SYNOPSIS_MaxMarginalRelevanceDownlinkPlanner

#include "DownlinkPlanner.hpp"
#include "RuleAST.hpp"
#include "Similarity.hpp"


namespace Synopsis {


    /**
     * Populate a mapping of field names to values in the given ASDP from the
     * ASDPDB message.
     *
     * @param[in] msg: ASDP information in ASDPDB message format
     * @param[out] asdp: reference of ASDP mapping that will be populated
     *
     * @return: SUCCESS if the entry was successfully populated, or error code
     */
    Status _populate_asdp(DpDbMsg msg, AsdpEntry &asdp);


    /**
     * Helper function to prioritize a list of ASDPs within a specific bin
     * using the provided rules and similarity configuration.
     *
     * @param[in] bin: priority bin from which ASDPs are selected
     * @param[in] asdps: list of ASDPs represented as mappings of field names
     * to values
     * @param[in] ruleset: a set of rules/constraints to be used for
     * prioritization
     * @param[in] similarity: similarity configuration to be used for
     * prioritization
     *
     * @return: a prioritized list of ASDP identifiers
     */
    std::vector<int> _prioritize_bin(
        int bin,
        AsdpList asdps,
        RuleSet ruleset, Similarity similarity
    );


    /**
     * Maximum Marginal Relevance downlink planner implementation
     */
    class MaxMarginalRelevanceDownlinkPlanner : public DownlinkPlanner {


        public:

            /**
             * Default virtual destructor
             */
            virtual ~MaxMarginalRelevanceDownlinkPlanner() = default;

            /**
             * @see: ApplicationModule::memory_requirement
             */
            size_t memory_requirement(void);

            /**
             * @see: ApplicationModule::init
             */
            Status init(size_t bytes, void* memory, Logger *logger);

            /**
             * @see: ApplicationModule::deinit
             */
            Status deinit(void);

            /**
             * @see: DownlinkPlanner::prioritize
             */
            Status prioritize(
                std::string rule_configuration_id,
                std::string similarity_configuration_id,
                double max_processing_time_sec,
                std::vector<int> &prioritized_list
            );


    };


};


#endif
