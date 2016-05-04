#ifndef REDUCEDHEURISTICWRAPPER_H
#define REDUCEDHEURISTICWRAPPER_H

#include "../Heuristic.h"
#include "../State.h"

#include "ReducedState.h"

namespace mlreduced
{

class ReducedHeuristicWrapper : public mlcore::Heuristic
{
private:
    mlcore::Heuristic* heuristic_;
public:
    ReducedHeuristicWrapper(mlcore::Heuristic* heuristic) :
        heuristic_(heuristic) { }

    virtual ~ReducedHeuristicWrapper() { }

    virtual double cost(const mlcore::State* s)
    {
        if (heuristic_ == nullptr)
            return 0.0;
        ReducedState* rs = (ReducedState *) s;
        return heuristic_->cost(rs->originalState());
    }
};

}

#endif // REDUCEDHEURISTICWRAPPER_H
