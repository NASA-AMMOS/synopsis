/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * @see RuleAST.hpp
 */
#include <limits>
#include <nlohmann/json.hpp>
#include <fstream>

#include "RuleAST.hpp"


namespace Synopsis {

    void RuleExpression::set_logger(Logger *logger) {
        this->_logger = logger;
    }

    /**
     * Determine the object type within the JSON AST representation using the
     * `__type__` field.
     *
     * @param[in] j_obj: JSON object representation of AST element
     * @param[out] status: reference to status variable that will be set to
     * return status
     *
     * @return: string object type
     */
    std::string _get_obj_type(nlohmann::json &j_obj, Status& status, Logger *logger);

    /**
     * Get JSON argument object from an AST element within the `__contents__`
     * field
     *
     * @param[out] results: pointer to location where extracted argument object
     * will be stored
     * @param[in] j_obj: JSON object representation of an AST element
     * @param[in] arg: string argument name
     *
     * @return: SUCCESS if the argument exists; or an error code
     */
    Status _get_argument_obj(
        nlohmann::json *result, nlohmann::json &j_obj, std::string arg, Logger *logger
    );

    /**
     * Generic prototype to parse an expression argument of a given type.
     *
     * @param[in] j_obj: JSON object representation of argument; the argument
     * will be within the `__contents__` field of the object
     * @param[in] arg: string argument name
     * @param[out] exprs: vector of parsed expression shared pointers to
     * maintain persistence in memory
     * @param[out] status: reference to status variable that will be set to
     * return status
     *
     * @return: parsed argument value, or pointer to a sub-expression
     */
    template<class T> T _parse_argument(
        nlohmann::json &j_obj, std::string arg,
        std::vector<std::shared_ptr<RuleExpression>> &exprs,
        Status& status,
        Logger *logger
    );

    /**
     * Parse a rule definition from JSON AST representation
     *
     * @param[in] j_rule: JSON object representation of a rule
     * @param[out] exprs: vector of parsed expression shared pointers to
     * maintain persistence in memory
     * @param[out] status: reference to status variable that will be set to
     * return status
     *
     * @return: parsed rule
     */
    Rule _parse_rule(
        nlohmann::json &j_rule,
        std::vector<std::shared_ptr<RuleExpression>> &exprs,
        Status& status,
        Logger *logger
    );

    /**
     * Parse a constraint definition from JSON AST representation
     *
     * @param[in] j_constraint: JSON object representation of a constraint
     * @param[out] exprs: vector of parsed expression shared pointers to
     * maintain persistence in memory
     * @param[out] status: reference to status variable that will be set to
     * return status
     *
     * @return: parsed constraint
     */
    Constraint _parse_constraint(
        nlohmann::json &j_constraint,
        std::vector<std::shared_ptr<RuleExpression>> &exprs,
        Status& status,
        Logger *logger
    );

    /**
     * Parses a list of rules and constraints from a priority bin within a JSON
     * configuration.
     *
     * @param[in] j_bin: JSON representation of priority bin configuration
     * @param[out] exprs: vector of parsed expression shared pointers to
     * maintain persistence in memory
     *
     * @return: pair containing a parsed rule list and a constraint list
     */
    std::pair<RuleList, ConstraintList>
    _parse_bin(
            nlohmann::json &j_bin,
            std::vector<std::shared_ptr<RuleExpression>> &exprs,
            Logger *logger
    );


    Rule::Rule(
        std::vector<std::string> variables,
        BoolValueExpression *application_expression,
        ValueExpression *adjustment_expression,
        int max_applications,
        Logger *logger
    ) :
        _variables(variables),
        _application_expression(application_expression),
        _adjustment_expression(adjustment_expression),
        _max_applications(max_applications),
        _logger(logger)
    {

    }

    double Rule::apply(AsdpList asdps) {

        int n_applications = 0;
        double total_adj_value = 0.0;
        DpMetadataValue adj;

        if (_variables.size() == 1) {
            for (auto a : asdps) {
                AsdpAssignments assignments = {
                    {_variables[0], a}
                };
                if (_application_expression->get_value(assignments, asdps)) {
                    adj = _adjustment_expression->get_value(assignments, asdps);
                    if (adj.is_numeric()) {
                        total_adj_value += adj.get_numeric();
                        n_applications += 1;
                    } else {
                        LOG(this->_logger, Synopsis::LogType::ERROR, "Applicaton/adjustment failed due to non-numeric adjustment value");
                    }
                    // TODO: Log application of rule. Should this print the adjustment? 
                    if ((_max_applications >= 0) && (n_applications >= _max_applications)) {
                        break;
                    }
                }
            }
            return total_adj_value;

        } else if (_variables.size() == 2) {
            for (auto a : asdps) {
                for (auto b : asdps) {
                    AsdpAssignments assignments = {
                        {_variables[0], a},
                        {_variables[1], b}
                    };
                    if (_application_expression->get_value(assignments, asdps)) {
                        adj = _adjustment_expression->get_value(assignments, asdps);
                        if (adj.is_numeric()) {
                            total_adj_value += adj.get_numeric();
                            n_applications += 1;
                        } else {
                            LOG(this->_logger, Synopsis::LogType::ERROR,"No application/adjustment, adjustment expression should be numeric");
                        }
                        // TODO: Log application of rule. Should this print the adjustment?
                        if ((_max_applications >= 0) && (n_applications >= _max_applications)) {
                            break;
                        }
                    }
                }
                if ((_max_applications >= 0) && (n_applications >= _max_applications)) {
                    break;
                }
            }
            return total_adj_value;

        } else {
            // TODO: Case not handled
            LOG(this->_logger, Synopsis::LogType::ERROR,"Ignoring rules with more than 2 variables specified; currently unsupported");
            return total_adj_value;
        }

        /* UNREACHABLE */

    }


    Constraint::Constraint(
        std::vector<std::string> variables,
        BoolValueExpression *application_expression,
        ValueExpression *sum_field,
        double constraint_value,
        Logger *logger
    ) :
        _variables(variables),
        _application_expression(application_expression),
        _sum_field(sum_field),
        _constraint_value(constraint_value),
        _logger(logger)
    {

    }


    bool Constraint::apply(AsdpList asdps) {

        double aggregate = 0.0;
        DpMetadataValue value;

        if (_variables.size() == 1) {
            for (auto a : asdps) {
                AsdpAssignments assignments = {
                    {_variables[0], a}
                };
                if (_application_expression->get_value(assignments, asdps)) {
                    if (_sum_field) {
                        value = _sum_field->get_value(assignments, asdps);
                        if (value.is_numeric()) {
                            aggregate += value.get_numeric();
                        } else {
                            LOG(this->_logger, Synopsis::LogType::ERROR,  "Non-numeric value prevented aggregation while applying constraint");
                        }
                    } else {
                        aggregate += 1;
                    }
                }
            }
            return aggregate < _constraint_value;

        } else {
            // TODO: Case not handled
            return true;
        }

        /* UNREACHABLE */

    }


    RuleSet::RuleSet():
        _rule_map({}),
        _constraint_map({}),
        _default_rules({}),
        _default_constraints({}),
        _expressions({}),
        _logger(nullptr)
    {

    }


    RuleSet::RuleSet(
        std::map<int, RuleList> rule_map,
        std::map<int, ConstraintList> constraint_map,
        RuleList default_rules,
        ConstraintList default_constraints,
        Logger *logger
    ) :
        _rule_map(rule_map),
        _constraint_map(constraint_map),
        _default_rules(default_rules),
        _default_constraints(default_constraints),
        _logger(logger)
    {

    }


    RuleSet::RuleSet(
        std::map<int, RuleList> rule_map,
        std::map<int, ConstraintList> constraint_map,
        RuleList default_rules,
        ConstraintList default_constraints,
        std::vector<std::shared_ptr<RuleExpression>> expressions,
        Logger *logger
    ) :
        _rule_map(rule_map),
        _constraint_map(constraint_map),
        _default_rules(default_rules),
        _default_constraints(default_constraints),
        _expressions(expressions),
        _logger(logger)
    {

    }


    RuleList RuleSet::get_rules(int bin) {
        if (this->_rule_map.count(bin)) {
            return this->_rule_map[bin];
        } else {
            return this->_default_rules;
        }
    }


    ConstraintList RuleSet::get_constraints(int bin) {
        if (this->_constraint_map.count(bin)) {
            return this->_constraint_map[bin];
        } else {
            return this->_default_constraints;
        }
    }


    std::pair<bool, double> RuleSet::apply(
        int bin,
        AsdpList queue
    ) {

        // Check constraints
        ConstraintList constraints = this->get_constraints(bin);
        bool violated = false;
        unsigned int num_constraints = constraints.size();
        // for (auto &constraint : constraints) {
        for(unsigned int i = 0; i < num_constraints; i++){
            Constraint constraint = constraints[i];
            if (!constraint.apply(queue)) {
                violated = true;
                //TODO: create a printable string representation of constraitns and call that here to report which constraint is violated instead logging of loop index
                LOG(this->_logger, Synopsis::LogType::INFO,   "Violated constraint index: %ld ", i); 
                
                break;
            }
        }
        if (violated) {
            return std::make_pair(false, 0.0);
        }

        // Apply rules
        RuleList rules = this->get_rules(bin);
        double utility = 0.0;
        for (auto &rule : rules) {
            double adj = rule.apply(queue);
            utility += adj;
        }

        return std::make_pair(true, utility);
    }


    LogicalConstant::LogicalConstant(bool value) :
        _value(value)
    {

    }


    bool LogicalConstant::get_value(
            AsdpAssignments assignments,
            AsdpList asdps
        ) {
        return this->_value;
    }


    ConstExpression::ConstExpression(double value) :
        _value(DpMetadataValue(value))
    {

    }

    DpMetadataValue ConstExpression::get_value(
            AsdpAssignments assignments,
            AsdpList asdps
        ) {
        return this->_value;
    }


    LogicalNot::LogicalNot(BoolValueExpression *expr) :
        _expr(expr)
    {

    }


    bool LogicalNot::get_value(
            AsdpAssignments assignments,
            AsdpList asdps
        ) {
        return !(this->_expr->get_value(assignments, asdps));
    }


    BinaryLogicalExpression::BinaryLogicalExpression(
        std::string op,
        BoolValueExpression *left_expr,
        BoolValueExpression *right_expr
    ) :
        _op(op),
        _left_expr(left_expr),
        _right_expr(right_expr)
    {

    }


    bool BinaryLogicalExpression::get_value(
            AsdpAssignments assignments,
            AsdpList asdps
        ) {
        bool left_value = this->_left_expr->get_value(assignments, asdps);
        if (this->_op == "AND") {
            if (left_value) {
                // Evaluate right-hand expression only if left-hand expression
                // evaluates to true
                return this->_right_expr->get_value(assignments, asdps);
            } else {
                // If left-hand expression evaluates to false; short circuit
                // expression evaluation
                return false;
            }
        } else if (this->_op == "OR") {
            if (left_value) {
                // If left-hand expression evaluates to true; short circuit
                // expression evaluation
                return true;
            } else {
                // Evaluate right-hand expression only if left-hand expression
                // evaluates to false
                return this->_right_expr->get_value(assignments, asdps);
            }
        } else {
            LOG(this->_logger, Synopsis::LogType::ERROR,  "invalid operator %s in binary logical expression", this->_op.c_str());

            return false;
        }
    }


    ComparatorExpression::ComparatorExpression(
        std::string comp,
        ValueExpression *left_expr,
        ValueExpression *right_expr
    ) :
        _comp(comp),
        _left_expr(left_expr),
        _right_expr(right_expr)
    {

    }


    bool ComparatorExpression::get_value(
            AsdpAssignments assignments,
            AsdpList asdps
        ) {
        DpMetadataValue left_value = this->_left_expr->get_value(assignments, asdps);
        DpMetadataValue right_value = this->_right_expr->get_value(assignments, asdps);
        if (left_value.is_numeric() ^ right_value.is_numeric()) {
            if (left_value.is_numeric()){
                LOG(this->_logger, Synopsis::LogType::ERROR, "type mismatch in ComparatorExpression::get_value, only left value is numeric");
            }else{
                LOG(this->_logger, Synopsis::LogType::ERROR,  "type mismatch in ComparatorExpression::get_value, only right value is numeric");
            }
            
            return false;
        }
        if (left_value.is_numeric()) {
            double left_value_dbl = left_value.get_numeric();
            double right_value_dbl = right_value.get_numeric();
            if (this->_comp == "==") {
                return (left_value_dbl == right_value_dbl);
            } else if (this->_comp == "!=") {
                return (left_value_dbl != right_value_dbl);
            } else if (this->_comp == ">") {
                return (left_value_dbl > right_value_dbl);
            } else if (this->_comp == ">=") {
                return (left_value_dbl >= right_value_dbl);
            } else if (this->_comp == "<") {
                return (left_value_dbl < right_value_dbl);
            } else if (this->_comp == "<=") {
                return (left_value_dbl <= right_value_dbl);
            } else {
                LOG(this->_logger, Synopsis::LogType::ERROR, "unknown string comparison %s in ComparatorExpression::get_value", this->_comp.c_str());
                return false;
            }
        } else {
            std::string left_value_str = left_value.get_string_value();
            std::string right_value_str = right_value.get_string_value();
            if (this->_comp == "==") {
                return (left_value_str == right_value_str);
            } else if (this->_comp == "!=") {
                return (left_value_str != right_value_str);
            } else {
                LOG(this->_logger, Synopsis::LogType::ERROR, "unknown string comparision %s in ComparatorExpression::get_value", this->_comp.c_str());
                return false;
            }
        }
    }


    StringConstant::StringConstant(std::string value) :
        _value(DpMetadataValue(value))
    {

    }

    DpMetadataValue StringConstant::get_value(
            AsdpAssignments assignments,
            AsdpList asdps
        ) {
        return this->_value;
    }


    MinusExpression::MinusExpression(ValueExpression *expr) :
        _expr(expr)
    {

    }

    DpMetadataValue MinusExpression::get_value(
            AsdpAssignments assignments,
            AsdpList asdps
        ) {
        DpMetadataValue value = this->_expr->get_value(assignments, asdps);
        if (value.is_numeric()) {
            return DpMetadataValue(-value.get_numeric());
        } else {
            LOG(this->_logger, Synopsis::LogType::WARN, "Not a number in MinusExpression::get_value");
            return DpMetadataValue(std::numeric_limits<double>::quiet_NaN());
        }
    }


    BinaryExpression::BinaryExpression(
        std::string op,
        ValueExpression *left_expr,
        ValueExpression *right_expr
    ) :
        _op(op),
        _left_expr(left_expr),
        _right_expr(right_expr)
    {

    }


    DpMetadataValue BinaryExpression::get_value(
            AsdpAssignments assignments,
            AsdpList asdps
        ) {
        DpMetadataValue left_value = this->_left_expr->get_value(assignments, asdps);
        DpMetadataValue right_value = this->_right_expr->get_value(assignments, asdps);
        if (left_value.is_numeric() && right_value.is_numeric()) {
            double left_value_dbl = left_value.get_numeric();
            double right_value_dbl = right_value.get_numeric();
            if (this->_op == "*") {
                return DpMetadataValue(left_value_dbl * right_value_dbl);
            } else if (this->_op == "+") {
                return DpMetadataValue(left_value_dbl + right_value_dbl);
            } else if (this->_op == "-") {
                return DpMetadataValue(left_value_dbl - right_value_dbl);
            } else {
                LOG(this->_logger, Synopsis::LogType::WARN, "Operator %s not supported in BinaryExpression::get_value", this->_op.c_str());
                return DpMetadataValue(std::numeric_limits<double>::quiet_NaN());
            }
        } else {
            if (left_value.is_numeric()){
                LOG(this->_logger, Synopsis::LogType::WARN, "Right value not numeric in BinaryExpression::get_value");

            } else {
                LOG(this->_logger, Synopsis::LogType::WARN,  "Left value not numeric in BinaryExpression::get_value");
            }
            
            return DpMetadataValue(std::numeric_limits<double>::quiet_NaN());
        }
    }


    Field::Field(
        std::string var_name,
        std::string field_name
    ) :
        _var_name(var_name),
        _field_name(field_name)
    {

    }


    DpMetadataValue Field::get_value(
            AsdpAssignments assignments,
            AsdpList asdps
        ) {
        if (assignments.count(this->_var_name) > 0) {
            auto fields = assignments[this->_var_name];
            if (fields.count(this->_field_name) > 0) {
                return fields[this->_field_name];
            } else {
                // TODO: Field not found
                return DpMetadataValue(
                    std::numeric_limits<double>::quiet_NaN()
                );
            }

        } else {
            // TODO: Variable not found
            return DpMetadataValue(std::numeric_limits<double>::quiet_NaN());
        }
    }


    ExistentialExpression::ExistentialExpression(
        std::string variable,
        BoolValueExpression *expr
    ) :
        _var(variable),
        _expr(expr)
    {

    }


    bool ExistentialExpression::get_value(
            AsdpAssignments assignments,
            AsdpList asdps
        ) {
        for (auto asdp : asdps) {

            // Assign asdp to variable
            AsdpAssignments new_assignments(assignments);
            new_assignments[this->_var] = asdp;

            // Evaluate expression and return if true
            bool evaluation = this->_expr->get_value(new_assignments, asdps);
            if (evaluation) { return true; }

        }
        return false;
    }


    Status _get_argument_obj(
        nlohmann::json *result, nlohmann::json &j_obj, std::string arg, Logger *logger
    ) {
        if (!j_obj.is_object()) {
            LOG(logger, Synopsis::LogType::ERROR, "Expected (key, value) pair in _get_argument_obj for arg: %s , but got: %s", arg.c_str(), j_obj.dump().c_str()); 
            return FAILURE;
        }

        // TODO: Check for key
        auto j_contents = j_obj["__contents__"];
        if (!j_contents.is_object()) {
            LOG(logger, Synopsis::LogType::ERROR, "Expected value of __contents__ key of %s to be (key, value) pair in _get_argument_obj", j_obj.dump().c_str());
            return FAILURE;
        }

        // TODO: Check for key
        *result = j_contents[arg];
        return SUCCESS;
    }


    std::string _get_obj_type(nlohmann::json &j_obj, Status& status, Logger *logger) {
        status = FAILURE;
        if (!j_obj.is_object()) {
            LOG(logger, Synopsis::LogType::ERROR, "Expected (key, value) pair in _get_obj_type but got: %s", j_obj.dump().c_str()); 
            return "";
        }
        auto j_type = j_obj["__type__"];
        if (!j_type.is_string()) {
            LOG(logger, Synopsis::LogType::ERROR, "Expected value of __type__ key of %s to be a string in _get_obj_type, status:  %ld", j_obj.dump().c_str(), status);
            return "";
        }
        status = SUCCESS;
        return j_type.get<std::string>();
    }


    template<> std::vector<std::string> _parse_argument(
        nlohmann::json &j_obj, std::string arg,
        std::vector<std::shared_ptr<RuleExpression>> &exprs,
        Status& status,
        Logger *logger
    ) {
        std::vector<std::string> bad = {};
        nlohmann::json j_arg;
        status = _get_argument_obj(&j_arg, j_obj, arg, logger);
        if (status != SUCCESS) { return bad; }

        if (j_arg.is_array()) {
            std::vector<std::string> result;
            for (auto& j_v : j_arg) {
                if (j_v.is_string()) {
                    result.push_back(j_v.get<std::string>());
                }
            }
            status = SUCCESS;
            return result;
        }

        status = FAILURE;
        return bad;
    }


    template<> std::string _parse_argument(
        nlohmann::json &j_obj, std::string arg,
        std::vector<std::shared_ptr<RuleExpression>> &exprs,
        Status& status,
        Logger *logger
    ) {
        std::string bad = "";
        nlohmann::json j_arg;
        status = _get_argument_obj(&j_arg, j_obj, arg, logger);
        if (status != SUCCESS) { return bad; }

        if (j_arg.is_string()) {
            status = SUCCESS;
            return j_arg.get<std::string>();
        }

        status = FAILURE;
        return bad;
    }


    template<> int _parse_argument(
        nlohmann::json &j_obj, std::string arg,
        std::vector<std::shared_ptr<RuleExpression>> &exprs,
        Status& status,
        Logger *logger
    ) {
        int bad = -1;
        nlohmann::json j_arg;
        status = _get_argument_obj(&j_arg, j_obj, arg, logger);
        if (status != SUCCESS) { return bad; }

        if (j_arg.is_number_integer()) {
            status = SUCCESS;
            return j_arg.get<int>();
        } else {
            status = FAILURE;
            LOG(logger, Synopsis::LogType::ERROR, "argument is not an integer in _parse_argument, status:  %ld", status);
            return bad;
        }

    }


    template<> double _parse_argument(
        nlohmann::json &j_obj, std::string arg,
        std::vector<std::shared_ptr<RuleExpression>> &exprs,
        Status& status,
        Logger *logger
    ) {
        double bad = 0.0;
        nlohmann::json j_arg;
        status = _get_argument_obj(&j_arg, j_obj, arg, logger);
        if (status != SUCCESS) { return bad; }

        if (j_arg.is_number()) {
            status = SUCCESS;
            return j_arg.get<double>();
        } else {
            status = FAILURE;
            LOG(logger, Synopsis::LogType::ERROR, "argument is not a double in _parse_argument, status:  %ld", status);
            return bad;
        }

    }


    template<> bool _parse_argument(
        nlohmann::json &j_obj, std::string arg,
        std::vector<std::shared_ptr<RuleExpression>> &exprs,
        Status& status,
        Logger *logger
    ) {
        bool bad = false;
        nlohmann::json j_arg;
        status = _get_argument_obj(&j_arg, j_obj, arg, logger);
        if (status != SUCCESS) { return bad; }

        if (j_arg.is_boolean()) {
            status = SUCCESS;
            return j_arg.get<bool>();
        } else {
            status = FAILURE;
            LOG(logger, Synopsis::LogType::ERROR, "argument is not a boolean in _parse_argument, status:  %ld", status);
            return bad;
        }

    }


    template<> ValueExpression* _parse_argument(
        nlohmann::json &j_obj, std::string arg,
        std::vector<std::shared_ptr<RuleExpression>> &exprs,
        Status& status,
        Logger *logger
    ) {
        ValueExpression* bad = nullptr;
        nlohmann::json j_arg;
        status = _get_argument_obj(&j_arg, j_obj, arg, logger);
        if (status != SUCCESS) { return bad; }

        // Allow null entries for expressions to support `sum_field`
        if(j_arg.is_null()) {
            status = SUCCESS;
            return nullptr;
        }

        std::string type = _get_obj_type(j_arg, status, logger);
        if (status != SUCCESS) { return bad; }

        std::shared_ptr<ValueExpression> ptr;

        if (type == "ConstExpression") {
            double value = _parse_argument<double>(j_arg, "value", exprs, status, logger);
            if (status != SUCCESS) {
                status = FAILURE;
                return bad;
            }
            ptr = std::make_shared<ConstExpression>(
                ConstExpression(value)
            );

        } else if (type == "StringConstant") {
            std::string value = _parse_argument<std::string>(
                j_arg, "value", exprs, status, logger
            );
            if (status != SUCCESS) {
                status = FAILURE;
                return bad;
            }
            ptr = std::make_shared<StringConstant>(
                StringConstant(value)
            );

        } else if (type == "MinusExpression") {
            ValueExpression *expr = _parse_argument<ValueExpression*>(
                j_arg, "expression", exprs, status, logger
            );
            if (status != SUCCESS) {
                status = FAILURE;
                return bad;
            }
            ptr = std::make_shared<MinusExpression>(
                MinusExpression(expr)
            );

        } else if (type == "BinaryExpression") {
            std::string op = _parse_argument<std::string>(
                j_arg, "operator", exprs, status, logger
            );
            if (status != SUCCESS) {
                status = FAILURE;
                return bad;
            }

            ValueExpression *left_expr = _parse_argument<ValueExpression*>(
                j_arg, "left_expression", exprs, status, logger
            );
            if (status != SUCCESS) {
                status = FAILURE;
                return bad;
            }

            ValueExpression *right_expr = _parse_argument<ValueExpression*>(
                j_arg, "right_expression", exprs, status, logger
            );
            if (status != SUCCESS) {
                status = FAILURE;
                return bad;
            }

            ptr = std::make_shared<BinaryExpression>(
                BinaryExpression(op, left_expr, right_expr)
            );

        } else if (type == "Field") {
            std::string var_name = _parse_argument<std::string>(
                j_arg, "variable_name", exprs, status, logger
            );
            if (status != SUCCESS) {
                status = FAILURE;
                return bad;
            }

            std::string field_name = _parse_argument<std::string>(
                j_arg, "field_name", exprs, status, logger
            );
            if (status != SUCCESS) {
                status = FAILURE;
                return bad;
            }

            ptr = std::make_shared<Field>(
                Field(var_name, field_name)
            );

        } else {
            status = FAILURE;
            return bad;
        }


        status = SUCCESS;
        exprs.push_back(ptr);
        ptr->set_logger(logger);
        return ptr.get();
    }


    template<> BoolValueExpression* _parse_argument(
        nlohmann::json &j_obj, std::string arg,
        std::vector<std::shared_ptr<RuleExpression>> &exprs,
        Status& status, 
        Logger *logger
    ) {
        BoolValueExpression* bad = nullptr;
        nlohmann::json j_arg;
        status = _get_argument_obj(&j_arg, j_obj, arg, logger);
        if (status != SUCCESS) { return bad; }

        std::string type = _get_obj_type(j_arg, status, logger);
        if (status != SUCCESS) { return bad; }

        std::shared_ptr<BoolValueExpression> ptr;

        if (type == "LogicalConstant") {
            bool value = _parse_argument<bool>(j_arg, "value", exprs, status, logger);
            if (status != SUCCESS) {
                status = FAILURE;
                return bad;
            }

            ptr = std::make_shared<LogicalConstant>(
                LogicalConstant(value)
            );

        } else if (type == "LogicalNot") {
            BoolValueExpression *expr = _parse_argument<BoolValueExpression*>(
                j_arg, "expression", exprs, status, logger
            );
            if (status != SUCCESS) {
                status = FAILURE;
                return bad;
            }

            ptr = std::make_shared<LogicalNot>(
                LogicalNot(expr)
            );

        } else if (type == "BinaryLogicalExpression") {
            std::string op = _parse_argument<std::string>(
                j_arg, "operator", exprs, status, logger
            );
            if (status != SUCCESS) {
                status = FAILURE;
                return bad;
            }

            BoolValueExpression *left_expr = _parse_argument<BoolValueExpression*>(
                j_arg, "left_expression", exprs, status, logger
            );
            if (status != SUCCESS) {
                status = FAILURE;
                return bad;
            }

            BoolValueExpression *right_expr = _parse_argument<BoolValueExpression*>(
                j_arg, "right_expression", exprs, status, logger
            );
            if (status != SUCCESS) {
                status = FAILURE;
                return bad;
            }

            ptr = std::make_shared<BinaryLogicalExpression>(
                BinaryLogicalExpression(op, left_expr, right_expr)
            );

        } else if (type == "ComparatorExpression") {
            std::string comp = _parse_argument<std::string>(
                j_arg, "comparator", exprs, status, logger
            );
            if (status != SUCCESS) {
                status = FAILURE;
                return bad;
            }

            ValueExpression *left_expr = _parse_argument<ValueExpression*>(
                j_arg, "left_expression", exprs, status, logger
            );
            if (status != SUCCESS) {
                status = FAILURE;
                return bad;
            }

            ValueExpression *right_expr = _parse_argument<ValueExpression*>(
                j_arg, "right_expression", exprs, status, logger
            );
            if (status != SUCCESS) {
                status = FAILURE;
                return bad;
            }

            ptr = std::make_shared<ComparatorExpression>(
                ComparatorExpression(comp, left_expr, right_expr)
            );

        } else if (type == "ExistentialExpression") {
            std::string variable = _parse_argument<std::string>(
                j_arg, "variable", exprs, status, logger
            );
            if (status != SUCCESS) {
                status = FAILURE;
                return bad;
            }

            BoolValueExpression *expr = _parse_argument<BoolValueExpression*>(
                j_arg, "expression", exprs, status, logger
            );
            if (status != SUCCESS) {
                status = FAILURE;
                return bad;
            }

            ptr = std::make_shared<ExistentialExpression>(
                ExistentialExpression(variable, expr)
            );

        } else {

            status = FAILURE;
            return bad;
        }

        status = SUCCESS;
        exprs.push_back(ptr);
        ptr->set_logger(logger);
        return ptr.get();
    }


    Constraint _parse_constraint(
        nlohmann::json &j_constraint,
        std::vector<std::shared_ptr<RuleExpression>> &exprs,
        Status& status,
        Logger *logger
    ) {
        Constraint bad = Constraint({}, nullptr, nullptr, 0.0, logger);

        std::string type = _get_obj_type(j_constraint, status, logger);
        if (status != SUCCESS || type != "Constraint") {
            LOG(logger, Synopsis::LogType::WARN, "Expected Constraint type, but encountered %s type while parsing constraint, status:  %ld", type.c_str(), status);
            status = FAILURE;
            return bad;
        }

        auto variables = _parse_argument<std::vector<std::string>>(
            j_constraint, "variables", exprs, status, logger
        );
        if (status != SUCCESS) {
            LOG(logger, Synopsis::LogType::WARN, "while parsing string argument in _parse_constraint, status:  %ld", status);
            status = FAILURE;
            return bad;
        }

        auto application = _parse_argument<BoolValueExpression*>(
            j_constraint, "application", exprs, status, logger
        );
        if (status != SUCCESS) {
            LOG(logger, Synopsis::LogType::WARN, "while parsing boolean argument in _parse_constraint, status:  %ld", status);
            status = FAILURE;
            return bad;
        }

        auto sum_field = _parse_argument<ValueExpression*>(
            j_constraint, "sum_field", exprs, status, logger
        );
        if (status != SUCCESS) {
            LOG(logger, Synopsis::LogType::WARN, "while parsing value argument in _parse_constraint, status:  %ld", status);
            status = FAILURE;
            return bad;
        }

        auto constraint_value = _parse_argument<double>( 
            j_constraint, "constraint_value", exprs, status, logger
        );
        if (status != SUCCESS) {
            LOG(logger, Synopsis::LogType::WARN, "while parsing double argument in _parse_constraint, status:  %ld", status);
            status = FAILURE;
            return bad;
        }

        status = SUCCESS;
        return Constraint(
            variables, application, sum_field, constraint_value, logger
        );

    }


    Rule _parse_rule(
        nlohmann::json &j_rule,
        std::vector<std::shared_ptr<RuleExpression>> &exprs,
        Status& status,
        Logger *logger
    ) {
        Rule bad = Rule({}, nullptr, nullptr, 0, logger);

        std::string type = _get_obj_type(j_rule, status, logger);
        if (status != SUCCESS || type != "Rule") {
            LOG(logger, Synopsis::LogType::WARN, "Expected Rule type, but encountered %s type while parsing rule, status:  %ld", type.c_str(), status);
            status = FAILURE;
            return bad;
        }

        auto variables = _parse_argument<std::vector<std::string>>(
            j_rule, "variables", exprs, status, logger
        );
        if (status != SUCCESS) {
            LOG(logger, Synopsis::LogType::WARN, "while parsing string argument in _parse_rule, status:  %ld", status);
            status = FAILURE;
            return bad;
        }

        auto application = _parse_argument<BoolValueExpression*>(
            j_rule, "application", exprs, status, logger
        );
        if (status != SUCCESS) {
            LOG(logger, Synopsis::LogType::WARN, "while parsing boolean argument in _parse_rule, status:  %ld", status);
            status = FAILURE;
            return bad;
        }

        auto adjustment = _parse_argument<ValueExpression*>(
            j_rule, "adjustment", exprs, status, logger
        );
        if (status != SUCCESS) {
            LOG(logger, Synopsis::LogType::WARN, "while parsing value argument in _parse_rule, status:  %ld", status);
            status = FAILURE;
            return bad;
        }

        auto max_applications = _parse_argument<int>(
            j_rule, "max_applications", exprs, status, logger
        );
        if (status != SUCCESS) {
            LOG(logger, Synopsis::LogType::WARN, "while parsing integer argument in _parse_rule, status:  %ld", status);
            // If there is no max_applications specified, use -1
            max_applications = -1;
        }

        status = SUCCESS;
        return Rule(
            variables, application, adjustment, max_applications, logger
        );

    }


    std::pair<RuleList, ConstraintList>
    _parse_bin(
            nlohmann::json &j_bin,
            std::vector<std::shared_ptr<RuleExpression>> &exprs,
            Logger *logger
    ) {
        Status status = SUCCESS;
        RuleList rules;
        ConstraintList constraints;

        // TODO: check for key
        auto j_rules = j_bin["rules"];
        if (j_rules.is_array()) {
            for (auto j_rule : j_rules) {
                Rule rule = _parse_rule(j_rule, exprs, status, logger);
                if (status == SUCCESS) {
                    rules.push_back(rule);
                } else {
                    LOG(logger, Synopsis::LogType::ERROR, "error while parsing rule in parse_bin, status:  %ld, rule: %s", status, j_rule.dump().c_str());
                }
            }
        }

        // TODO: check for key
        auto j_constraints = j_bin["constraints"];
        if (j_constraints.is_array()) {
            for (auto j_constraint : j_constraints) {
                Constraint constraint = _parse_constraint(j_constraint, exprs, status, logger);
                if (status == SUCCESS) {
                    constraints.push_back(constraint);
                } else {
                    LOG(logger, Synopsis::LogType::ERROR, "error while parsing constraint in parse_bin, status:  %ld", status);
                }
            }
        }

        return std::make_pair(rules, constraints);
    }


    RuleSet parse_rule_config(std::string config_file, Logger *logger) {
        // If no config file provided, return default configuration
        if (config_file.size() == 0) {
            return RuleSet();
        }

        std::ifstream file_input(config_file);
        auto j = nlohmann::json::parse(file_input);

        if (!j.is_object()) {
            LOG(logger, Synopsis::LogType::WARN, "JSON parse result is not an object in parse_rule_config");
            return RuleSet();
        }

        std::map<int, RuleList> rule_map;
        std::map<int, ConstraintList> constraint_map;
        RuleList default_rules;
        ConstraintList default_constraints;
        std::vector<std::shared_ptr<RuleExpression>> exprs;

        for (auto& el : j.items()) {
            std::string key = el.key();
            auto val = el.value();

            auto bin_rules = _parse_bin(val, exprs, logger);

            if (key == "default") {
                default_rules = bin_rules.first;
                default_constraints = bin_rules.second;
            } else {
                try {
                    int ikey = std::stoi(key);
                    rule_map[ikey] = bin_rules.first;
                    constraint_map[ikey] = bin_rules.second;
                } catch ( std::invalid_argument & e ) {
                    LOG(logger, Synopsis::LogType::ERROR,  "Bad alpha key type in parse_rule_config"); // log bad alpha key type
                    continue;
                }
            }
        }

        return RuleSet(
            rule_map, constraint_map,
            default_rules, default_constraints,
            exprs, logger
        );
    }


};
