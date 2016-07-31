#!/usr/bin/python

import argparse
import copy
import sys

from create_all_determinizations import parse_sexp
from create_all_determinizations import make_str


def clean_up_problem_str(problem_str):
  cleaned_up_str = ""
  i = 0;
  while i < len(problem_str):
    cleaned_up_str += problem_str[i]
    if problem_str[i:i+6] == "(:init":
      cnt = 1
      j = i + 1 
      while cnt > -1:
        if problem_str[j] == ")":
          cleaned_up_str += problem_str[j]
          cnt -= 1
        elif problem_str[j] == "(":
          cleaned_up_str += problem_str[j]
          cnt += 1
        elif problem_str[j] == "\n":
          cleaned_up_str += " "
        else:
          cleaned_up_str += problem_str[j]
        j += 1
      i = j
    if problem_str[i:i+6] == "(:goal":
      cleaned_up_str += "(:goal"
      cnt = 0
      j = i + 6
      while cnt > 0:
        if problem_str[j] == "(:"
    i += 1
  print cleaned_up_str
    
            
def main(argv):
  """
  This program reads a PPDDL problem file, possibly containing a domain
  description as well, and prints the problem description to a new file.
  """
  parser = argparse.ArgumentParser(
    description="Create all possible determinizations of this domain.")
  parser.add_argument("-p", "--problem_file", required=True)
  parser.add_argument("-o", "--output", required=True)    
  args = parser.parse_args()
  problem_file_name = args.problem_file
  output_file_name = args.output
  
  # Reading problem file.
  ppddl_str = ""
  try:
    with open(problem_file_name, "r") as problem_file:
      for line in problem_file:
        if line.startswith(";;") or line.isspace():
          continue;
        ppddl_str += line
  except IOError:
    print "Could not read file:", domain_file_name
    sys.exit(-1)  
  
  # Parsing the problem tree.
  problem_tree = parse_sexp(ppddl_str)
  
  for sub_tree in problem_tree:
    if sub_tree[0] == "define" and sub_tree[1][0] == "problem":
      problem_str = make_str(sub_tree)
        
  clean_up_problem_str(problem_str)
  

if __name__ == "__main__":
  main(sys.argv[1:])