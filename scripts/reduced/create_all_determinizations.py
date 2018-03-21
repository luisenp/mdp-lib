#!/usr/bin/python

import argparse
import copy
import sys
import ppddl_util


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
  parser.add_argument("-n", "--domain_name", required=False)  
  args = parser.parse_args()
  domain_file_name = args.domain
  output_file_name = args.output
  domain_name = args.domain_name
  
  # Reading domain file.
  ppddl_str = ''
  try:
    with open(domain_file_name, 'r') as domain_file:
      for line in domain_file:
        if line.startswith(';;') or line.isspace():
          continue;
        ppddl_str += line
  except IOError:
    print "Could not read file:", domain_file_name
    sys.exit(-1)  
  
  # Parsing the PPDDL tree.
  ppddl_tree = ppddl_util.parse_sexp(ppddl_str)
    
  # Finding the specified domain
  domain_tree = None
  for sub_tree in ppddl_tree:
    if sub_tree[0] == "define" and sub_tree[1][0] == "domain":
      if domain_name is None or sub_tree[1][1] == domain_name:
        domain_tree = [sub_tree]
  if domain_tree is None:
    print "Error: Domain name not found in the given file."
    sys.exit(0)
  
    # Getting all possible probabilistic effects.
  all_prob_effects_list = []
  ppddl_util.get_all_probabilistic_effects(domain_tree, all_prob_effects_list)
  
  # Getting all possible determinizations.
  determinizations_of_all_effects = []
  for probabilistic_effect_info in all_prob_effects_list:
    determinizations_of_all_effects.append(
      ppddl_util.get_all_determinizations_effect(probabilistic_effect_info))
  all_determinizations = (
    ppddl_util.get_all_determinizations_comb(determinizations_of_all_effects))  
  
  # Writing all possible determinizations to a single PPDDL file each.
  idx = 0
  for determinization in all_determinizations:
    determinization_ppddl_tree = copy.deepcopy(domain_tree)
    description_text = ""
    for effect_info in determinization:
      description_text += "%s %d\n" % (effect_info[0], effect_info[1][0])
    ppddl_util.determinize_tree(determinization, determinization_ppddl_tree)
    ppddl_util.clean_up_tree(determinization_ppddl_tree)
    f = open('%s_det%d.pddl' % (output_file_name, idx), 'w')
    fd = open('%s_det%d.desc' % (output_file_name, idx), 'w')
    f.write(ppddl_util.make_str(determinization_ppddl_tree[0]))
    fd.write(description_text)
    f.close()
    fd.close()
    idx += 1
  

if __name__ == "__main__":
  main(sys.argv[1:])
  