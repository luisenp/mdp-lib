#!/usr/bin/python

import argparse
import functools
import sys


def main(argv):
  """
  This program receives the successes and cost obtained with 
  different determinizations on mdpsim, and prints the indices of 
  the n best results in a space separated list. 
  
  The arguments successes and costs are string containing a comma separated
  list of numbers. 
  """
  parser = argparse.ArgumentParser(
    description='Get the n best determinization results.')
  parser.add_argument('-s', '--successes', required=True)
  parser.add_argument('-c', '--costs', required=True)
  parser.add_argument('-n', '--n', required=True)
  parser.add_argument(
    '--debug', required=False, action='store_const', const=True)
  args = parser.parse_args()
  successes = args.successes
  costs = args.costs
  n = args.n
  
  success_array = [ int(x) for x in successes.split(',') ]
  costs_array = [ float(x) for x in costs.split(',') ]
  indices = range(len(success_array))
  n = min(int(n), len(success_array))
  
  def cmp(idx1, idx2):
    if success_array[idx1] == success_array[idx2]:
      return costs_array[idx1] - costs_array[idx2]
    return success_array[idx2] - success_array[idx1]
  
  indices.sort(key=functools.cmp_to_key(cmp))
    
  if args.debug:
    print n, success_array, costs_array
    print indices
    
  result=""
  for i in range(int(n)):
    result += "%s " % str(indices[i])
  print result[:-1]
  
if __name__ == "__main__":
  main(sys.argv[1:])