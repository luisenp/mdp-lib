#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include "../include/solvers/solver.h"
#include "../include/solvers/VISolver.h"
#include "../include/solvers/LRTDPSolver.h"
#include "../include/solvers/UCTSolver.h"
#include "../include/solvers/LAOStarSolver.h"

#include "../include/util/general.h"
#include "../include/util/graph.h"

#include "../include/domains/racetrack/RacetrackProblem.h"
#include "../include/domains/racetrack/RacetrackState.h"
#include "../include/domains/racetrack/RacetrackAction.h"
#include "../include/domains/racetrack/RTrackDetHeuristic.h"

using namespace mlcore;
using namespace mlsolvers;
using namespace std;

int main(int argc, char* args[])
{
    Problem* problem = new RacetrackProblem(args[1]);
    ((RacetrackProblem*) problem)->setPError(0.10);
    ((RacetrackProblem*) problem)->setPSlip(0.20);
    ((RacetrackProblem*) problem)->setMDS(-1);
    problem->generateAll();

    Heuristic* heuristic = new RTrackDetHeuristic(args[1]);
    problem->setHeuristic(heuristic);

    cerr << problem->states().size() << " states" << endl;

    VISolver vi(problem, 1000000, 1.0e-6);
    vi.solve();

//    LAOStarSolver lao(problem, 0.001);
//    lao.solve(problem->initialState());
    cerr << "Estimated cost " << problem->initialState()->cost() << endl;

//    for (State* s : problem->states()) {
//        RacetrackState* rts = (RacetrackState*) s;
//        if (rts->x() != 1 || rts->y() != 1)
//            continue;
//        Action* a = s->bestAction();
//        cerr << s << " " << s->cost() << endl;
//        if (!problem->applicable(s, a))
//            continue;
//        list<Successor> sccrs = problem->transition(s, a);
//        for (Successor su : sccrs) {
//            cerr << " **** " << su.su_state << " " << su.su_prob;
//            cerr << " " << su.su_state->cost() << endl;
//        }
//    }

    int nsims = atoi(args[2]);
    int verbosity = 1;
    double expectedCost = 0.0;
    for (int i = 0; i < nsims; i++) {
        State* tmp = problem->initialState();
        if (verbosity > 100) {
            cerr << " ********* Simulation Starts ********* " << endl;
            cerr << tmp << " ";
        }
        while (!problem->goal(tmp)) {
            Action* a = tmp->bestAction();
            expectedCost += problem->cost(tmp, a);
            tmp = randomSuccessor(problem, tmp, a);
            if (verbosity > 100) {
                cerr << a << " " << endl;
                cerr << tmp << " ";
            }
        }
        if (verbosity > 100)
            cerr << endl;
    }

    cerr << "Avg. cost " << expectedCost / nsims << endl;

    delete problem;
    delete ((RTrackDetHeuristic*) heuristic);
}
