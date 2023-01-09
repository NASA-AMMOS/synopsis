#!/usr/bin/env python
import json
import argparse


from rule_parser import parse_file
from rule_ast import RuleJSONEncoder


def compile_rules(input_file, output_file):

    # Parse input
    ast = parse_file(input_file)

    # Save output
    with open(output_file, 'w+') as f:
        json.dump(ast, f, cls=RuleJSONEncoder, indent=2)


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument('input_file',
        help='Input SYNOPSIS Rules Definition (SRD) file'
    )
    parser.add_argument('output_file',
        help='Output JSON file'
    )

    args = parser.parse_args()
    compile_rules(**vars(args))


if __name__ == '__main__':
    main()
