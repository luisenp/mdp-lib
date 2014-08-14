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

using namespace mlcore;
using namespace mlsolvers;
using namespace std;

int main(int argc, char* args[])
{
    char* filename = args[1];
    Problem* problem = new RacetrackProblem(filename);

    problem->generateAll();

    cerr << problem->states().size() << endl;

    for (State* s : problem->states()) {
        cerr << s << endl;
        for (Action* a : problem->actions()) {
            cerr << " *** " << a << endl;
            if (!problem->applicable(s, a)) continue;
            list<Successor> sccrs = problem->transition(s,a);
            for (Successor su : sccrs) {
                cerr << " **************** " << su.su_prob << " ";
                cerr << su.su_state << endl;
            }
        }
    }


    LAOStarSolver lao(problem, 0.001);
    lao.solve(problem->initialState());

    int nsims = atoi(args[2]);
    for (int i = 0; i < nsims; i++) {
        cerr << " ********* Simulation Starts ********* " << endl;
        State* tmp = problem->initialState();
        cerr << tmp << " ";
        double cost = 0.0;
        while (!problem->goal(tmp)) {
            Action* a = tmp->bestAction();
            cost += problem->cost(tmp, a);
            tmp = randomSuccessor(problem, tmp, a);
            cerr << a << " " << " - Cost: " << cost << endl;
            cerr << tmp << " ";
        }
        cerr << endl;
    }

    delete problem;
}
