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
  domain_tree = ppddl_util.parse_sexp(domain_str)
  all_prob_effects_list = []
  
  # Getting all possible probabilistic effects.
  ppddl_util.get_all_probabilistic_effects(domain_tree, all_prob_effects_list)
  
  # Getting the most likely outcome determinization.
  mlo_determinization = []
  for probabilistic_effect_info in all_prob_effects_list:
    mlo_determinization.append(
      ppddl_util.get_mlo_determinization_effect(probabilistic_effect_info))
  
  # Writing the MLO determinization to a PPDDL file.
  determinization_ppddl_tree = copy.deepcopy(domain_tree)
  description_text = ""
  for effect_info in mlo_determinization:
    description_text += "%s %d\n" % (effect_info[0], effect_info[1][0])
  ppddl_util.determinize_tree(mlo_determinization, determinization_ppddl_tree)
  ppddl_util.clean_up_tree(determinization_ppddl_tree)
  f = open('%s_mlo_det.pddl' % (output_file_name), 'w')
  fd = open('%s_mlo_det.desc' % (output_file_name), 'w')
  f.write(ppddl_util.make_str(determinization_ppddl_tree[0]))
  fd.write(description_text)
  f.close()
  fd.close()
  
  print 'Created MLO determinization for %s' % domain_file_name

if __name__ == "__main__":
  main(sys.argv[1:])
  