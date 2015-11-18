#include <iostream>
#include <ctime>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include "../include/solvers/solver.h"
#include "../include/solvers/MetareasoningSolver.h"

#include "../include/util/general.h"
#include "../include/util/graph.h"

#include "../include/domains/racetrack/RacetrackProblem.h"
#include "../include/domains/racetrack/RacetrackState.h"
#include "../include/domains/racetrack/RacetrackAction.h"
#include "../include/domains/racetrack/RTrackDetHeuristic.h"
#include "../include/domains/racetrack/RTrackLowResHeuristic.h"

using namespace mlcore;
using namespace mlsolvers;
using namespace std;

// usage ./testmeta racetrack nsims verbosity use_metareasoning
int main(int argc, char* args[])
{
    Problem* problem = new RacetrackProblem(args[1]);
    ((RacetrackProblem*) problem)->setPError(0.10);
    ((RacetrackProblem*) problem)->setPSlip(0.20);
    ((RacetrackProblem*) problem)->setMDS(-1);
    problem->generateAll();

    Heuristic* heuristic = new RTrackDetHeuristic(args[1]);
    problem->setHeuristic(heuristic);

    cout << problem->states().size() << " states" << endl;

    MetareasoningSolver solver(problem);
    solver.useMetareasoning(atoi(args[4]));
    int nsims = atoi(args[2]);
    int verbosity = atoi(args[3]);
    double expectedCost = 0.0;
    for (int i = 0; i < nsims; i++) {
        if (verbosity > 1)
            cout << "sim " << i << endl;
        State* tmp = problem->initialState();
        if (verbosity > 100) {
            cout << " ********* Simulation Starts ********* " << endl;
            cout << tmp << " ";
        }
        while (!problem->goal(tmp)) {
            clock_t startTime = clock();
            Action* a = solver.solve(tmp);
            clock_t endTime = clock();
            if (verbosity > 100) {
                cout << (double(endTime - startTime) / CLOCKS_PER_SEC) << endl;
            }
            expectedCost += problem->cost(tmp, a);
            tmp = randomSuccessor(problem, tmp, a);
            if (verbosity > 100) {
                cout << a << " " << endl;
                cout << tmp << " ";
            }
        }
        if (verbosity > 100)
            cout << endl;
    }

    cout << "Avg. Exec cost " << expectedCost / nsims << endl;

    delete problem;
    delete ((RTrackDetHeuristic*) heuristic);
}

