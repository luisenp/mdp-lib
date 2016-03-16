#include <ctime>
#include <iostream>

#include "../include/solvers/EpicSolver.h"
#include "../include/solvers/LAOStarSolver.h"
#include "../include/solvers/LRTDPSolver.h"
#include "../include/solvers/Solver.h"
#include "../include/solvers/VISolver.h"

#include "../include/util/flags.h"
#include "../include/util/general.h"

#include "../include/domains/WrapperProblem.h"
#include "../include/domains/gridworld/GridWorldState.h"
#include "../include/domains/gridworld/GridWorldProblem.h"
#include "../include/domains/gridworld/GridWorldAction.h"
#include "../include/domains/gridworld/GWManhattanHeuristic.h"

using namespace std;
using namespace mdplib;
using namespace mlcore;
using namespace mlsolvers;

int main(int argc, char* args[])
{
    register_flags(argc, args);

    assert(flag_is_registered_with_value("problem"));

    PairDoubleMap goals;
    Problem* problem =
        new GridWorldProblem(flag_value("problem").c_str(), &goals, 1.0);

    GridWorldState* gws = (GridWorldState *) problem->initialState();
    Heuristic* heuristic =
        new GWManhattanHeuristic((GridWorldProblem*) problem);
    problem->setHeuristic(heuristic);

    assert(flag_is_registered_with_value("algo"));
    string algo = flag_value("algo");
    clock_t startTime = clock();
    double tol = 1.0e-6;
    if (algo == "wlao") {
        LAOStarSolver wlao(problem, tol, 1000000, atof(args[2]));
        wlao.solve(problem->initialState());
    } else if (algo == "lao") {
        LAOStarSolver lao(problem, tol, 1000000);
        lao.solve(problem->initialState());
    } else if (algo == "lrtdp") {
        LRTDPSolver lrtdp(problem, 1000000000, tol);
        lrtdp.solve(problem->initialState());
    } else if (algo == "vi") {
        problem->generateAll();
        VISolver vi(problem, 1000000000, tol);
        vi.solve();
    } else if (algo == "epic") {
      int horizon = 0;
      if (flag_is_registered_with_value("horizon"))
          horizon = stoi(flag_value("horizon"));
      EpicSolver epic(problem, horizon);
      epic.solve(problem->initialState());
    }
    clock_t endTime = clock();

    StateSet states = problem->states();
    cout << problem->states().size() << endl;
    cout << problem->initialState()->cost() << endl;
    cout << "solved in " << double(endTime - startTime) / CLOCKS_PER_SEC << endl;

    delete heuristic;
    delete problem;
}
