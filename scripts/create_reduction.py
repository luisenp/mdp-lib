#!/usr/bin/python

import argparse, sys


def parse_sexp(string):
  """
  Code to parse an S-Expression. 
  
  >>> parse_sexp("(+ 5 (+ 3 5))")
  [['+', '5', ['+', '3', '5']]]
  
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


def make_str(ppddltree, level=0):
  """
  Creates a string representation of a PPDDL tree.
  """
  indent = (' ' * (2*level))
  ppddlstr = indent + '('
  for i, element in enumerate(ppddltree):
    if isinstance(element, list):
      ppddlstr += '\n' + make_str(element, level + 1)
    else:
      if element.startswith(':') and i != 0:
        ppddlstr += '\n' + indent
      ppddlstr += element
      if i != len(ppddltree) - 1:
        ppddlstr += ' '
  ppddlstr += ')' + ' '
  return ppddlstr
    

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
        if line.startswith(';;'):
          continue;
        domain_str += line
  except IOError:
    print "Could not read file:", domain_file_name
    sys.exit(-1)
    
  #print domain_str
  
  domain_tree = parse_sexp(domain_str)
  #print domain_sexp
  print make_str(domain_tree[0])
    

if __name__ == "__main__":
  main(sys.argv[1:])