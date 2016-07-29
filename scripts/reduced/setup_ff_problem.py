#!/usr/bin/python

import argparse
import copy
import sys

from create_all_determinizations import parse_sexp

            
def main(argv):
  """
  This program reads a PPDDL problem file, possibly containing a domain
  description as well, and prints the problem description to a new file.
  """
  parser = argparse.ArgumentParser(
    description='Create all possible determinizations of this domain.')
  parser.add_argument('-p', '--problem_file', required=True)
  parser.add_argument('-o', '--output', required=True)    
  args = parser.parse_args()
  problem_file_name = args.problem_file
  output_file_name = args.output
  
  # Reading problem file.
  problem_str = ''
  try:
    with open(problem_file_name, 'r') as problem_file:
      for line in problem_file:
        if line.startswith(';;') or line.isspace():
          continue;
        problem_str += line
  except IOError:
    print "Could not read file:", domain_file_name
    sys.exit(-1)  
  
  # Parsing the problem tree.
  problem_tree = parse_sexp(problem_str)
  print problem_tree
  

if __name__ == "__main__":
  main(sys.argv[1:])