#ifndef RACETRACKOBVIOUSREDUCTION_H
#define RACETRACKOBVIOUSREDUCTION_H

#include <vector>

#include "../domains/racetrack/RacetrackAction.h"
#include "../domains/racetrack/RacetrackProblem.h"
#include "../domains/racetrack/RacetrackState.h"

#include "ReducedTransitionConfig.h"

namespace mlreduced
{

class RacetrackObviousReduction : public ReducedTransitionConfig
{
private:
    RacetrackProblem* problem_;
public:
    RacetrackObviousReduction(RacetrackProblem* problem) : problem_(problem) { }

    virtual ~RacetrackObviousReduction() { }

    virtual
    std::vector<bool> isPrimary(mlcore::State* s, mlcore::Action *a) const
    {
        std::vector<bool> primaryValues;
        if (s == problem_->initialState()) {
            for (std::pair<int,int> start : problem_->starts())
                primaryValues.push_back(true);
            return primaryValues;
        }

        if (s == problem_->absorbing() || problem_->goal(s)) {
            primaryValues.push_back(true);
            return primaryValues;
        }

        RacetrackState* rts = (RacetrackState *) s;
        RacetrackAction* rta = (RacetrackAction *) a;
        std::vector<std::vector <char> > & track = problem_->track();

        if (track[rts->x()][rts->y()] == rtrack::wall) {
            primaryValues.push_back(true);
            return primaryValues;
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
            primaryValues.push_back(false);

        if (p_int != 0.0)
            primaryValues.push_back(true);

        if (p_err != 0.0) {
            int ta = abs(rta->ax()) + abs(rta->ay());
            int cnt = 4;
            if (ta == 1) cnt = 3;
            if (ta == 2) cnt = 2;
            for (int i = 0; i < cnt; i++)
                primaryValues.push_back(false);
        }

        return primaryValues;
    }
};

}
#endif // RACETRACKOBVIOUSREDUCTION_H
