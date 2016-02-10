#ifndef MDPLIB_RAWFILEMOSTATE_H
#define MDPLIB_RAWFILEMOSTATE_H

#include <vector>

#include "../State.h"
#include "../Problem.h"

#include "../MObjProblem.h"
#include "../MObjState.h"

namespace mlmobj
{

class RawFileMOState : public MOState
{

private:

    int id_;

    std::vector<mlcore::SuccessorsList> allSuccessors_;
    std::vector< std::vector<double> > costs_;

    std::ostream& print(std::ostream& os) const;

public:
    RawFileMOState(int id, int nActions, int levels) : id_(id)
    {
        allSuccessors_ = std::vector<mlcore::SuccessorsList> (nActions);
        costs_ = std::vector< std::vector<double> > (levels, std::vector<double> (nActions));

        mobjCost_ = std::vector<double> (levels, 0.0);
    }

    virtual ~RawFileMOState() {}

    int id() { return id_; }

    /**
     * Adds the given state as a successor of this state with the given action and
     * probability.
     */
    void addSuccessor(int actionId, RawFileMOState* succState, double prob);

    /**
     * Returns the list of successors for the given action.
     */
    mlcore::SuccessorsList& getSuccessors(int actionId);

    /**
     * Sets the value of the cost corresponding to the given action at the given
     * lexicographic level.
     */
    void setCostAction(int actionId, int level, double cost);

    /**
     * Returns the cost of executing the given action at the given lexicographic
     * level.
     */
    double getCostAction(int actionId, int level);

    /**
     * Overrides method from State.
     */
    virtual mlcore::State& operator=(const mlcore::State& rhs);

    /**
     * Overrides method from State.
     */
    virtual bool operator==(const mlcore::State& rhs) const;

    /**
     * Overrides method from State.
     */
    virtual bool equals(mlcore::State* other) const;

    /**
     * Overrides method from State.
     */
    virtual int hashValue() const;

};

}

#endif // MDPLIB_RAWFILEMOSTATE_H
