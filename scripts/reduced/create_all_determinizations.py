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
  if not ppddl_tree:
    return ''
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
                                  all_prob_effects_list,
                                  action = None):
  """
  Adds all probabilistic effects in this PPDDL tree to the given list.
  The effects are added in pre-order traversal.
  We assume no nesting of probabilistic effect.
  """
  if not isinstance(ppddl_tree, list):
    return
  if ppddl_tree[0] == ':action':
    action = ppddl_tree[1].rstrip('\r')
  if ppddl_tree[0] == 'probabilistic':
    all_prob_effects_list.append( (action, ppddl_tree) )
  else:
    for element in ppddl_tree:
      get_all_probabilistic_effects(element, all_prob_effects_list, action)
  

def get_all_determinizations_effect(probabilistic_effect_info):
  """
  Generates all possible determinizations of the given probabilistic effect.
  """    
  all_determinizations = []
  total_explicit_prob = Fraction(0)
  idx = 0
  probabilistic_effect = probabilistic_effect_info[1]
  for i in range(2, len(probabilistic_effect), 2):
    total_explicit_prob += Fraction(probabilistic_effect[i - 1])
    # The first element is the index of the deterministic effect, the second
    # is the effect itself.
    all_determinizations.append( (idx, probabilistic_effect[i]) )
    idx += 1
  if total_explicit_prob != Fraction(1):
    all_determinizations.append( (idx, ['and']) )  # No-op effect
  return (probabilistic_effect_info[0], all_determinizations)
  

def get_all_determinizations_comb(determinizations_of_all_effects):
  """
  Generates all possible combinations of the given list with of probabilistic 
  effects determinizations.
  """
  # Base case for the recursion, only determinizations for one effect.  
  all_determinizations = []
  if len(determinizations_of_all_effects) == 1:
    # Note that determinizations_of_all_effects[0] is a tuple:
    # (action_name, all_determinizations_of_the_actions_effect)
    for determinization in determinizations_of_all_effects[0][1]:
      all_determinizations.append([(determinizations_of_all_effects[0][0], 
                                    determinization)])
    return all_determinizations
  
  # We do this recursively by generating all combinations of effects from the 
  # second effect onwards, then generating all combinations of the first
  # effect's determinization with the resulting list.
  remaining_determinizations_comb = (
    get_all_determinizations_comb(determinizations_of_all_effects[1:]))
  
  for effect_determinization in determinizations_of_all_effects[0][1]:
    for remaining_effects_determinization in remaining_determinizations_comb:
      determinization = [(determinizations_of_all_effects[0][0], 
                          effect_determinization)]
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
    ppddl_tree.extend(determinization[index][1][1])
    return index + 1
  else:
    for element in ppddl_tree:
      index = determinize_tree(determinization, element, index)
  return index
  

def clean_up_tree(ppddl_tree):
  """
  Removes from the tree all effects that affect fluents (increase, decrease) 
  as well as void effects generated by the determinization.
  """
  # Ignore fluent-related effects and requirements list.  
  if not ppddl_tree:
    return
  if (ppddl_tree[0] == 'increase' or ppddl_tree[0] == 'decrease'
        or ppddl_tree[0] == ':requirements'):
    ppddl_tree[:] = []
    return
      
  for sub_tree in ppddl_tree:
    if isinstance(sub_tree, list):      
      clean_up_tree(sub_tree)
      # Cleaning up empty conjuctions.
      if sub_tree and sub_tree[0] == 'and' and len(sub_tree) > 1:
        valid = [x for x in sub_tree[1:] if x != ['and'] and x != []]
        if not valid:
          sub_tree[:] = []
          sub_tree.append('and')
          
            
def main(argv):
  """
  This program reads a PPDDL domain file and creates multiple files, each
  containing one of the possible determinizations of the actions in this
  problem.
  The files are generated in a format that the FF planner can read.
  """
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
        if line.startswith(';;') or line.isspace():
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
  for probabilistic_effect_info in all_prob_effects_list:
    determinizations_of_all_effects.append(
      get_all_determinizations_effect(probabilistic_effect_info))
  all_determinizations = (
    get_all_determinizations_comb(determinizations_of_all_effects))  
  
  # Writing all possible determinizations to a single PPDDL file each.
  idx = 0
  for determinization in all_determinizations:
    determinization_ppddl_tree = copy.deepcopy(domain_tree)
    description_text = ""
    for effect_info in determinization:
      description_text += "%s %d\n" % (effect_info[0], effect_info[1][0])
    determinize_tree(determinization, determinization_ppddl_tree)
    clean_up_tree(determinization_ppddl_tree)
    f = open('%s_det%d.pddl' % (output_file_name, idx), 'w')
    fd = open('%s_det%d.desc' % (output_file_name, idx), 'w')
    f.write(make_str(determinization_ppddl_tree[0]))
    fd.write(description_text)
    f.close()
    fd.close()
    idx += 1
  

if __name__ == "__main__":
  main(sys.argv[1:])