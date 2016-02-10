#include <iostream>
#include <mutex>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include "../include/solvers/Solver.h"
#include "../include/solvers/VISolver.h"
#include "../include/solvers/LRTDPSolver.h"
#include "../include/solvers/UCTSolver.h"
#include "../include/solvers/LAOStarSolver.h"

#include "../include/util/general.h"
#include "../include/util/graph.h"

#include "../include/domains/ctp/CTPProblem.h"
#include "../include/domains/ctp/CTPOptimisticHeuristic.h"

using namespace std;
using namespace mlcore;
using namespace mlsolvers;

int main(int argc, char* args[])
{
    Problem* problem = new CTPProblem(args[1]);
    Heuristic* heuristic = new CTPOptimisticHeuristic((CTPProblem *) problem);
    problem->setHeuristic(heuristic);

    LAOStarSolver lao(problem, 1.0e-4, 1000000, 5.0);
    lao.solve(problem->initialState());
    int nsim = 1000;
    int ngood = 0;
    double eCost = 0.0;
    for (int i = 0; i < nsim; i++) {
        State* tmp = problem->initialState();
        double costSim = 0.0;
        while (true) {
            if (problem->goal(tmp)) {
                CTPState* ctps = (CTPState*) tmp;
                if (!ctps->badWeather()) {
                    eCost  = eCost + costSim;
                    ngood++;
                }
                break;
            }
            Action* a = greedyAction(problem, tmp);
            costSim = costSim + problem->cost(tmp, a);
            tmp = randomSuccessor(problem, tmp, a);
        }
    }
    cout << "LAO* " << eCost / ngood << " " << ngood << endl;

    delete ((CTPProblem *) problem);
    delete ((CTPOptimisticHeuristic *) heuristic);
    return 0;
}

