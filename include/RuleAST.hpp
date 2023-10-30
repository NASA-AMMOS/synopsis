/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * Provides classes used for the abstract syntax tree (AST) representation of
 * parsed prioritization rules and constraints. These classes also provide
 * functionality to evaluate rules and constraints. Functionality is also
 * provided for parsing an AST from a JSON configuration file.
 */
#ifndef JPL_SYNOPSIS_RuleAST
#define JPL_SYNOPSIS_RuleAST

#include <vector>
#include <map>
#include <utility>
#include <memory>

#include "synopsis_types.hpp"
#include "DpDbMsg.hpp"
#include "Logger.hpp"

namespace Synopsis {

    /**
     * An abstract generic expression within a rule or constraint definition.
     */
    class RuleExpression {

        public:

            /**
             * Default virtual destructor
             */
            virtual ~RuleExpression() = default;

            // declare function set_parent which gets called when children are defined. Create protected members of the RuleExpression class to point to. Do we need this still?
            
            /**
             * Sets the logger instance to be used for this expression
             *
             * @param[in] logger: pointer to a logger instance to be used by
             * the module
             *
             */
            void set_logger( Logger* logger );


            // /**
            //  * Logs a message of the specified type, using a format string with
            //  * arguments.
            //  *
            //  * @param[in] type: log message type
            //  * @param[in] fmt: C-style format string
            //  * @param[in] ...: variable argument list for format string
            //  */
            // void log(LogType type, const char* fmt, ...); // declare log function to actually do the logging, this would do a null pointer check first, in which case don't do any logging. define it the same way as the log function in logger.h. See if this is easy to do otherwise just call the logger.h's log
            
        protected:
            /* Reference to the logger instance to be used by this module
            */
            Logger *_logger = nullptr; 


    };


    /**
     * An abstract expression within a rule or constraint definition that
     * returns a Boolean value upon evaluation.
     */
    class BoolValueExpression : public RuleExpression {


        public:

            /**
             * Default virtual destructor
             */
            virtual ~BoolValueExpression() = default;

            /**
             * Abstract function that returns the value of this expression.
             *
             * @param[in] assignments: a list of ASDPs assigned to variable names
             * @param[in] asdps: a list of ASDPs in the downlink queue
             *
             * @return: Boolean value
             */
            virtual bool get_value(
                AsdpAssignments assignments,
                AsdpList asdps
            ) = 0;


    };


    /**
     * An abstract expression within a rule or constraint definition that
     * returns a metadata value upon evaluation.
     */
    class ValueExpression : public RuleExpression {


        public:

            /**
             * Default virtual destructor
             */
            virtual ~ValueExpression() = default;

            /**
             * Abstract function that returns the value of this expression.
             *
             * @param[in] assignments: a list of ASDPs assigned to variable names
             * @param[in] asdps: a list of ASDPs in the downlink queue
             *
             * @return: metadata value
             */
            virtual DpMetadataValue get_value(
                AsdpAssignments assignments,
                AsdpList asdps
            ) = 0;


    };


    /**
     * AST representation of a parsed prioritization rule.
     */
    class Rule {


        public:

            /**
             * Constructs a rule from a variable list, application expression,
             * adjustment expression, and maximum number of applications.
             *
             * @param[in] variables: a list of variables in the rule definition
             * @param[in] application_expression: a Boolean-valued expression
             * used to determine if the rule applies
             * @param[in] adjustment_expression: a numeric metadata-valued
             * expression used to adjust the utility if the rule applies
             * @param[in] max_applications: the maximum number of times this
             * rule should be applied; a negative value indicates that there is
             * no limit
             * @param[in] logger: pointer to a logger instance to be used by 
             * the module
             */
            Rule(
                std::vector<std::string> variables,
                BoolValueExpression *application_expression,
                ValueExpression *adjustment_expression,
                int max_applications,
                Logger *logger
            );

            /**
             * Default destructor
             */
            ~Rule() = default;


            /**
             * Returns the total SUE adjustment due to application of rule to a
             * given downlink queue.
             *
             * @param[in] asdps: list of ASDPs in the downlink queue
             *
             * @return: total science utility adjustment
             */
            double apply(AsdpList asdps);


        private:

            /**
             * Stores the list of variable names in the rule definition
             */
            std::vector<std::string> _variables;

            /**
             * Stores a pointer to the application expression
             */
            BoolValueExpression *_application_expression;

            /**
             * Stores a pointer to the adjustment expression
             */
            ValueExpression *_adjustment_expression;

            /**
             * Stores the number of maximum applications
             */
            int _max_applications;

            /**
             * Reference to the logger instance to be used by this module
             */
            Logger *_logger = nullptr;

    };


    /**
     * AST representation of a parsed prioritization constraint.
     */
    class Constraint {


        public:

            /**
             * Constructs a rule from a variable list, application expression,
             * sum field, and constraint value.
             *
             * @param[in] variables: a list of variables in the constraint
             * definition
             * @param[in] application_expression: a Boolean-valued expression
             * used to determine if the constraint applies when aggregating the
             * sum field
             * @param[in] sum_field: a numeric metadata-valued expression that
             * is summed across applicable ASDPs; if null, the count is used
             * @param[in] constraint_value: the constraint upper bound; the
             * constraint is satisfied if the aggregate value is strictly less
             * than this amount
             * @param[in] logger: pointer to a logger instance to be used by 
             * the module
             */
            Constraint(
                std::vector<std::string> variables,
                BoolValueExpression *application_expression,
                ValueExpression *sum_field,
                double constraint_value,
                Logger *logger
            );

            /**
             * Default destructor
             */
            ~Constraint() = default;

            /**
             * Returns the constraint is satisfied for the given downlink
             * queue.
             *
             * @param[in] asdps: list of ASDPs in the downlink queue
             *
             * @return: `true` if the constraint is satisfied, or `false` if
             * not
             */
            bool apply(AsdpList asdps);


        private:

            /**
             * Stores the list of variable names in the rule definition
             */
            std::vector<std::string> _variables;

            /**
             * Stores a pointer to the application expression
             */
            BoolValueExpression *_application_expression;

            /**
             * Stores a pointer to the sum field (or null if count is to be
             * used)
             */
            ValueExpression *_sum_field;

            /**
             * Stores the constraint upper bound
             */
            double _constraint_value;

            /**
             * Reference to the logger instance to be used by this module
             */
            Logger *_logger = nullptr;


    };


    /**
     * Type alias for a list (vector) of rules
     */
    using RuleList = std::vector<Rule>;

    /**
     * Type alias for a list (vector) of constraints
     */
    using ConstraintList = std::vector<Constraint>;


    /**
     * AST representation of a set of rules and constraints across all priority
     * bins.
     */
    class RuleSet {


        public:

            /**
             * Constructs an empty rule set
             */
            RuleSet();

            /**
             * Constructs a rule set from a list of bin-specific rules and
             * constraints, and default lists for all other bins.
             *
             * @param[in] rule_map: mapping of priority bins to rule lists
             * @param[in] constraint_map: mapping of priority bins to
             * constraint lists
             * @param[in] default_rules: a list of default rules to use for all
             * unspecified bins
             * @param[in] default_constraints: a list of default constraints to
             * use for all unspecified bins
             * @param[in] logger: pointer to a logger instance to be used by 
             * the module
             */
            RuleSet(
                std::map<int, RuleList> rule_map,
                std::map<int, ConstraintList> constraint_map,
                RuleList default_rules,
                ConstraintList default_constraints,
                Logger *logger
            );

            /**
             * Constructs a rule set from a list of bin-specific rules and
             * constraints, and default lists for all other bins. This
             * constructor takes an optional argument holding a vector of
             * pointers to dynamically allocated expressions. The purpose of
             * this argument is to ensure that the expressions used by this
             * rule set persist at least until the rule set is destructed.
             *
             * @param[in] rule_map: mapping of priority bins to rule lists
             * @param[in] constraint_map: mapping of priority bins to
             * constraint lists
             * @param[in] default_rules: a list of default rules to use for all
             * unspecified bins
             * @param[in] default_constraints: a list of default constraints to
             * use for all unspecified bins
             * @param[in] expressions: a list of pointers to AST expressions
             * used by this rule set
             * @param[in] logger: pointer to a logger instance to be used by 
             * the module
             */
            RuleSet(
                std::map<int, RuleList> rule_map,
                std::map<int, ConstraintList> constraint_map,
                RuleList default_rules,
                ConstraintList default_constraints,
                std::vector<std::shared_ptr<RuleExpression>> expressions,
                Logger *logger
            );

            /**
             * Default destructor
             */
            ~RuleSet() = default;

            /**
             * Get the list of rules for the given priority bin. If not set of
             * rules is explicitly specified for this bin, the default set is
             * returned.
             *
             * @param[in] bin: priority bin
             *
             * @return: list of rules
             */
            RuleList get_rules(int bin);

            /**
             * Get the list of constraints for the given priority bin. If not
             * set of constraints is explicitly specified for this bin, the
             * default set is returned.
             *
             * @param[in] bin: priority bin
             *
             * @return: list of constraints
             */
            ConstraintList get_constraints(int bin);

            /**
             * Applies a set of rules and constraints to a queue for a given
             * priority bin.
             *
             * @param[in] bin: priority bin
             * @param[in] queue: an ASDP queue to which the rules/constraints
             * should be applied
             *
             * @return: a pair of values; the first entry indicates whether all
             * constraints were satisfied, and if true, the second entry
             * specifies the total utility adjustment to apply.
             */
            std::pair<bool, double> apply(int bin, AsdpList queue);


        private:

            /**
             * Stores a list of bin-specific rules
             */
            std::map<int, RuleList> _rule_map;

            /**
             * Stores a list of bin-specific constraints
             */
            std::map<int, ConstraintList> _constraint_map;

            /**
             * Stores a list of default rules
             */
            RuleList _default_rules;

            /**
             * Stores a list of default constraints
             */
            ConstraintList _default_constraints;

            /**
             * Stores a list of references to dynamically allocated expressions
             */
            std::vector<std::shared_ptr<RuleExpression>> _expressions;  // TODO: double check shared_ptr's get dynamically deallocated

            /**
             * Reference to the logger instance to be used by this module
             */
            Logger *_logger = nullptr;
    };


    /**
     * AST representation of a logical constant (e.g. TRUE or FALSE)
     */
    class LogicalConstant : public BoolValueExpression {


        public:

            /**
             * Constructs a logical constant with the given value
             *
             * @param[in] value: logical constant value
             */
            LogicalConstant(bool value);

            /**
             * Default virtual destructor
             */
            virtual ~LogicalConstant() = default;

            /**
             * @see BoolValueExpression::get_value
             */
            bool get_value(AsdpAssignments assignments, AsdpList asdps);


        private:

            /**
             * Stores the logical constant value
             */
            bool _value;


    };


    /**
     * AST representation of a constant numerical value (e.g. 1.0)
     */
    class ConstExpression : public ValueExpression {


        public:

            /**
             * Constructs a numeric constant with the given value
             *
             * @param[in] value: numeric constant value
             */
            ConstExpression(double value);

            /**
             * Default virtual destructor
             */
            virtual ~ConstExpression() = default;

            /**
             * @see ValueExpression::get_value
             */
            DpMetadataValue get_value(
                AsdpAssignments assignments,
                AsdpList asdps
            );


        private:

            /**
             * Stores the numeric constant value
             */
            DpMetadataValue _value;


    };


    /**
     * AST representation of a unary logical NOT expression
     */
    class LogicalNot : public BoolValueExpression {


        public:

            /**
             * Constructs a logical negation of the given expression
             *
             * @param[in] expr: Boolean-valued expression to negate
             */
            LogicalNot(BoolValueExpression *expr);

            /**
             * Default virtual destructor
             */
            virtual ~LogicalNot() = default;

            /**
             * @see BoolValueExpression::get_value
             */
            bool get_value(AsdpAssignments assignments, AsdpList asdps);

        private:

            /**
             * Pointer to the Boolean-valued expression to negate
             */
            BoolValueExpression *_expr;


    };


    /**
     * AST representation of a binary logical expression; e.g., (... AND ...)
     */
    class BinaryLogicalExpression : public BoolValueExpression {


        public:

            /**
             * Constructs a binary logical expression using the operation
             * between the two Boolean expressions.
             *
             * @param[in] op: Boolean binary operator (AND, OR)
             * @param[in] left_expr: left-hand expression of the operator
             * @param[in] right_expr: right-hand expression of the operator
             */
            BinaryLogicalExpression(
                std::string op,
                BoolValueExpression *left_expr,
                BoolValueExpression *right_expr
            );

            /**
             * Default virtual destructor
             */
            virtual ~BinaryLogicalExpression() = default;

            /**
             * @see BoolValueExpression::get_value
             */
            bool get_value(AsdpAssignments assignments, AsdpList asdps);


        private:

            /**
             * Stores the binary logical operator
             */
            std::string _op;

            /**
             * Pointer to the left-hand expression of the binary operator
             */
            BoolValueExpression *_left_expr;

            /**
             * Pointer to the right-hand expression of the binary operator
             */
            BoolValueExpression *_right_expr;


    };


    /**
     * AST representation of a comparator expression; e.g., (... >= ...)
     */
    class ComparatorExpression : public BoolValueExpression {


        public:

            /**
             * Constructs a comparator expression using the comparator between
             * the two numeric expressions.
             *
             * @param[in] comp: comparator (==, !=, >=, >, <=, <)
             * @param[in] left_expr: left-hand expression of the comparator
             * @param[in] right_expr: right-hand expression of the comparator
             */
            ComparatorExpression(
                std::string comp,
                ValueExpression *left_expr,
                ValueExpression *right_expr
            );

            /**
             * Default virtual destructor
             */
            virtual ~ComparatorExpression() = default;

            /**
             * @see BoolValueExpression::get_value
             */
            bool get_value(AsdpAssignments assignments, AsdpList asdps);

        private:

            /**
             * Stores the comparator
             */
            std::string _comp;

            /**
             * Pointer to the left-hand expression of the comparator
             */
            ValueExpression *_left_expr;

            /**
             * Pointer to the right-hand expression of the comparator
             */
            ValueExpression *_right_expr;


    };


    /**
     * AST representation of a string constant; e.g., "foo"
     */
    class StringConstant : public ValueExpression {


        public:

            /**
             * Constructs a string constant with the given value
             *
             * @param[in] value: string constant value
             */
            StringConstant(
                std::string value
            );

            /**
             * Default virtual destructor
             */
            virtual ~StringConstant() = default;

            /**
             * @see ValueExpression::get_value
             */
            DpMetadataValue get_value(
                AsdpAssignments assignments,
                AsdpList asdps
            );


        private:

            /**
             * Stores the string constant value
             */
            DpMetadataValue _value;


    };


    /**
     * AST representation of a unary minus expression; e.g., -(...)
     */
    class MinusExpression : public ValueExpression {


        public:

            /**
             * Constructs a numeric negation of the given expression
             *
             * @param[in] expr: numeric-valued expression to negate
             */
            MinusExpression(
                ValueExpression *expr
            );

            /**
             * Default virtual destructor
             */
            virtual ~MinusExpression() = default;

            /**
             * @see ValueExpression::get_value
             */
            DpMetadataValue get_value(
                AsdpAssignments assignments,
                AsdpList asdps
            );


        private:

            /**
             * Pointer to the numeric-valued expression to negate
             */
            ValueExpression *_expr;

    };


    /**
     * AST representation of a binary numerical expression; e.g., (... + ...)
     */
    class BinaryExpression : public ValueExpression {


        public:

            /**
             * Constructs a binary numeric expression using the operation
             * between the two numeric expressions.
             *
             * @param[in] op: numeric binary operator (*, +, -)
             * @param[in] left_expr: left-hand expression of the operator
             * @param[in] right_expr: right-hand expression of the operator
             */
            BinaryExpression(
                std::string op,
                ValueExpression *left_expr,
                ValueExpression *right_expr
            );

            /**
             * Default virtual destructor
             */
            virtual ~BinaryExpression() = default;

            /**
             * @see ValueExpression::get_value
             */
            DpMetadataValue get_value(
                AsdpAssignments assignments,
                AsdpList asdps
            );


        private:

            /**
             * Stores the binary operator
             */
            std::string _op;

            /**
             * Pointer to the left-hand expression of the binary operator
             */
            ValueExpression *_left_expr;

            /**
             * Pointer to the right-hand expression of the binary operator
             */
            ValueExpression *_right_expr;


    };


    /**
     * AST representation of a field access expression; e.g., x.foo
     */
    class Field : public ValueExpression {


        public:

            /**
             * Constructs a field access expression from a variable name and
             * field name.
             *
             * @param[in] var_name: variable name
             * @param[in] field_name: ASDP metadata field name
             */
            Field(
                std::string var_name,
                std::string field_name
            );

            /**
             * Default virtual destructor
             */
            virtual ~Field() = default;

            /**
             * @see ValueExpression::get_value
             */
            DpMetadataValue get_value(
                AsdpAssignments assignments,
                AsdpList asdps
            );


        private:

            /**
             * Stores the variable name
             */
            std::string _var_name;

            /**
             * Stores the ASDP metadata field name
             */
            std::string _field_name;


    };


    /**
     * AST representation of an existential expression; e.g., EXISTS x: (...)
     */
    class ExistentialExpression : public BoolValueExpression {


        public:

            /**
             * Constructs an existential expression from a variable name and
             * Boolean-valued expression.
             *
             * @param[in] variable: variable name
             * @param[in] expr: Boolean-valued existentially quantified
             * expression
             */
            ExistentialExpression(
                std::string variable,
                BoolValueExpression *expr
            );

            /**
             * Default virtual destructor
             */
            virtual ~ExistentialExpression() = default;

            /**
             * @see BoolValueExpression::get_value
             */
            bool get_value(AsdpAssignments assignments, AsdpList asdps);


        private:

            /**
             * Stores the variable name of the existential quantifier
             */
            std::string _var;

            /**
             * Pointer to the existential quantified Boolean expression
             */
            BoolValueExpression *_expr;


    };


    /**
     * Parse a rule set from a JSON configuration file.
     *
     * @param[in] config_file: configuration file path
     *
     * @return: the parsed rule set, or an empty rule set if the configuration
     * file string is empty
     */
    RuleSet parse_rule_config(std::string config_file, Logger *logger);


};


#endif
