#ifndef MDPLIB_LEXISTATE_H
#define MDPLIB_LEXISTATE_H

#include <vector>

#include "../state.h"
#include "lexi_problem.h"

namespace mllexi {

class LexiState : public mlcore::State
{
private:

    virtual std::ostream& print(std::ostream& os) const =0;

protected:

    std::vector<double> lexiCost_;

public:

    LexiState() { }

    virtual ~LexiState() {}

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
