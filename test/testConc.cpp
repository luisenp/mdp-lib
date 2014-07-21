#include <thread>

#include "../include/util/general.h"
#include "../include/domains/gridworld/GridWorldState.h"
#include "../include/domains/gridworld/GridWorldProblem.h"
#include "../include/domains/gridworld/GridWorldAction.h"
#include "../include/domains/gridworld/GWManhattanHeuristic.h"

#include "../include/solvers/ConcurrentSolver.h"
#include "../include/solvers/LRTDPSolver.h"

using namespace std;
using namespace mlsolvers;
using namespace mlcore;

int main()
{
    int n = 50;
    PairDoubleMap goals;
    goals.insert(make_pair(pair<int,int> (n - 1,n - 1), -1.0));
    Problem* problem = new GridWorldProblem(n, n, 0, 0, &goals);
    GridWorldState* s0 = (GridWorldState *) problem->initialState();
    Heuristic* heuristic = new GWManhattanHeuristic((GridWorldProblem*) problem);

    LRTDPSolver lrtdp(problem, 100, 1.0e-1);
    ConcurrentSolver* solver = new ConcurrentSolver(lrtdp);
    solver->run(problem->initialState());

    cerr << problem->initialState()->cost() << endl;

    delete solver;
}
