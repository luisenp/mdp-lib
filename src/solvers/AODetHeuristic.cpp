#include <cassert>

#include "../../include/domains/AODeterminization.h"
#include "../../include/solvers/AODetHeuristic.h"
#include "../../include/solvers/VISolver.h"


using namespace mlcore;
using namespace std;

namespace mlsolvers
{

AODetHeuristic::AODetHeuristic(mlcore::Problem* problem) {
    AllOutcomesDeterminization* aodet = new AllOutcomesDeterminization(problem);
    VISolver solver(aodet);
    solver.solve(aodet->initialState());
    for (State* s : problem->states()) {
        costs_[s] = s->cost();
    }
}

double AODetHeuristic::cost(const State* s) {
    return costs_.at(const_cast<State*>(s));
}

} // namespace mlsovers
