#include "GridWorldProblem.h"

GridWorldProblem::GridWorldProblem() : width_(0), height_(0), goals_(0) {}

GridWorldProblem::GridWorldProblem(int width, int height, IntPairSet * goals)
                                   : width_(width), height_(height), goals_(goals) { }

GridWorldProblem::~GridWorldProblem() { }

bool GridWorldProblem::goal(const State *s)
{
    return false;
}

std::list<Successor> GridWorldProblem::transition(State *s, Action *a)
{
    std::list<Successor> successors;
    return successors;
}

Rational GridWorldProblem::cost(State *s, Action *a)
{
    return Rational(0);
}

bool GridWorldProblem::applicable(State *s, Action *a)
{
    return false;
}
