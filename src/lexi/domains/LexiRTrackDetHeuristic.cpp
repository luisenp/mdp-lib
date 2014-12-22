#include <cassert>
#include <ctime>

#include "../../../include/state.h"
#include "../../../include/solvers/VISolver.h"

#include "../../../include/lexi/domains/LexiRacetrackProblem.h"
#include "../../../include/lexi/domains/LexiRacetrackState.h"
#include "../../../include/lexi/domains/LexiRTrackDetHeuristic.h"

namespace mllexi
{

LexiRTrackDetHeuristic::LexiRTrackDetHeuristic(char* filename)
{
    detProblem_ = new LexiRacetrackProblem(filename, 1);
    detProblem_->setPSlip(0.00);
    detProblem_->setPError(0.00);
    detProblem_->generateAll();
    mlsolvers::VISolver vi(detProblem_, 1000, 0.001);
    vi.solve();
}

double LexiRTrackDetHeuristic::cost(const mlcore::State* s) const
{
    LexiRacetrackState* rts = (LexiRacetrackState*) s;
    LexiRacetrackState* tmp =
        new LexiRacetrackState(rts->x(), rts->y(), rts->vx(), rts->vy(), detProblem_);
    mlcore::StateSet::const_iterator it = detProblem_->states().find((mlcore::State *) tmp);
    assert(it != detProblem_->states().end());
    delete tmp;
    return (*it)->cost();
}

}
