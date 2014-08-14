#include <cassert>

#include "../../../include/state.h"
#include "../../../include/solvers/VISolver.h"

#include "../../../include/domains/racetrack/RacetrackProblem.h"
#include "../../../include/domains/racetrack/RacetrackState.h"
#include "../../../include/domains/racetrack/RTrackDetHeuristic.h"

RTrackDetHeuristic::RTrackDetHeuristic(char* filename)
{
    detProblem_ = new RacetrackProblem(filename);
    detProblem_->setPSlip(0.0);
    detProblem_->setPError(0.0);
    detProblem_->generateAll();
    mlsolvers::VISolver vi(detProblem_, 1000, 0.01);
    vi.solve();
}

double RTrackDetHeuristic::cost(const mlcore::State* s) const
{
    RacetrackState* rts = (RacetrackState*) s;
    RacetrackState* tmp =
        new RacetrackState(rts->x(), rts->y(), rts->vx(), rts->vy(), detProblem_);
    mlcore::StateSet::const_iterator it = detProblem_->states().find((mlcore::State *) tmp);
    assert(it != detProblem_->states().end());
    return (*it)->cost();
}
