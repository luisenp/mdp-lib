#ifndef MDPLIB_PPDDLHEURISTIC_H
#define MDPLIB_PPDDLHEURISTIC_H

#include "mini-gpt/heuristics.h"

#include "../Heuristic.h"
#include "../State.h"

#include "PPDDLProblem.h"

namespace mlppddl
{

enum HeuristicType
{
    atomMin1Forward,
    atomMinMForward,
    atomMinMBackward,
    FF
};

class PPDDLHeuristic : public mlcore::Heuristic
{
private:

    heuristic_t* pHeuristic_;

public:
    PPDDLHeuristic(PPDDLProblem* problem, HeuristicType type, int m = 2);

    ~PPDDLHeuristic() { delete pHeuristic_; }

    virtual double cost(const mlcore::State* s);
};

} // namespace mlppddl

#endif // MDPLIB_PPDDLHEURISTIC_H
