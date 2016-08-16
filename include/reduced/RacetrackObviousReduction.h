#ifndef RACETRACKOBVIOUSREDUCTION_H
#define RACETRACKOBVIOUSREDUCTION_H

#include <vector>

#include "../domains/racetrack/RacetrackAction.h"
#include "../domains/racetrack/RacetrackProblem.h"
#include "../domains/racetrack/RacetrackState.h"

#include "ReducedTransition.h"

namespace mlreduced
{

// Note: This works with the non-flat representation of the transition function
class RacetrackObviousReduction : public ReducedTransition
{
private:
    RacetrackProblem* problem_;

public:
    RacetrackObviousReduction(RacetrackProblem* problem) : problem_(problem) { }

    virtual ~RacetrackObviousReduction() { }

    /**
     * Overrides method from ReducedTransition.
     */
    virtual void setPrimary(mlcore::State* s,
                            mlcore::Action *a,
                            std::vector<bool>& primaryIndicators) const
    {
        if (s == problem_->initialState()) {
            for (std::pair<int,int> start : problem_->starts())
                primaryIndicators.push_back(true);
            return;
        }

        if (s == problem_->absorbing() || problem_->goal(s)) {
            primaryIndicators.push_back(true);
            return;
        }

        RacetrackState* rts = (RacetrackState *) s;
        RacetrackAction* rta = (RacetrackAction *) a;
        std::vector<std::vector <char> > & track = problem_->track();

        if (track[rts->x()][rts->y()] == rtrack::wall) {
            primaryIndicators.push_back(true);
            return;
        }

        int mds = problem_->mds();
        double pSlip = problem_->pSlip();
        double pError = problem_->pError();

        bool isDet = (abs(rts->vx()) + abs(rts->vy())) < mds;
        bool isErr = track[rts->x()][rts->y()] == rtrack::error;
        double p_err = isDet ? 0.0 : pError * (1 - pSlip);
        double p_slip = isDet ? 0.0 : pSlip;
        double p_int = isDet ? 1.0 : (1.0 - pSlip) * (1.0 - pError);

        if (!isErr) {
            p_int = 1.0 - p_slip;
            p_err = 0.0;
        }

        if (p_slip != 0.0)
            primaryIndicators.push_back(false);

        if (p_int != 0.0)
            primaryIndicators.push_back(true);

        if (p_err != 0.0) {
            int ta = abs(rta->ax()) + abs(rta->ay());
            int cnt = 4;
            if (ta == 1) cnt = 3;
            if (ta == 2) cnt = 2;
            for (int i = 0; i < cnt; i++)
                primaryIndicators.push_back(false);
        }
    }
};

}
#endif // RACETRACKOBVIOUSREDUCTION_H
