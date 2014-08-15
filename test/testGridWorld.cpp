#include <iostream>

#include "../include/solvers/solver.h"
#include "../include/solvers/VISolver.h"
#include "../include/solvers/LRTDPSolver.h"
#include "../include/util/general.h"
#include "../include/domains/gridworld/GridWorldState.h"
#include "../include/domains/gridworld/GridWorldProblem.h"
#include "../include/domains/gridworld/GridWorldAction.h"
#include "../include/domains/gridworld/GWManhattanHeuristic.h"

using namespace std;
using namespace mlcore;
using namespace mlsolvers;

int main()
{
    PairDoubleMap goals;
    goals.insert(make_pair(pair<int,int> (99, 99), -1.0));

    Problem* problem = new GridWorldProblem(100, 100, 0, 0, &goals);
    GridWorldState* gws = (GridWorldState *) problem->initialState();
    Heuristic* heuristic = new GWManhattanHeuristic((GridWorldProblem*) problem);
    problem->setHeuristic(heuristic);

    problem->generateAll();
    problem->generateAll();
    StateSet states = problem->states();

    VISolver solver(problem, 100, 1.0e-3);
    solver.solve();

//    std::cout << "Value Iteration Estimates" << std::endl;
//    for (State* s : problem->states())
//        std::cout << s << " " << s->cost() << " " << heuristic->cost(s) << std::endl;

//    LRTDPSolver lrtdp(problem, 1000, 1.0e-3);
//    lrtdp.solve(problem->initialState());
//    std::cout << "LRTDP Estimates" << std::endl;
//    for (State* s : problem->states())
//        std::cout << s << " " << s->cost() << std::endl;

    delete heuristic;
    delete problem;
}
