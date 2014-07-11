/* -*-C++-*- */
/*
 * Actions.
 *
 * Copyright 2003-2005 Carnegie Mellon University and Rutgers University
 * Copyright 2007 Håkan Younes
 * Copyright 2014 Luis Pineda
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef ACTIONS_H
#define ACTIONS_H

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "config.h"
#include "effects.h"
#include "formulas.h"
#include "expressions.h"
#include "terms.h"

#include "../ppddl_successor.h"

/*
 * This class was modified to add an EXPAND method to obtain all successors of a state-action pair
 *
 * - Luis Pineda, July 2014
 */

namespace mlppddl
{
    class Node;
}

/* ====================================================================== */
/* ActionSchema */

struct Action;
struct ActionSet;

/*
 * Action schema.
 */
struct ActionSchema {
  /* Constructs an action schema with the given name. */
  explicit ActionSchema(const std::string& name);

  /* Deletes this action schema. */
  ~ActionSchema();

  /* Adds a parameter to this action schema. */
  void add_parameter(const Variable& parameter) {
    parameters_.push_back(parameter);
  }

  /* Sets the precondition of this action schema. */
  void set_precondition(const StateFormula& precondition);

  /* Sets the effect of this action schema. */
  void set_effect(const Effect& effect);

  /* Returns the name of this action schema. */
  const std::string& name() const { return name_; }

  /* Returns the parameters of this action schema. */
  const VariableList& parameters() const { return parameters_; }

  /* Returns the preconditions of this action schema. */
  const StateFormula& precondition() const { return *precondition_; }

  /* Returns the effect of this action schema. */
  const Effect& effect() const { return *effect_; }

  /* Fills the provided list with instantiations of this action
     schema. */
  void instantiations(ActionSet& actions, const TermTable& terms,
                      const AtomSet& atoms, const ValueMap& values) const;

  /* Returns an instantiation of this action schema. */
  const Action& instantiation(const SubstitutionMap& subst,
                              const TermTable& terms,
                              const AtomSet& atoms,
                              const ValueMap& values,
                              const StateFormula& precond) const;

 private:
  /* Action name. */
  std::string name_;
  /* Action parameters. */
  VariableList parameters_;
  /* Action precondition. */
  const StateFormula* precondition_;
  /* Action effect. */
  const Effect* effect_;
};

/* Output operator for action schemas. */
std::ostream& operator<<(std::ostream& os, const ActionSchema& a);


/* ====================================================================== */
/* ActionSchemaMap */

/*
 * Table of action schemas.
 */
struct ActionSchemaMap : public std::map<std::string, const ActionSchema*> {
};


/* ====================================================================== */
/* Action */

/*
 * A fully instantiated action.
 */
struct Action {
  /* Constructs an action with the given name. */
  explicit Action(const std::string& name);

  /* Deletes this action. */
  ~Action();

  /* Adds an argument to this action. */
  void add_argument(const Object& argument) { arguments_.push_back(argument); }

  /* Sets the precondition of this action. */
  void set_precondition(const StateFormula& precondition);

  /* Sets the effect of this action. */
  void set_effect(const Effect& effect);

  /* Returns the name of this action. */
  const std::string& name() const { return name_; }

  /* Returns the arguments of this action. */
  const ObjectList& arguments() const { return arguments_; }

  /* Returns the precondition of this action. */
  const StateFormula& precondition() const { return *precondition_; }

  /* Returns the effect of this action. */
  const Effect& effect() const { return *effect_; }

  /* Tests if this action is enabled in the given state. */
  bool enabled(const TermTable& terms,
               const AtomSet& atoms, const ValueMap& values) const;

  /* Changes the given state according to the effects of this action. */
  void affect(const TermTable& terms, AtomSet& atoms, ValueMap& values) const;

  /*
   * Returns a type Node object with information about all successor states of the action
   * - Luis Pineda
   */
  mlppddl::Node* expand(const TermTable& terms, AtomSet& atoms, ValueMap& values) const;

 private:
  /* Action name. */
  std::string name_;
  /* Action arguments. */
  ObjectList arguments_;
  /* Action precondition. */
  const StateFormula* precondition_;
  /* Action effect. */
  const Effect* effect_;
};

/* Output operator for actions. */
std::ostream& operator<<(std::ostream& os, const Action& a);

/*
 * Less than function object for action pointers.
 */
namespace std {
template<>
struct less<const Action*>
  : public binary_function<const Action*, const Action*, bool> {
  /* Comparison function operator. */
  bool operator()(const Action* a1, const Action* a2) const {
    if (a1->name() < a2->name()) {
      return true;
    } else if (a2->name() < a1->name()) {
      return false;
    } else {
      return a1->arguments() < a2->arguments();
    }
  }
};
}


/* ====================================================================== */
/* ActionSet */

/*
 * Set of actions.
 */
struct ActionSet : public std::set<const Action*> {
};


/* ====================================================================== */
/* ActionList */

/*
 * List of actions.
 */
struct ActionList : public std::vector<const Action*> {
};


#endif /* ACTIONS_H */
