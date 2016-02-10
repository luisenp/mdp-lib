#include <iostream>
#include <ctime>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include "../include/domains/racetrack/RacetrackAction.h"
#include "../include/mobj/domains/MORTrackDetHeuristic.h"
#include "../include/mobj/domains/MORacetrackProblem.h"
#include "../include/mobj/domains/MORacetrackState.h"
#include "../include/solvers/Solver.h"
#include "../include/solvers/mobj/CMDPSolver.h"
#include "../include/solvers/mobj/CMDPLagrangeSearchSolver.h"
#include "../include/solvers/mobj/LexiVISolver.h"
#include "../include/solvers/mobj/MOLAOStarSolver.h"
#include "../include/solvers/mobj/CMDPSlackSolver.h"
#include "../include/util/general.h"
#include "../include/util/graph.h"


using namespace mdplib_mobj_solvers;
using namespace mlcore;
using namespace mlmobj;
using namespace mlsolvers;
using namespace std;

int main(int argc, char* args[])
{
    if (argc < 5) {
        cout << "Usage ./textlexirace TRACK ALGORITHM SLACK SAFETY";
        cout << " --optional [NSIMS VERBOSITY GAMMA]" << endl;
        exit(0);
    }

    mdplib_debug = true;

    double slack = atof(args[3]);
    double gamma = argc > 7 ? atof(args[7]) : 0.95;
    bool useSafety = atoi(args[4]);
    int verbosity = argc > 6 ? atoi(args[6]) : 1;
    int size = 3;

    MOProblem* problem = new MORacetrackProblem(args[1], size);
    ((MORacetrackProblem*) problem)->setPError(0.00);
    ((MORacetrackProblem*) problem)->setPSlip(0.20);
    ((MORacetrackProblem*) problem)->setMDS(0);
    ((MORacetrackProblem*) problem)->useSafety(useSafety);
    problem->slack(slack);

    vector<Heuristic*> heuristics;
    Heuristic* heuristic =
        (strcmp(args[2], "vi") == 0 || strcmp(args[2], "lagrange") == 0) ?
            nullptr : new MORTrackDetHeuristic(args[1], useSafety);
    for (int i = 0; i < size; i++)
        heuristics.push_back(heuristic);
    problem->heuristics(heuristics);

    clock_t startTime = clock();
    double tol = 1.0e-6;
    CMDPSlackSolver css(problem, vector<double> (problem->size(), slack));
    if (strcmp(args[2], "css") == 0) {
        problem->gamma(gamma);
        css.solve(problem->initialState());
    } else if (strcmp(args[2], "lao") == 0) {
        MOLAOStarSolver lao(problem, tol, 1000000);
        lao.solve(problem->initialState());
    } else if (strcmp(args[2], "vi") == 0) {
        problem->gamma(gamma);
        problem->slack(slack * (1 - gamma));
        problem->generateAll();
        if (verbosity > 1)
            cout << problem->states().size() << " states" << endl;
        LexiVISolver vi(problem, 1000000000, tol);
        vi.solve();
    } else if (strcmp(args[2], "lagrange") == 0) {
        problem->gamma(gamma);
        problem->slack(0.0);
        problem->generateAll();
        CMDPLagrangeSearchSolver lag(problem);
        lag.solve(problem->initialState());
    }
    clock_t endTime = clock();
    if (verbosity > 10) {
        cout << "Estimated cost "
             << ((MOState *) problem->initialState())->mobjCost()[0] << " "
             << ((MOState *) problem->initialState())->mobjCost()[1] << endl;
        cout << startTime << " " << endTime << endl;
        cout << "Time " << ((endTime - startTime + 0.0) / CLOCKS_PER_SEC) << endl;
    } else if (verbosity > 1) {
        cout << ((MOState *) problem->initialState())->mobjCost()[0] << " "
             << ((MOState *) problem->initialState())->mobjCost()[1] << endl;
    }

    int nsims = argc > 5 ? atoi(args[5]) : 1;
    vector <double> expectedCost(2, 0.0);
    mdplib_debug = false;
    for (int i = 0; i < nsims; i++) {
        State* tmp = problem->initialState();
        if (verbosity > 100) {
            cout << " ********* Simulation Starts ********* " << endl;
        }

        double discount = 1.0;
        vector <double> cost(problem->size(), 0.0);
        while (!problem->goal(tmp)) {
            Action* a;
            if (strcmp(args[2], "css") == 0)
                a = css.policy()->getRandomAction(tmp);
            else
                a = tmp->bestAction();

            if (verbosity > 100) {
                MOState* lex = (MOState *) tmp;
                cout << endl << "STATE-ACTION *** " << tmp << " " << a << " est. cost " << endl;
                double c0 = problem->cost(lex,a,0), c1 = problem->cost(lex,a,1);
                cout << lex->mobjCost()[0] << " " <<  lex->mobjCost()[1];
                cout << " - acc. costs " << cost[0] << " " << cost[1] << endl;
                dsleep(250);
            }

            double discCost = discount * problem->cost(tmp, a, i);
            if (discCost < 1.0-6)
                break;

            for (int i = 0; i < problem->size(); i++) {
                cost[i] += discount * problem->cost(tmp, a, i);
                expectedCost[i] += discount * problem->cost(tmp, a, i);
            }
            tmp = randomSuccessor(problem, tmp, a);
            discount *= gamma;
        }
        if (verbosity > 100)
            cout << endl;
    }

    if (verbosity > 0) {
        cout << "Avg. Exec cost ";
        for (int i = 0; i < problem->size(); i++)
            cout << expectedCost[i] / nsims << " ";
        cout << endl;
    }

    delete problem;
    delete ((MORTrackDetHeuristic*) heuristic);
}

