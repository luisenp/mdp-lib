#!/usr/bin/python

import argparse
import copy
import sys

from fractions import Fraction


def parse_sexp(string):
  """
  Parses an S-Expression into a list-based tree.
  
  parse_sexp("(+ 5 (+ 3 5))") results in [['+', '5', ['+', '3', '5']]]  
  """
  sexp = [[]]
  word = ''
  in_str = False
  for c in string:
    if c == '(' and not in_str:
      sexp.append([])
    elif c == ')' and not in_str:
      if word:
        sexp[-1].append(word)
        word = ''
      temp = sexp.pop()
      sexp[-1].append(temp)
    elif c in (' ', '\n', '\t') and not in_str:
      if word != '':
        sexp[-1].append(word)
        word = ''
    elif c == '\"':
      in_str = not in_str
    else:
      word += c
  return sexp[0]


def make_str(ppddl_tree, level=0):
  """
  Creates a string representation of a PPDDL tree.
  """
  # Make sure the resulting string has the correct indentation.
  indent = (' ' * (2*level))
  ppddlstr = indent + '('
  indent += ' '
    
  # Appending subelements of the PPDDL tree.
  for i, element in enumerate(ppddl_tree):
    if isinstance(element, list):
      ppddlstr += '\n' + make_str(element, level + 1)
    else:
      if element.startswith(':') and i != 0:
        ppddlstr += '\n' + indent
      ppddlstr += element
      if i != len(ppddl_tree) - 1:
        ppddlstr += ' '
  ppddlstr += ') '
  return ppddlstr

  
def get_all_probabilistic_effects(ppddl_tree,
                                  all_prob_effects_list):
  """
  Adds all probabilistic effects in this PPDDL tree to the given list.
  The effects are added in pre-order traversal.
  We assume no nesting of probabilistic effect.
  """
  if not isinstance(ppddl_tree, list):
    return
  if ppddl_tree[0] == 'probabilistic':
    all_prob_effects_list.append(ppddl_tree)
  else:
    for element in ppddl_tree:
      get_all_probabilistic_effects(element, all_prob_effects_list)
  

def get_all_determinizations_effect(probabilistic_effect):
  """
  Generates all possible determinizations of the given probabilistic effect.
  """    
  all_determinizations = []
  total_explicit_prob = Fraction(0)
  for i in range(2, len(probabilistic_effect), 2):
    total_explicit_prob += Fraction(probabilistic_effect[i - 1])
    all_determinizations.append(probabilistic_effect[i])
  if total_explicit_prob != Fraction(1):
    all_determinizations.append([])    
  return all_determinizations
  

def get_all_determinizations_comb(determinizations_of_all_effects):
  """
  Generates all possible combinations of the given list with of probabilistic 
  effects determinizations.
  """
  # Base case for the recursion, only determinizations for one effect.
  all_determinizations = []
  if len(determinizations_of_all_effects) == 1:
    for determinization in determinizations_of_all_effects[0]:
      all_determinizations.append([determinization])
    return all_determinizations
  
  # We do this recursively by generating all combinations of the second to last 
  # effect, then generating all combinations of the first's element 
  # determinization with the resulting list.
  remaining_determinizations_comb = (
    get_all_determinizations_comb(determinizations_of_all_effects[1:]))
  
  for effect_determinization in determinizations_of_all_effects[0]:
    for remaining_effects_determinization in remaining_determinizations_comb:
      determinization = [effect_determinization]
      determinization.extend(remaining_effects_determinization)
      all_determinizations.append(determinization)
  return all_determinizations
      
      
def determinize_tree(determinization, ppddl_tree, index = 0):
  """
  Replaces all probabilistic effects with the given determinization.
  Each probabilistic effect is replaced by an element of determinization.
  The index in the determinization order corresponds to the pre-order
  index of the probabilistic effect in the PPDDL tree. The index parameter
  is passed and returned to keep track of this.
  """
  if not isinstance(ppddl_tree, list):
    return index
  if ppddl_tree[0] == 'probabilistic':
    ppddl_tree[:] = []
    ppddl_tree.extend(determinization[index])
    return index + 1
  else:
    for element in ppddl_tree:
      index = determinize_tree(determinization, element, index)
  return index
  
  
def main(argv):
  parser = argparse.ArgumentParser(
    description='Create all possible determinizations of this domain.')
  parser.add_argument('-d', '--domain', required=True)
  parser.add_argument('-o', '--output', required=True)    
  args = parser.parse_args()
  domain_file_name = args.domain
  output_file_name = args.output
  
  # Reading domain file.
  domain_str = ''
  try:
    with open(domain_file_name, 'r') as domain_file:
      for line in domain_file:
        if line.startswith(';;'):
          continue;
        domain_str += line
  except IOError:
    print "Could not read file:", domain_file_name
    sys.exit(-1)  
  
  # Parsing the domain tree.
  domain_tree = parse_sexp(domain_str)
  all_prob_effects_list = []
  get_all_probabilistic_effects(domain_tree, all_prob_effects_list)
  
  # Getting all possible determinizations.
  determinizations_of_all_effects = []
  for probabilistic_effect in all_prob_effects_list:
    determinizations_of_all_effects.append(
      get_all_determinizations_effect(probabilistic_effect))      
  all_determinizations = (
    get_all_determinizations_comb(determinizations_of_all_effects))
  
  # Writing all possible determinizations to a single PPDDL file each.
  idx = 0
  for determinization in all_determinizations:
    determinization_ppddl_tree = copy.deepcopy(domain_tree)
    determinize_tree(determinization, determinization_ppddl_tree)
    f = open('%s_det%d.pddl' % (output_file_name, idx), 'w')
    f.write(make_str(determinization_ppddl_tree))
    f.close()
    idx += 1
  

if __name__ == "__main__":
  main(sys.argv[1:])