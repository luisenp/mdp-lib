#include <cassert>
#include <ctime>

#include "../../../include/State.h"
#include "../../../include/solvers/VISolver.h"

#include "../../../include/domains/racetrack/RacetrackProblem.h"
#include "../../../include/domains/racetrack/RacetrackState.h"
#include "../../../include/domains/racetrack/RTrackDetHeuristic.h"

RTrackDetHeuristic::RTrackDetHeuristic(const char* filename)
{
    detProblem_ = new RacetrackProblem(filename);
    detProblem_->pSlip(0.00);
    detProblem_->pError(0.00);
    detProblem_->generateAll();
    mlsolvers::VISolver vi(detProblem_, 1000, 0.001);
    vi.solve();
}

double RTrackDetHeuristic::cost(const mlcore::State* s)
{
    const RacetrackState* rts = static_cast<const RacetrackState*>(s);
    RacetrackState* tmp = new RacetrackState(rts->x(),
                                             rts->y(),
                                             rts->vx(),
                                             rts->vy(),
                                             detProblem_);
    mlcore::StateSet::const_iterator it =
        detProblem_->states().find((mlcore::State *) tmp);
    assert(it != detProblem_->states().end());
    delete tmp;
    return (*it)->cost();
}
