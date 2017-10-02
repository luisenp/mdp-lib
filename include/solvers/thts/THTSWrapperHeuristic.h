#ifndef MDPLIB_THTSWRAPPERHEURISTIC_H
#define MDPLIB_THTSWRAPPERHEURISTIC_H

#include "THTSHeuristic.h"

#include "../../Heuristic.h"

namespace mlsolvers {

// This class wraps a heuristic for states and converts it into a heuristic
// for THTS state-action pairs.
class THTSWrapperHeuristic : public THTSHeuristic {
private:
    // THe MDP problem for which this heuristic computes the estimate for.
    mlcore::Problem* problem_;

    // The heuristic to be wrapped.
    mlcore::Heuristic* heuristic_;
public:
    THTSWrapperHeuristic(mlcore::Problem* problem,
                         mlcore::Heuristic* heuristic)
        : problem_(problem), heuristic_(heuristic) {}

    virtual ~THTSWrapperHeuristic() {}

    // Overrides method in THTSHeuristic.
    virtual double value(mlcore::State* state, mlcore::Action* action) {
        if (heuristic_ == nullptr)
            return 0.0;
        double return_value = 0.0;
        for (auto & successor : problem_->transition(state, action)) {
            return_value += successor.su_prob
                * heuristic_->cost(successor.su_state);
        }
        return return_value;
    }
};

}

#endif // MDPLIB_THTSWRAPPERHEURISTIC_H
