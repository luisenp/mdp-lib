#ifndef CLIENT_H
#define CLIENT_H

#include "rational.h"
#include "strxml.h"
#include <iostream>

class Atom;
class Application;
class action_t;
class planner_t;
class problem_t;
class state_t;


/*******************************************************************************
 *
 * XML client
 *
 ******************************************************************************/

class XMLClient_t
{
  const problem_t *problem_;
  planner_t *planner_;
  int total_rounds_;
  int rounds_left_;
  float round_time_;
  int round_turns_;
  std::pair<state_t*,Rational> *display_;

public:
  XMLClient_t( planner_t *planner, const problem_t *problem,
	       std::string name, std::istream &is, std::ostream &os );
  ~XMLClient_t();

  const state_t* getState( XMLNodePtr stateNode );
  const Atom* getAtom( XMLNodePtr atomNode );
  const Application* getApplication( XMLNodePtr appNode );
  void sendDone( std::ostream& os ) const;
  void sendAction( std::ostream& os, const action_t *a ) const;
  int doRound( void );
};

#endif // CLIENT_H
