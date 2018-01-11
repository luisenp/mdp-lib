#!/usr/bin/python

import argparse
import copy
import sys

from ppddl_util import parse_sexp
from ppddl_util import make_str


def convert_list_to_single_line(list_str):
  single_line_block = ""
  j = 0
  cnt = 0
  previous_char_was_space = False
  while True:
    char_to_add = list_str[j]
    if list_str[j] == "(":
      cnt += 1
    elif list_str[j] == ")":
      cnt -= 1
    elif list_str[j] == "\n":
      char_to_add = " "
    if char_to_add != " " or not previous_char_was_space:
      single_line_block += char_to_add
    previous_char_was_space = (char_to_add == " ")        
    j += 1
    if cnt == 0:
      break
  return (single_line_block, j)


# Cleans up the PPDDL problem string by removing line breaks inside the init
# and goal lists. This also removes other types of goals that can't be read
# the FF planner (e.g., :goal-reward, :metric).
def clean_up_problem_str(problem_str):
  cleaned_up_str = ""
  i = 0;
  while i < len(problem_str):
    if problem_str.startswith("(:goal-reward", i):
      (str_to_add, size_str) = convert_list_to_single_line(problem_str[i:])
      i += size_str
    elif problem_str.startswith("(:metric", i):
      (str_to_add, size_str) = convert_list_to_single_line(problem_str[i:])
      i += size_str
    elif (problem_str.startswith("(:init",i) or 
          problem_str.startswith("(:goal", i)):
      (str_to_add, size_str) = convert_list_to_single_line(problem_str[i:])
      cleaned_up_str += str_to_add
      i += size_str
    else:
      cleaned_up_str += problem_str[i]
    i += 1
  return cleaned_up_str
    
            
def main(argv):
  """
  This program reads a PPDDL problem file, possibly containing a domain
  description as well, and prints the problem description to a new file.
  """
  parser = argparse.ArgumentParser(
    description="Create all possible determinizations of this domain.")
  parser.add_argument("-p", "--problem_file", required=True)
  parser.add_argument("-o", "--output", required=True)
  parser.add_argument("-n", "--problem_name", required=False)
  args = parser.parse_args()
  problem_file_name = args.problem_file
  output_file_name = args.output
  problem_name = args.problem_name
  
  # Reading problem file.
  ppddl_str = ""
  try:
    with open(problem_file_name, "r") as problem_file:
      for line in problem_file:
        if line.startswith(";;") or line.isspace():
          continue;
        ppddl_str += line
  except IOError:
    print "Could not read file:", problem_file_name
    sys.exit(-1)  
  
  # Parsing the problem tree.
  ppddl_tree = parse_sexp(ppddl_str)
  
  problem_str = None
  for sub_tree in ppddl_tree:
    if sub_tree[0] == "define" and sub_tree[1][0] == "problem":
      if problem_name is None or sub_tree[1][1] == problem_name:
        problem_str = make_str(sub_tree)
  if problem_str is None:
    print "Error: Problem name not found in the given file."
    sys.exit(0)
      
  cleaned_up_str = clean_up_problem_str(problem_str)
  
  f = open(output_file_name, 'w')
  f.write(cleaned_up_str)
  f.close()
  

if __name__ == "__main__":
  main(sys.argv[1:])