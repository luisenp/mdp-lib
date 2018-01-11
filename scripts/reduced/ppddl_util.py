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
        if element == ':parameters' and ppddl_tree[i + 1] == []:
          ppddlstr += '()'
  ppddlstr += ') '
  return ppddlstr

  
def get_all_probabilistic_effects(ppddl_tree,
                                  all_prob_effects_list,
                                  action = None):
  """
  Adds all probabilistic effects in this PPDDL tree to the given list.
  The effects are added in pre-order traversal.
  We assume no nesting of probabilistic effect.
  
  A probabilistic effect is represented as a tuple 
    (action, ppddl_tree)    
  where "action" is the name of the action to which the effect belongs and
  "ppddl_tree" is the tree representation of the effect.
  """
  if not isinstance(ppddl_tree, list) or not ppddl_tree:
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
  
  A determinization is stored as a tuple (index, effect), where index
  represents the effect's order in the list of outcomes of the probabilistic
  effect, and effect represents the specific outcome. 
  
  The method returns a tuple (action_name, all_determinizations)
  where "action_name" is the name of the action to which the 
  probabilistic effect belongs, and "all_determinizations" is a list with 
  all the determinizations of the effect. 
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


def get_mlo_determinization_effect(probabilistic_effect_info):
  """
  Generates the most likely outcome determinization of the given probabilistic 
  effect.
  
  A determinization is stored as a tuple (index, effect), where index
  represents the effect's order in the list of outcomes of the probabilistic
  effect, and effect represents the specific outcome. 
  
  The method returns a tuple (action_name, mlo_determinization)
  where "action_name" is the name of the action to which the 
  probabilistic effect belongs, and "mlo_determinization" is the most likely
  effect. 
  """
  probability_mlo = Fraction(-1)
  total_non_explicit_prob = Fraction(1)
  idx = 0
  probabilistic_effect = probabilistic_effect_info[1]
  mlo_determinization = []
  for i in range(2, len(probabilistic_effect), 2):
    probability_effect = Fraction(probabilistic_effect[i - 1])
    total_non_explicit_prob -= probability_effect
    if Fraction(probabilistic_effect[i - 1]) > probability_mlo:
      probability_mlo = probability_effect
      mlo_determinization = (idx, probabilistic_effect[i])
      idx += 1
  if total_non_explicit_prob > probability_mlo:
    # No-op effect is the most likely outcome
    mlo_determinization = (idx, ['and'])
  return (probabilistic_effect_info[0], mlo_determinization)


def get_all_determinizations_comb(determinizations_of_all_effects):
  """
  Generates all possible combinations of the given list of lists of 
  probabilistic effects determinizations.
  
  Each element of the input variable is a tuple with all the determinizations of 
  a given probabilistic effect, as returned by function
  "get_all_determinizations_effect".
  
  The method returns a list with all the possible combinations of these
  determinizations, in the same order they are given. 
  
  So, for example, if the input is:
  
    ((action_0, (0, det_0_0), ..., (k_0, det_0_2)),
     (action_1, (0, det_1_0), ..., (k_0, det_1_1)))
     
  The method will return the following list of 6 combinations:
  
    ( ((action_0, (0, det_0_0)), (action_1, (0, det_1_0))),
      ((action_0, (0, det_0_0)), (action_1, (0, det_1_1))),
      ...,
      ((action_0, (2, det_0_2)), (action_1, (0, det_1_0))),
      ((action_0, (2, det_0_2)), (action_1, (0, det_1_1))) )
      
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
  
  Variable "determinization" is a list of determinizations, as created by 
  "get_all_determinizations_effect". 
  
  This function will visit the PPDDL tree in pre-order traversal and each time 
  it encounters a probabilistic effect, it will replace it with the effect at 
  "determinizaton[index][1][1]", then increment variable "index" and return its 
  new value. 
  
  Therefore, the user of this function must ensure that the each effect in 
  the given determinization corresponds to the proper probabilistic effect 
  in the PPDDL tree.
  """
  if not isinstance(ppddl_tree, list) or not ppddl_tree:
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
          