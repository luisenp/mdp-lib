#include "../../include/solvers/VISolver.h"

#include "../../include/reduced/BestDeterminizationReduction.h"


using namespace mlcore;
using namespace mlsolvers;
using namespace std;


namespace mlreduced
{

BestDeterminizationReduction::BestDeterminizationReduction(Problem* problem)
{
    vector< vector<int> > counts;
    VISolver viSolver(problem);
    viSolver.solve();
    for (State* s : problem->states()) {
        int actionIdx = 0;
        for (Action * a : problem->actions()) {
            if (counts.size() < problem->actions().size())
                counts.push_back(vector<int> (MAX_SUCCESSORS, 0));
            if (!problem->applicable(s, a)) {
                actionIdx++;
                continue;
            }
            double bestCost = mdplib::dead_end_cost;
            int successorIdx = 0;
            int indexBestSuccessor = -1;
            for (auto const & successor : problem->transition(s, a)) {
                if (successor.su_state->cost() < bestCost) {
                    indexBestSuccessor = successorIdx;
                }
                successorIdx++;
            }
            counts[actionIdx][indexBestSuccessor]++;
            dprint2(indexBestSuccessor, problem->cost(s, a));
            actionIdx++;
        }
    }

    for (int i = 0; i < counts.size(); i++) {
      for (int j = 0; j < 10; j++) {
          cerr << counts[i][j] << " ";
      }
      cout << endl;
    }
}

void
BestDeterminizationReduction::setPrimary(
    State* s, Action *a, vector<bool>& primaryIndicator) const
{

}

}
