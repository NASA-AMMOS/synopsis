"""
Abstract Syntax Tree representation for SYNOPSIS rules
"""
from itertools import product
from json import JSONEncoder


class RuleJSONEncoder(JSONEncoder):

    def default(self, obj):
        if hasattr(obj, '__json__'):
            return obj.__json__()
        else:
            return JSONEncoder.default(self, obj)


class JSONEncodable:


    def _json(self):
        raise NotImplementedError()


    def __json__(self):
        return {
            '__type__': type(self).__name__,
            '__contents__': self._json(),
        }


class Rule(JSONEncodable):


    def __init__(self, variables, application, adjustment, max_applications):
        self.variables = tuple(variables)

        application.validate(self.variables)
        self.application = application

        adjustment.validate(self.variables)
        self.adjustment = adjustment

        self.max_applications = max_applications

        unused = (
            set(self.variables) -
            (application.exposed_variables() | adjustment.exposed_variables())
        )
        if len(unused) > 0:
            print(f'Warning: unused variables {unused}')


    def apply(self, asdps):

        total_adj_value = 0
        n_applications = 0

        for values in product(*(len(self.variables) * [asdps])):
            assignment = dict(zip(self.variables, values))
            assigned_ids = {
                k: v['id']
                for k, v in assignment.items()
            }

            if self.application.get_value(assignment, asdps):
                n_applications += 1
                adj_value = self.adjustment.get_value(assignment, asdps)
                total_adj_value += adj_value
                print(f'Applied rule with {assigned_ids}, adjustment = {adj_value}')

            if ((self.max_applications is not None) and
                (n_applications >= self.max_applications)):
                break

        return total_adj_value


    def __repr__(self):
        app_repr = repr(self.application)
        adj_repr = repr(self.adjustment)
        max_clause = (
            '' if self.max_applications is None
            else f', max_applications={self.max_applications}'
        )
        return f'RULE({self.variables}, {app_repr}, {adj_repr}{max_clause})'


    def _json(self):
        return {
            'variables': list(self.variables),
            'application': self.application,
            'adjustment': self.adjustment,
            'max_applications': self.max_applications,
        }


class Constraint(JSONEncodable):


    def __init__(self, variables, application, sum_field, constraint_value):
        self.variables = tuple(variables)

        application.validate(self.variables)
        self.application = application

        if sum_field is not None:
            sum_field.validate(self.variables)
        self.sum_field = sum_field

        self.constraint_value = constraint_value


    def apply(self, asdps):
        aggregate = 0

        for values in product(*(len(self.variables) * [asdps])):
            assignment = dict(zip(self.variables, values))

            if self.application.get_value(assignment, asdps):
                if self.sum_field is None:
                    value = 1
                else:
                    value = self.sum_field.get_value(assignment, asdps)
                aggregate += value

        return (aggregate < self.constraint_value)


    def __repr__(self):
        app_repr = repr(self.application)
        return f'RULE({self.variables}, {app_repr}, sum_field={self.sum_field}, constraint_value={self.constraint_value})'


    def _json(self):
        return {
            'variables': list(self.variables),
            'application': self.application,
            'sum_field': self.sum_field,
            'constraint_value': self.constraint_value,
        }


class ValueExpression:


    def get_value(self, assignments, asdps):
        raise NotImplementedError()


    def validate(self, scope):
        pass


    def exposed_variables(self):
        return set([])


class ExistentialExpression(ValueExpression, JSONEncodable):


    def __init__(self, variable, expression):
        self.variable = variable
        self.expression = expression


    def validate(self, scope):
        self.expression.validate(scope + (self.variable,))


    def exposed_variables(self):
        return (
            self.expression.exposed_variables() -
            set([self.variable])
        )


    def get_value(self, assignments, asdps):
        for a in asdps:
            new_assignemnts = dict(assignments)
            new_assignemnts[self.variable] = a
            try:
                value = self.expression.get_value(new_assignemnts, asdps)
            except KeyError:
                # If assignment invalid, skip candidate for existential qualifier
                continue
            if value: return True
        return False


    def __str__(self):
        return f'EXISTS {self.variable} : ({self.expression})'


    def __repr__(self):
        return f'ExistentialExpression({self.variable}, {repr(self.expression)})'


    def _json(self):
        return {
            'variable': self.variable,
            'expression': self.expression,
        }


class LogicalConstant(ValueExpression, JSONEncodable):


    def __init__(self, value):
        vtype = type(value)

        if vtype == str:
            if value.lower() == 'true':
                self.value = True
            elif value.lower() == 'false':
                self.value = False
            else:
                raise ValueError(f'Unexpected logical constant value "{value}"')

        elif vtype == bool:
            self.value = value

        else:
            raise ValueError(f'Unexpected value type "{vtype}"')


    def get_value(self, assignment, asdps):
        return self.value


    def __str__(self):
        return f'{self.value}'


    def __repr__(self):
        return f'LogicalConstant({self.value})'


    def _json(self):
        return {
            'value': self.value,
        }


class LogicalNot(ValueExpression, JSONEncodable):


    def __init__(self, expression):
        self.expression = expression


    def validate(self, scope):
        self.expression.validate(scope)


    def exposed_variables(self):
        return self.expression.exposed_variables()


    def get_value(self, assignment, asdps):
        return not self.expression.get_value(assignment, asdps)


    def __str__(self):
        return f'NOT {self.expression}'


    def __repr__(self):
        return f'LogicalNot({repr(self.expression)})'


    def _json(self):
        return {
            'expression': self.expression,
        }


class BinaryLogicalExpression(ValueExpression, JSONEncodable):


    def __init__(self, operator, left_expression, right_expression):
        self.operator = operator
        self.left_expression = left_expression
        self.right_expression = right_expression


    @staticmethod
    def evaluate(operator, left_value, right_value):
        if operator == 'AND':
            return (left_value and right_value)
        elif operator == 'OR':
            return (left_value or right_value)
        else:
            raise ValueError(f'Unknown logical operator "{operator}"')


    def get_value(self, assignment, asdps):
        return BinaryLogicalExpression.evaluate(
            self.operator,
            self.left_expression.get_value(assignment, asdps),
            self.right_expression.get_value(assignment, asdps),
        )


    def validate(self, scope):
        self.left_expression.validate(scope)
        self.right_expression.validate(scope)


    def exposed_variables(self):
        return (
            self.left_expression.exposed_variables() |
            self.right_expression.exposed_variables()
        )


    def __str__(self):
        return f'({self.left_expression} {self.operator} {self.right_expression})'


    def __repr__(self):
        lrepr = repr(self.left_expression)
        rrepr = repr(self.right_expression)
        return f'BinaryLogicalExpression({self.operator}, {lrepr}, {rrepr})'


    def _json(self):
        return {
            'operator': self.operator,
            'left_expression': self.left_expression,
            'right_expression': self.right_expression,
        }


class StringConstant(ValueExpression, JSONEncodable):


    def __init__(self, value):
        self.value = value


    def get_value(self, assignment, asdps):
        return self.value


    def __str__(self):
        return f'"{self.value}"'


    def __repr__(self):
        return f'StringConstant({self.value})'


    def _json(self):
        return {
            'value': self.value,
        }


class ComparatorExpression(ValueExpression, JSONEncodable):


    def __init__(self, comparator, left_expression, right_expression):
        self.comparator = comparator
        self.left_expression = left_expression
        self.right_expression = right_expression


    @staticmethod
    def evaluate(comparator, left_value, right_value):
        # TODO: Handle type mismatch
        if comparator == '<':
            return (left_value < right_value)
        elif comparator == '<=':
            return (left_value <= right_value)
        elif comparator == '>':
            return (left_value > right_value)
        elif comparator == '>=':
            return (left_value >= right_value)
        elif comparator == '==':
            return (left_value == right_value)
        elif comparator == '!=':
            return (left_value != right_value)
        else:
            raise ValueError(f'Unknown comparator "{comparator}"')


    def get_value(self, assignment, asdps):
        return ComparatorExpression.evaluate(
            self.comparator,
            self.left_expression.get_value(assignment, asdps),
            self.right_expression.get_value(assignment, asdps),
        )


    def validate(self, scope):
        self.left_expression.validate(scope)
        self.right_expression.validate(scope)


    def exposed_variables(self):
        return (
            self.left_expression.exposed_variables() |
            self.right_expression.exposed_variables()
        )


    def __str__(self):
        return f'({self.left_expression} {self.comparator} {self.right_expression})'


    def __repr__(self):
        lrepr = repr(self.left_expression)
        rrepr = repr(self.right_expression)
        return f'ComparatorExpression({self.comparator}, {lrepr}, {rrepr})'


    def _json(self):
        return {
            'comparator': self.comparator,
            'left_expression': self.left_expression,
            'right_expression': self.right_expression,
        }


class ArithmeticExpression(ValueExpression): pass


class ConstExpression(ArithmeticExpression, JSONEncodable):


    def __init__(self, value):
        self.value = float(value)


    def get_value(self, assignment, asdps):
        return self.value


    def __str__(self):
        return f'{self.value}'


    def __repr__(self):
        return f'ConstExpression({self.value})'


    def _json(self):
        return {
            '__type__': type(self).__name__,
            'value': self.value,
        }


class BinaryExpression(ArithmeticExpression, JSONEncodable):


    def __init__(self, operator, left_expression, right_expression):
        self.operator = operator
        self.left_expression = left_expression
        self.right_expression = right_expression


    def validate(self, scope):
        self.left_expression.validate(scope)
        self.right_expression.validate(scope)


    def exposed_variables(self):
        return (
            self.left_expression.exposed_variables() |
            self.right_expression.exposed_variables()
        )


    @staticmethod
    def evaluate(operator, left_value, right_value):
        if operator == '*':
            return (left_value * right_value)
        elif operator == '+':
            return (left_value + right_value)
        elif operator == '-':
            return (left_value - right_value)
        else:
            raise ValueError(f'Unknown operator "{operator}"')


    def get_value(self, assignment, asdps):
        return BinaryExpression.evaluate(
            self.operator,
            self.left_expression.get_value(assignment, asdps),
            self.right_expression.get_value(assignment, asdps),
        )


    def __str__(self):
        return f'({self.left_expression} {self.operator} {self.right_expression})'


    def __repr__(self):
        lrepr = repr(self.left_expression)
        rrepr = repr(self.right_expression)
        return f'BinaryExpression({self.operator}, {lrepr}, {rrepr})'


    def _json(self):
        return {
            'operator': self.operator,
            'left_expression': self.left_expression,
            'right_expression': self.right_expression,
        }


class MinusExpression(ArithmeticExpression, JSONEncodable):


    def __init__(self, expression):
        self.expression = expression


    def validate(self, scope):
        self.expression.validate(scope)


    def exposed_variables(self):
        return self.expression.exposed_variables()


    def get_value(self, assignment, asdps):
        return -self.expression.get_value(assignment, asdps)


    def __str__(self):
        return f'-{self.expression}'


    def __repr__(self):
        return f'MinusExpression({repr(self.expression)})'


    def _json(self):
        return {
            'expression': self.expression,
        }


class Field(ArithmeticExpression, JSONEncodable):


    def __init__(self, variable_name, field_name, lineno):
        self.variable_name = variable_name
        self.field_name = field_name
        self.lineno = lineno


    def get_value(self, assignment, asdps):
        return assignment[self.variable_name][self.field_name]


    def validate(self, scope):
        if self.variable_name not in scope:
            raise ValueError(f'Variable "{self.variable_name}" not in scope (line {self.lineno})')


    def exposed_variables(self):
        return set([self.variable_name])


    def __str__(self):
        return f'{self.variable_name}.{self.field_name}'


    def __repr__(self):
        return f'Field({self.variable_name}, {self.field_name}, {self.lineno})'


    def _json(self):
        return {
            'variable_name': self.variable_name,
            'field_name': self.field_name,
        }
