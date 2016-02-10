#ifndef MDPLIB_MOSTATE_H
#define MDPLIB_MOSTATE_H

#include <vector>

#include "../State.h"

#include "MObjProblem.h"

namespace mlmobj {

class MOState : public mlcore::State
{

private:

    virtual std::ostream& print(std::ostream& os) const =0;

protected:

    std::vector<double> mobjCost_;

public:

    using mlcore::State::cost;

    using mlcore::State::setCost;

    MOState() { }

    virtual ~MOState() {}

    /**
     * Updates the estimate of the expected cost to reach a goal from this state w.r.t.
     * to the i-th value function according to the lexicographical order of the problem.
     *
     * @param c The updated expected cost to reach a goal from this state.
     * @param i The index of the value function w.r.t. which the cost is to be updated
     */
    void setCost(double c, int i) {
        mobjCost_[i] = c;
    }

    /**
     * An estimate of the expected cost of reaching the goal from this state w.r.t.
     * to all the value functions on the corresponding lexicographical problem.
     */
    std::vector<double> & mobjCost() { return mobjCost_; }

    /**
     * Resets the costs of the state to use the heuristics.
     * Each value in mobjCost_ is update according to the corresponding heuristic,
     * while cost_ is updated to be a linear combination of the heuristics
     * functions, using the weights passed as parameters.
     */
    void resetCost(std::vector<double>& weights, int keepIdx)
    {
        MOProblem* mobjProblem_ = (MOProblem*) problem_;
        cost_ = 0;
        gValue_ = 0;
        hValue_ = 0;
        mobjCost_ = std::vector<double> (mobjProblem_->size(), 0.0);
        for (int i = 0; i < mobjProblem_->size(); i++) {
            if (i == keepIdx) {
                cost_ += mobjCost_[i] * weights[i];
                continue;
            }
            if (mobjProblem_->heuristics().size() > i &&
                    mobjProblem_->heuristics()[i] != nullptr) {
                mobjCost_[i] = mobjProblem_->heuristics()[i]->cost(this);
                cost_ += mobjProblem_->heuristics()[i]->cost(this) * weights[i];
                hValue_ += mobjProblem_->heuristics()[i]->cost(this) * weights[i];
            }
        }
    }

    /**
     * Overrides method from State.
     */
    virtual mlcore::State& operator=(const mlcore::State& rhs) =0;

    /**
     * Overrides method from State.
     */
    virtual bool operator==(const mlcore::State& rhs) const =0;

    /**
     * Overrides method from State.
     */
    virtual bool equals(mlcore::State* other) const =0;

    /**
     * Overrides method from State.
     */
    virtual int hashValue() const =0;
};

}

#endif // MDPLIB_MOSTATE_H
