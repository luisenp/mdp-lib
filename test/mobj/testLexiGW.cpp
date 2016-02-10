#include <iostream>
#include <vector>

#include "../include/domains/gridworld/GridWorldAction.h"
#include "../include/domains/gridworld/GridWorldState.h"
#include "../include/mobj/domains/MOGridWorldProblem.h"
#include "../include/mobj/domains/MOGWManhattanHeuristic.h"
#include "../include/mobj/MObjProblem.h"
#include "../include/solvers/mobj/CMDPSolver.h"
#include "../include/solvers/mobj/CMDPSlackSolver.h"
#include "../include/solvers/mobj/LexiVISolver.h"
#include "../include/solvers/mobj/MOLAOStarSolver.h"
#include "../include/solvers/Solver.h"
#include "../include/util/general.h"

using namespace mdplib_mobj_solvers;
using namespace mlcore;
using namespace mlmobj;
using namespace std;

int main(int argc, char* args[])
{
    if (argc <= 3) {
        cerr << "Usage ./textlexigw N SLACK ALGORITHM --optional [NSIMS VERBOSITY]" << endl;
        exit(0);
    }

    mdplib_debug = true;

    int n = atoi(args[1]);
    double slack = atof(args[2]);
    double gamma = 0.9;
    vector<PairDoubleMap> goals(1);
    goals[0].insert(make_pair(pair<int,int> (n - 1, n - 1), 0.0));
//    goals[0].insert(make_pair(pair<int,int> (n - 1, n / 2), 0.0));
//    goals[0].insert(make_pair(pair<int,int> (1, n - 1), 0.0));

//    goals[0].insert(make_pair(pair<int,int> (n-1,n-1), 0.0));

//    MOGridWorldProblem* problem = new MOGridWorldProblem(n, n, n-1, n-1, goals, 2, 1.0);
    int problemSize = 1;
    double actionCost = 1.0;
    MOGridWorldProblem* problem =
        new MOGridWorldProblem(n, n, 0, 0, goals, problemSize, actionCost);
    problem->slack(slack);
    problem->gamma(gamma);

    vector<Heuristic*> heuristics;
    Heuristic* heur = new MOGWManhattanHeuristic(problem, 1.0);
    heuristics.push_back(heur); heuristics.push_back(heur);
    problem->heuristics(heuristics);
    GridWorldState* gws = (GridWorldState *) problem->initialState();

    MOLAOStarSolver lao(problem, 0.0001, 10000000L);
    LexiVISolver vi(problem);
    CMDPSlackSolver css(problem, vector<double> (10, slack));

    if (strcmp(args[3], "lao") == 0) {
        dprint2("SOLVING WITH LAO", problem->initialState());
        lao.solve(problem->initialState());
        dprint1("SOLVED!");
    } else if (strcmp(args[3], "css") == 0) {
        css.solve(problem->initialState());
    } else if (strcmp(args[3], "vi") == 0) {
        vi.solve();
    }
    cerr << "Estimated cost "
         << ((MOState *) problem->initialState())->mobjCost()[0] << " "
         << ((MOState *) problem->initialState())->mobjCost()[1] << endl;


    int nsims = argc > 4 ? atoi(args[4]) : 1;
    int verbosity = argc > 5 ? atoi(args[5]) : 1;
    vector <double> expectedCost(problem->size(), 0.0);
    for (int i = 0; i < nsims; i++) {
        State* tmp = problem->initialState();
        if (verbosity > 100) {
            cerr << " ********* Simulation Starts ********* " << endl;
        }

        double discount = 1.0;
        vector <double> cost(problem->size(), 0.0);
        while (!problem->goal(tmp)) {
            Action* a;
            a = tmp->bestAction();
            expectedCost[0] += problem->cost(tmp, a, 0);
            expectedCost[1] += problem->cost(tmp, a, 1);
            if (strcmp(args[3], "css") == 0)
                a = css.policy()->getRandomAction(tmp);
            else
                a = tmp->bestAction();

            if (verbosity > 100) {
                cerr << endl << "STATE-ACTION *** " << tmp << " " << a << " costs: ";
                for (int i = 0; i < problem->size(); i++)
                    cerr << problem->cost(tmp, a, i) << " ";
                cerr << endl;
            }

            for (int i = 0; i < problem->size(); i++) {
                cost[i] += discount * problem->cost(tmp, a, i);
                expectedCost[i] += discount * problem->cost(tmp, a, i);
            }
            tmp = mlsolvers::randomSuccessor(problem, tmp, a);
            discount *= gamma;
        }

        if (verbosity > 10) {
            cerr << endl << i << " GOAL **** " << tmp << " costs: ";
            for (int i = 0; i < problem->size(); i++)
                cerr << cost[i] << " ";
            cerr << endl;
        }

    }

    cerr << "Avg. Exec cost " << expectedCost[0] / nsims << " " << expectedCost[1] / nsims << endl;

    delete problem;
    delete heur;
}

