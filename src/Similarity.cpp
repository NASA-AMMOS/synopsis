/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * @see: Similarity.hpp
 */
#include <cmath>
#include <nlohmann/json.hpp>
#include <fstream>

#include "Similarity.hpp"


namespace Synopsis {


    /**
     * Utility to parse a list of functions from a JSON object.
     *
     * @param[in] flist: JSON object containing function list
     *
     * @return: mapping from similarity function keys to functions
     */
    SimFuncMap _parse_function_list(nlohmann::json flist);


    double _sq_euclidean_dist(
        std::vector<double> dd1, std::vector<double> dd2
    ) {
        double acc = 0.0;
        int n1 = dd1.size();
        int n2 = dd2.size();
        int n = std::min(n1, n2);

        for (int i = 0; i < n; i++) {
            double diff = dd1[i] - dd2[i];
            acc += (diff * diff);
        }

        return acc;
    }


    double _gaussian_similarity(
        double sigma,
        std::vector<double> dd1, std::vector<double> dd2
    ) {
        double dist_sq = _sq_euclidean_dist(dd1, dd2);
        return exp(-(dist_sq / (sigma * sigma)));
    }


    SimilarityFunction::SimilarityFunction(
        std::vector<std::string> diversity_descriptors,
        std::vector<double> dd_factors,
        std::string similarity_type,
        SimParamMap similarity_params
    ) :
        _diversity_descriptors(diversity_descriptors),
        _dd_factors(dd_factors),
        _similarity_type(similarity_type),
        _similarity_params(similarity_params)
    {

    }

    std::vector<double> SimilarityFunction::_extract_dd(
        AsdpEntry asdp
    ) {
        std::vector<double> dd;
        int n_dd = this->_diversity_descriptors.size();

        for (int i = 0; i < n_dd; i++) {
            std::string key = this->_diversity_descriptors[i];

            // TODO: handle missing key
            double dd_i = asdp[key].get_float_value();

            // Multiply by factor (if provided)
            if (i < this->_dd_factors.size()) {
                dd_i *= this->_dd_factors[i];
            }

            dd.push_back(dd_i);

        }

        return dd;
    }


    double SimilarityFunction::get_similarity(
        AsdpEntry asdp1,
        AsdpEntry asdp2
    ) {

        double similarity = 0.0;
        auto dd1 = this->_extract_dd(asdp1);
        auto dd2 = this->_extract_dd(asdp2);

        if (this->_similarity_type == "gaussian") {
            double sigma = 1.0;
            if (this->_similarity_params.count("sigma")) {
                sigma = this->_similarity_params["sigma"];
            } else {
                // TODO: Log warning about missing parameter
            }
            similarity = _gaussian_similarity(sigma, dd1, dd2);
        } else {
            // TODO: Log unknown similarity type
        }

        // TODO: Implement
        return similarity;
    }


    Similarity::Similarity(
        std::map<int, double> alpha,
        double default_alpha,
        std::map<int, SimFuncMap> functions,
        std::map<SimKey, SimilarityFunction
        > default_functions
    ) :
        _alpha(alpha),
        _default_alpha(default_alpha),
        _functions(functions),
        _default_functions(default_functions)
    {

    }


    double Similarity::_get_cached_similarity(
        SimilarityFunction &similarity_function,
        AsdpEntry asdp1,
        AsdpEntry asdp2
    ) {

        // Make cache key
        std::pair<int, int> cache_key;
        int aid1 = asdp1["id"].get_int_value();
        int aid2 = asdp2["id"].get_int_value();

        // Smaller ASDP ID is first element
        if (aid1 < aid2) {
            cache_key = std::make_pair(aid1, aid2);
        } else {
            cache_key = std::make_pair(aid2, aid1);
        }

        // Compute similarity if not in cache
        double similarity;
        if (!this->_cache.count(cache_key)) {
            similarity = similarity_function.get_similarity(asdp1, asdp2);
            this->_cache[cache_key] = similarity;
        } else {
            similarity = this->_cache[cache_key];
        }

        return similarity;
    }


    double Similarity::get_max_similarity(
        int bin,
        AsdpList queue,
        AsdpEntry asdp
    ) {

        if (queue.size() == 0) {
            // Special case: queue is empty
            return 0.0;
        }

        auto inst_type = std::make_pair(
            asdp["instrument_name"].get_string_value(),
            asdp["type"].get_string_value()
        );

        SimFuncMap sf;
        if (this->_functions.count(bin)) {
            sf = this->_functions[bin];
        } else {
            sf = this->_default_functions;
        }

        // Get similarity function
        if (!sf.count(inst_type)) {
            // No similarity function specified for this ASDP
            return 0.0;
        }
        SimilarityFunction &sim = sf.at(inst_type);

        // Compute max similarity
        int aid1 = asdp["id"].get_int_value();
        double max_similarity = 0.0;
        for (auto &asdp2 : queue) {

            // Check for type match
            auto inst_type2 = std::make_pair(
                asdp2["instrument_name"].get_string_value(),
                asdp2["type"].get_string_value()
            );
            if (inst_type != inst_type2) {
                continue;
            }

            double similarity = this->_get_cached_similarity(
                sim, asdp, asdp2
            );

            if (similarity > max_similarity) {
                max_similarity = similarity;
            }

        }

        return max_similarity;
    }


    double Similarity::get_discount_factor(
        int bin,
        AsdpList queue,
        AsdpEntry asdp
    ) {
        double max_similarity = this->get_max_similarity(bin, queue, asdp);
        double alpha = this->_default_alpha;
        if (this->_alpha.count(bin)) {
            // If alpha specified, overwrite default 1.0 value
            alpha = this->_alpha[bin];
        }
        return (1.0 - alpha) + (alpha * (1.0 - max_similarity));
    }


    SimFuncMap _parse_function_list(nlohmann::json flist) {

        SimFuncMap functions;
        SimKey key;

        for (auto& f : flist) {
            if (!f.is_object()) {
                // TODO: log bad function type
                continue;
            }

            // Parse Key
            // TODO: handle key missing
            auto j_key = f["key"];
            if (j_key.is_array() && j_key.size() == 2) {
                // TODO: verify string values
                key = std::make_pair(
                    j_key[0].get<std::string>(),
                    j_key[1].get<std::string>()
                );
            } else {
                // TODO: log bad function key type/size
                continue;
            }

            // Parse Function
            // TODO: handle keys missing
            auto j_func = f["function"];
            if (!j_func.is_object()) {
                // TODO: log bad j_func type
                continue;
            }

            // TODO: handle keys missing
            auto j_dd = j_func["diversity_descriptor"];
            auto j_weights = j_func["weights"];
            auto j_sim_type = j_func["similarity_type"];
            auto j_sim_param = j_func["similarity_parameters"];

            // Check types
            if (!j_dd.is_array()) {
                // TODO: log bad type
                continue;
            }
            if (!j_weights.is_array()) {
                // TODO: log bad type
                continue;
            }
            if (!j_sim_type.is_string()) {
                // TODO: log bad type
                continue;
            }
            if (!j_sim_param.is_object()) {
                // TODO: log bad type
                continue;
            }

            if (j_dd.size() != j_weights.size()) {
                // TODO: log mismatch
                continue;
            }

            std::vector<std::string> dds;
            std::vector<double> dd_factors;
            std::string similarity_type;
            SimParamMap similarity_params;

            // Parse DDs and weights
            for (int i = 0; i < j_dd.size(); i++) {
                auto j_dd_i = j_dd[i];
                if (!j_dd_i.is_string()) {
                    // TODO: warn
                    continue;
                }
                auto j_weight_i = j_weights[i];
                if (!j_weight_i.is_number()) {
                    // TODO: warn
                    continue;
                }
                dds.push_back(j_dd_i.get<std::string>());
                dd_factors.push_back(j_weight_i.get<double>());
            }

            similarity_type = j_sim_type.get<std::string>();

            for (auto& j_p_el : j_sim_param.items()) {
                std::string p_key = j_p_el.key();
                auto val = j_p_el.value();
                if (!val.is_number()) {
                    // TODO: log error
                    continue;
                }
                similarity_params[p_key] = val.get<double>();
            }

            SimilarityFunction func = SimilarityFunction(
                dds, dd_factors, similarity_type, similarity_params
            );
            functions.insert(std::make_pair(key, func));

        }

        return functions;
    }


    Similarity parse_similarity_config(std::string config_file) {

        // If no config file provided, return default configuration
        if (config_file.size() == 0) {
            return Similarity({}, 1.0, {}, {});
        }

        std::ifstream file_input(config_file);
        auto j = nlohmann::json::parse(file_input);

        // Parse alpha parameters

        std::map<int, double> alpha;
        double default_alpha = 1.0;

        // TODO: handle no "alphas" key
        auto j_alphas = j["alphas"];
        if (j_alphas.is_object()) {
            for (auto& el : j_alphas.items()) {

                std::string key = el.key();
                auto val = el.value();
                double dval = 1.0;
                if (val.is_number()) {
                    dval = val.get<double>();
                } else {
                    // TODO: log bad alpha entry type
                    continue;
                }

                if (key == "default") {
                    default_alpha = dval;
                } else {
                    try {
                        int ikey = std::stoi(key);
                        alpha[ikey] = dval;
                    } catch ( std::invalid_argument & e ) {
                        // TODO: log bad alpha key type
                        continue;
                    }
                }
            }
        } else {
            // TODO: log bad "alphas" type
        }

        // Parse similarity functions

        std::map<int, SimFuncMap> functions;
        SimFuncMap default_functions;

        // TODO: handle no "functions" key
        auto j_functions = j["functions"];

        if (j_functions.is_object()) {
            for (auto& el : j_functions.items()) {

                std::string key = el.key();
                auto val = el.value();

                SimFuncMap _functions;

                if (val.is_array()) {
                    _functions = _parse_function_list(val);
                } else {
                    // TODO: log bad functions entry type
                    continue;
                }

                if (key == "default") {
                    default_functions = _functions;
                } else {
                    try {
                        int ikey = std::stoi(key);
                        functions[ikey] = _functions;
                    } catch ( std::invalid_argument & e ) {
                        // TODO: log bad function key type
                        continue;
                    }
                }

            }
        } else {
            // TODO: log bad "functions" type
        }

        Similarity similarity(
            alpha, default_alpha,
            functions, default_functions
        );
        return similarity;
    }


};
