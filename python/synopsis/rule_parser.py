from ply.lex import lex
from ply.yacc import yacc

from rule_ast import *

reserved = (
    'BIN',
    'DEFAULT',
    'RULE',
    'CONSTRAINT',
    'APPLIES',
    'ADJUST',
    'UTILITY',
    'MAXIMUM',
    'APPLICATIONS',
    'AND',
    'OR',
    'NOT',
    'EXISTS',
    'COUNT',
    'SUM',
    'LESS',
    'THAN',
    'TRUE',
    'FALSE',
)

tokens = reserved + (
    # Literals and constants
    'ID', 'ICONST', 'FCONST', 'SCONST',

    # Comparators
    'LT', 'LE', 'GT', 'GE', 'EQ', 'NE',

    # Operators
    'PLUS', 'MINUS', 'TIMES',

    # Delimiters
    'LPAREN', 'RPAREN',
    'COMMA', 'PERIOD', 'SEMI', 'COLON',
)

# Comparators
t_LT = r'<'
t_GT = r'>'
t_LE = r'<='
t_GE = r'>='
t_EQ = r'=='
t_NE = r'!='

# Operators
t_PLUS = r'\+'
t_MINUS = r'-'
t_TIMES = r'\*'

# Delimeters
t_LPAREN = r'\('
t_RPAREN = r'\)'
t_COMMA = r','
t_PERIOD = r'\.'
t_SEMI = r';'
t_COLON = r':'

RESERVED_MAP = { r.lower() : r for r in reserved }

def t_ID(t):
    r'[A-Za-z_][\w_]*'
    t.type = RESERVED_MAP.get(t.value.lower(), "ID")
    return t

# Integer literal
t_ICONST = r'\d+?'

# Floating literal
t_FCONST = r'((\d+)(\.\d+)(e(\+|-)?(\d+))? | (\d+)e(\+|-)?(\d+))'

# String literal
t_SCONST = r'\"([^\\\n]|(\\.))*?\"'


def t_NEWLINE(t):
    r'\n+'
    t.lexer.lineno += t.value.count("\n")


def t_comment(t):
    r'\#(.)*?\n'
    t.lexer.lineno += 1


def t_error(t):
    print("Illegal character %s" % repr(t.value[0]))
    t.lexer.skip(1)


# Ignore tabs and spaces
t_ignore = ' \t'


precedence = (
    ('left', 'PLUS', 'MINUS'),
    ('left', 'TIMES'),
    ('right', 'UMINUS'),
    ('left', 'AND', 'OR'),
    ('right', 'NOT'),
)


def p_rule_file_bins(p):
    """
    rule_file : bin_list
    """
    p[0] = p[1]


def p_rule_file_no_bins(p):
    """
    rule_file : rule_constraint_list
    """
    p[0] = {
        'default': p[1]
    }


def p_bin_list(p):
    """
    bin_list : bin_definition
             | bin_list bin_definition
    """
    p[0] = p[1]
    if len(p) > 2:
        duplicate_keys = set(p[2].keys()) & set(p[1].keys())
        if len(duplicate_keys) > 0:
            raise ValueError(f'Duplicate bin definitions: {duplicate_keys}')
        p[0].update(p[2])


def p_bin_definition(p):
    """
    bin_definition : BIN ICONST COLON rule_constraint_list
                   | DEFAULT COLON rule_constraint_list
    """
    if len(p) == 4:
        p[0] = {
            'default': p[3]
        }
    else:
        p[0] = {
            int(p[2]): p[4]
        }


def p_rule_constraint_list(p):
    """
    rule_constraint_list : rule_constraint_list rule_declaration
                         | rule_constraint_list constraint_declaration
                         | rule_declaration
                         | constraint_declaration
    """
    if len(p) == 2:
        if type(p[1]) == Rule:
            p[0] = {
                'rules': [p[1]],
                'constraints': [],
            }
        else:
            p[0] = {
                'rules': [],
                'constraints': [p[1]],
            }
    else:
        p[0] = p[1]
        if type(p[2]) == Rule:
            p[0]['rules'].append(p[2])
        else:
            p[0]['constraints'].append(p[2])


def p_constraint_declaration(p):
    """
    constraint_declaration : CONSTRAINT variable_declaration COLON constraint_body SEMI
    """
    variable_list = p[2]
    constraint_kwargs = p[4]
    p[0] = Constraint(variable_list, **constraint_kwargs)


def p_constraint_body(p):
    """
    constraint_body : applies_clause aggregate_expression LESS THAN constant_expression
    """
    p[0] = {
        'application': p[1],
        'sum_field': p[2],
        'constraint_value': p[5],
    }


def p_aggregate_expression(p):
    """
    aggregate_expression : COUNT
                         | SUM field
    """
    if len(p) == 2:
        p[0] = None
    else:
        p[0] = p[2]


def p_constant_expression(p):
    """
    constant_expression : ICONST
                        | FCONST
                        | MINUS ICONST
                        | MINUS FCONST
    """
    if len(p) == 3:
        p[0] = -float(p[2])
    else:
        p[0] = float(p[1])


def p_rule_declaration(p):
    """
    rule_declaration : RULE variable_declaration COLON rule_body SEMI
    """
    variable_list = p[2]
    rule_kwargs = p[4]
    p[0] = Rule(variable_list, **rule_kwargs)


def p_variable_declaration(p):
    """
    variable_declaration : LPAREN variable_decl_list RPAREN
    """
    variable_list = p[2]

    # Check for duplicate variables
    if len(variable_list) != len(set(variable_list)):
        raise ValueError(
            f'Line {p.lineno(1)}: Duplicate variables in list: {variable_list}'
        )

    p[0] = variable_list


def p_variable_decl_list(p):
    """
    variable_decl_list : ID
                       | variable_decl_list COMMA ID
    """
    if len(p) == 2:
        p[0] = (p[1],)
    else:
        p[0] = p[1] + (p[3],)


def p_rule_body(p):
    """
    rule_body : applies_clause adjust_clause
              | applies_clause adjust_clause max_app_clause
    """
    if len(p) == 4:
        max_applications = p[3]
    else:
        max_applications = None

    p[0] = {
        'application': p[1],
        'adjustment': p[2],
        'max_applications': max_applications,
    }


def p_applies_clause(p):
    """
    applies_clause : APPLIES conditional_expression
    """
    p[0] = p[2]


def p_conditional_expression(p):
    """
    conditional_expression : comparator_expression
                           | logical_constant
                           | logical_expression
                           | existential_expression
    """
    p[0] = p[1]


def p_paren_conditional_expression(p):
    """
    conditional_expression : LPAREN conditional_expression RPAREN
    """
    p[0] = p[2]


def p_existential_expression(p):
    """
    existential_expression : EXISTS ID COLON LPAREN conditional_expression RPAREN
    """
    if p[2] not in p[5].exposed_variables():
        # Unused variable in existential expression
        p[0] = p[5]

    else:
        p[0] = ExistentialExpression(p[2], p[5])


def p_logical_expression(p):
    """
    logical_expression : conditional_expression AND conditional_expression
                       | conditional_expression OR conditional_expression
                       | NOT conditional_expression
    """
    if len(p) == 4:
        # Binary Operator
        if type(p[1]) == LogicalConstant and type(p[3]) == LogicalConstant:
            p[0] = LogicalConstant(BinaryLogicalExpression.evaluate(
                p[2],
                p[1].value,
                p[3].value,
            ))
        else:
            p[0] = BinaryLogicalExpression(p[2], p[1], p[3])

    else:
        # Unary Operator
        if type(p[2]) == LogicalConstant:
            p[0] = LogicalConstant(not p[2].value)
        else:
            p[0] = LogicalNot(p[2])


def p_logical_constant(p):
    """
    logical_constant : TRUE
                     | FALSE
    """
    p[0] = LogicalConstant(p[1])


def p_comparator(p):
    """
    comparator : LT
               | LE
               | GT
               | GE
               | EQ
               | NE
    """
    p[0] = p[1]


def p_value_expression(p):
    """
    value_expression : arithmetic_expression
                     | string_expression
    """
    p[0] = p[1]


def p_string_expression(p):
    """
    string_expression : SCONST
    """
    # Strip off quote marks
    p[0] = StringConstant(p[1][1:-1])


def p_comparator_expression(p):
    """
    comparator_expression : value_expression comparator value_expression
    """
    if ((type(p[1]) == ConstExpression and type(p[3]) == ConstExpression) or
        (type(p[1]) == StringConstant and type(p[3]) == StringConstant)):
        p[0] = LogicalConstant(ComparatorExpression.evaluate(
            p[2],
            p[1].value,
            p[3].value
        ))

    else:
        p[0] = ComparatorExpression(p[2], p[1], p[3])


def p_adjust_clause(p):
    """
    adjust_clause : ADJUST UTILITY arithmetic_expression
    """
    p[0] = p[3]


def p_max_app_clause(p):
    """
    max_app_clause : MAXIMUM APPLICATIONS ICONST
    """
    p[0] = int(p[3])


def p_arithmetic_expression(p):
    """
    arithmetic_expression : field
    """
    p[0] = p[1]


def p_unary_arithmetic_expression(p):
    """
    arithmetic_expression : MINUS arithmetic_expression %prec UMINUS
    """

    # If operand is constant, negate directly
    if type(p[2]) == ConstExpression:
        p[0] = ConstExpression(-p[2].value)

    # Otherwise, represent negation
    else:
        p[0] = MinusExpression(p[2])


def p_paren_arithmetic_expression(p):
    """
    arithmetic_expression : LPAREN arithmetic_expression RPAREN
    """
    p[0] = p[2]


def p_binary_arithmetic_expression(p):
    """
    arithmetic_expression : arithmetic_expression TIMES arithmetic_expression
                          | arithmetic_expression MINUS arithmetic_expression
                          | arithmetic_expression PLUS arithmetic_expression
    """

    # If both expressions are constant, evaluate operation
    if (type(p[1]) == ConstExpression) and (type(p[3]) == ConstExpression):
        p[0] = ConstExpression(BinaryExpression.evaluate(
            p[2],
            p[1].value,
            p[3].value,
        ))

    # Otherwise, represent binary operation
    else:
        p[0] = BinaryExpression(p[2], p[1], p[3])


def p_const_arithmetic_expression(p):
    """
    arithmetic_expression : ICONST
                          | FCONST
    """
    p[0] = ConstExpression(p[1])


def p_field(p):
    """
    field : ID PERIOD ID
    """
    p[0] = Field(p[1], p[3], p.slice[1].lineno)


SYNOPSIS_LEXER = lex()
SYNOPSIS_PARSER = yacc()


def parse_str(srd_str):
    return SYNOPSIS_PARSER.parse(srd_str)


def parse_file(filename):
    with open(filename, 'r') as f:
        return parse_str(f.read())


def get_grammar():
    lines = []
    for name, obj in globals().items():
        if name.startswith('p_') and 'p_error' not in name:
            lines.append(obj.__doc__.replace(' : ', ' = '))
    return '\n'.join(lines)
