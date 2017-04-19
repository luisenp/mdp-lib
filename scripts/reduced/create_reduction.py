#!/usr/bin/python

import argparse
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
    

def reduce_probabilistic_effect(probabilistic_effect):
  """
  Applies model reduction to a probabilistic effect. 
  """
  total_primary_prob = 0.0;
  primary_effects = []
  exceptions = []
  for i in range(2, len(probabilistic_effect), 2):
    effect = probabilistic_effect[i]
    probability = Fraction(probabilistic_effect[i - 1])
    if remove_primary_label(effect):    
      # Returns True if the effect was labeled primary.
      primary_effects.append((effect, probability))
      total_primary_prob += probability
    else:
      exceptions.append((effect, probability))
  
  # If this happens no reduction should be applied.
  if not primary_effects:
    return probabilistic_effect
  
  # Create the reduced effect.  
  probabilistic_effect[:] = []
  probabilistic_effect.append('and')
  probabilistic_effect.append(
    create_effect_before_exception_limit(primary_effects, exceptions))
  probabilistic_effect.append(
    create_effect_after_exception_limit(primary_effects, total_primary_prob))


def create_effect_before_exception_limit(primary_effects, exceptions):
  """ 
  Creates the reduced effect's component for when exceptions are allowed.
  """
  prob_effect_before = ['probabilistic']
  for effect in primary_effects:
    prob_effect_before.append(str(effect[1]))
    prob_effect_before.append(effect[0])
  for effect in exceptions:
    prob_effect_before.append(str(effect[1]))
    prob_effect_before.append(['and', ['k-1'], effect[0]])
    
  return ['when', ['k-0'], prob_effect_before]


def create_effect_after_exception_limit(primary_effects, total_primary_prob):
  """ 
  Creates the reduced effect's component for when exceptions are not allowed.
  """
  effect_no_exceptions = ['when', ['k-1']]
  normalized_primary_effects = ['probabilistic']
  for effect in primary_effects:
    normalized_primary_effects.append(str(effect[1] / total_primary_prob))
    normalized_primary_effects.append(effect[0])
  effect_no_exceptions.append(normalized_primary_effects)
  return effect_no_exceptions
  

def is_primary(effect):
  """
  Checks if the given effect is primary.
  """
  if effect == ['primary']:
    return True;
  if effect[0] != 'and':
    return False
  return ['primary'] in effect[1:]


def remove_primary_label(effect):
  """
  Removes the (primary) label from a PPDDL effect.
  
  Returns False if the effect was not tagged primary, otherwise returns True.
  """  
  if not is_primary(effect):
    return False
  if effect == ['primary']:
    effect[:] = ['k-0']
    return True
  for subeffect in effect[1:]:
    if subeffect != ['primary']:
      effect[:] = subeffect[:]
      break
  return True
  

def reduce_ppddl_tree(ppddl_tree):
  """
  Applies model reduction to all probabilistic effects in a PPDDL tree. 
  
  The method performs a depth-first search of the tree and applies the 
  reduction in post-order transversal. 
  
  Additionally, if the given tree represents the set of predicates, 
  it adds the (k-0) and (k-1) predicates.
  """
  if ppddl_tree[0] == ":predicates":
    ppddl_tree.append(['k-0'])
    ppddl_tree.append(['k-1'])
  
  for sub_tree in ppddl_tree:
    if isinstance(sub_tree, list):      
      reduce_ppddl_tree(sub_tree)
      if sub_tree[0] == 'probabilistic':
        reduce_probabilistic_effect(sub_tree)


def main(argv):
  parser = argparse.ArgumentParser(
    description='Create a reduced model of a PPDDL domain.')
  parser.add_argument('-d', '--domain', required=True)  
  args = parser.parse_args()
  domain_file_name = args.domain
  
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
  
  domain_tree = parse_sexp(domain_str)
  reduce_ppddl_tree(domain_tree)
  print make_str(domain_tree[0])
  

if __name__ == "__main__":
  main(sys.argv[1:])