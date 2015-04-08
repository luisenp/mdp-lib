#include <cassert>
#include <ctime>

#include "../../../include/state.h"
#include "../../../include/solvers/VISolver.h"

#include "../../../include/lexi/domains/MORacetrackProblem.h"
#include "../../../include/lexi/domains/MORacetrackState.h"
#include "../../../include/lexi/domains/MORTrackDetHeuristic.h"

namespace mlmobj
{

MORTrackDetHeuristic::MORTrackDetHeuristic(char* filename, bool useSafety)
{
            mdplib_debug = true;
    detProblem_ = new MORacetrackProblem(filename, 1);
    detProblem_->setPSlip(0.00);
    detProblem_->setPError(0.00);
    detProblem_->useSafety(useSafety);
    detProblem_->generateAll();
    mlsolvers::VISolver vi(detProblem_, 1000, 0.001);
    vi.solve();
            mdplib_debug = true;
    dprint2("size deterministic ", detProblem_->states().size());
}

double MORTrackDetHeuristic::cost(const mlcore::State* s) const
{
    MORacetrackState* rts = (MORacetrackState*) s;
    MORacetrackState* tmp =
        new MORacetrackState(rts->x(), rts->y(), rts->vx(), rts->vy(), rts->safe(), detProblem_);

//    dprint3("       state in heuristic ", rts, ((void *) detProblem_));
    mlcore::StateSet::const_iterator it = detProblem_->states().find((mlcore::State *) tmp);
    assert(it != detProblem_->states().end());
    delete tmp;
    return (*it)->cost();
}

}
