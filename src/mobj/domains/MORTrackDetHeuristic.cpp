#include <cassert>
#include <ctime>

#include "../../../include/State.h"
#include "../../../include/solvers/VISolver.h"

#include "../../../include/mobj/domains/MORacetrackProblem.h"
#include "../../../include/mobj/domains/MORacetrackState.h"
#include "../../../include/mobj/domains/MORTrackDetHeuristic.h"

namespace mlmobj
{

MORTrackDetHeuristic::MORTrackDetHeuristic(char* filename, bool useSafety)
{
    detProblem_ = new MORacetrackProblem(filename, 1);
    detProblem_->setPSlip(0.00);
    detProblem_->setPError(0.00);
    detProblem_->useSafety(useSafety);
    detProblem_->generateAll();
    mlsolvers::VISolver vi(detProblem_, 1000, 0.001);
    vi.solve();
}

double MORTrackDetHeuristic::cost(const mlcore::State* s)
{
    MORacetrackState* rts = (MORacetrackState*) s;
    MORacetrackState* tmp =
        new MORacetrackState(rts->x(), rts->y(), rts->vx(), rts->vy(), rts->safe(), detProblem_);
    mlcore::StateSet::const_iterator it = detProblem_->states().find((mlcore::State *) tmp);
    assert(it != detProblem_->states().end());
    delete tmp;
    return (*it)->cost();
}

}
