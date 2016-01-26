#include "requirements.h"

Requirements::Requirements()
  : strips(true), typing(false), negative_preconditions(false),
    disjunctive_preconditions(false), equality(false),
    existential_preconditions(false), universal_preconditions(false),
    conditional_effects(false), fluents(false), probabilistic_effects(false),
    rewards(false)
{
}

void
Requirements::quantified_preconditions( void )
{
  existential_preconditions = true;
  universal_preconditions = true;
}

void
Requirements::adl( void )
{
  strips = true;
  typing = true;
  negative_preconditions = true;
  disjunctive_preconditions = true;
  equality = true;
  quantified_preconditions();
  conditional_effects = true;
}
