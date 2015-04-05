#ifndef MDPLIB_LEXISTATE_H
#define MDPLIB_LEXISTATE_H

#include <vector>

#include "../state.h"
#include "mobj_problem.h"

namespace mllexi {

class MOState : public mlcore::State
{

private:

    virtual std::ostream& print(std::ostream& os) const =0;

protected:

    std::vector<double> lexiCost_;

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
        lexiCost_[i] = c;
    }

    /**
     * An estimate of the expected cost of reaching the goal from this state w.r.t.
     * to all the value functions on the corresponding lexicographical problem.
     */
    std::vector<double> & lexiCost() { return lexiCost_; }

    /**
     * Resets the costs of the state to use the heuristics.
     * Each value in lexiCost_ is update according to the corresponding heuristic,
     * while cost_ is updated to be a linear combination of the heuristics
     * functions, using the weights passed as parameters.
     */
    void resetCost(std::vector<double>& weights, int keepIdx)
    {
        cost_ = 0;
        MOProblem* lp_ = (MOProblem*) problem_;
        lexiCost_ = std::vector<double> (lp_->size());
        for (int i = 0; i < lp_->size(); i++) {
            if (i == keepIdx) {
                cost_ += lexiCost_[i] * weights[i];
                continue;
            }

            if (lp_->heuristics().size() > i && lp_->heuristics()[i] != nullptr) {
                lexiCost_[i] = lp_->heuristics()[i]->cost(this);
                cost_ += lp_->heuristics()[i]->cost(this) * weights[i];
            }
            else {
                cost_ += 0.0;
                lexiCost_[i] = 0.0;
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

#endif // MDPLIB_LEXISTATE_H
