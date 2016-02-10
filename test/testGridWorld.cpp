#include <iostream>

#include "../include/solvers/Solver.h"
#include "../include/solvers/VISolver.h"
#include "../include/solvers/LRTDPSolver.h"
#include "../include/solvers/LAOStarSolver.h"

#include "../include/util/general.h"

#include "../include/domains/gridworld/GridWorldState.h"
#include "../include/domains/gridworld/GridWorldProblem.h"
#include "../include/domains/gridworld/GridWorldAction.h"
#include "../include/domains/gridworld/GWManhattanHeuristic.h"

using namespace std;
using namespace mlcore;
using namespace mlsolvers;

int main(int argc, char* args[])
{
    PairDoubleMap goals;
    goals.insert(make_pair(pair<int,int> (4, 4), 0.0));

    Problem* problem = new GridWorldProblem(5, 5, 0, 0, &goals, 1.0);
    GridWorldState* gws = (GridWorldState *) problem->initialState();
    Heuristic* heuristic = new GWManhattanHeuristic((GridWorldProblem*) problem);
    problem->setHeuristic(heuristic);

    problem->generateAll();
    StateSet states = problem->states();

    double tol = 1.0e-6;
    if (strcmp(args[1], "wlao") == 0) {
        LAOStarSolver wlao(problem, tol, 1000000, atof(args[2]));
        wlao.solve(problem->initialState());
    } else if (strcmp(args[1], "lao") == 0) {
        LAOStarSolver lao(problem, tol, 1000000);
        lao.solve(problem->initialState());
    } else if (strcmp(args[1], "lrtdp") == 0) {
        LRTDPSolver lrtdp(problem, 1000000000, tol);
        lrtdp.solve(problem->initialState());
    } else if (strcmp(args[1], "vi") == 0) {
        VISolver vi(problem, 1000000000, tol);
        vi.solve();
    }

    cout << problem->initialState()->cost() << endl;


    delete heuristic;
    delete problem;
}
