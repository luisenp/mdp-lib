#include <iostream>
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

#include "../include/domains/sailing/SailingProblem.h"

using namespace mlcore;
using namespace mlsolvers;
using namespace std;

int main(int argc, char* args[])
{
    vector<double> costs;
    costs.push_back(1);
    costs.push_back(2);
    costs.push_back(5);
    costs.push_back(10);
    costs.push_back(mdplib::dead_end_cost + 1);

	double windTransition[] = {
        0.20, 0.20, 0.20, 0.00, 0.00, 0.00, 0.20, 0.20,
        0.20, 0.20, 0.20, 0.20, 0.00, 0.00, 0.00, 0.20,
        0.20, 0.20, 0.20, 0.20, 0.20, 0.00, 0.00, 0.00,
        0.00, 0.20, 0.20, 0.20, 0.20, 0.20, 0.00, 0.00,
        0.00, 0.00, 0.20, 0.20, 0.20, 0.20, 0.20, 0.00,
        0.00, 0.00, 0.00, 0.20, 0.20, 0.20, 0.20, 0.20,
        0.20, 0.00, 0.00, 0.00, 0.20, 0.20, 0.20, 0.20,
        0.20, 0.20, 0.00, 0.00, 0.00, 0.20, 0.20, 0.20};


    int size = atoi(args[1]);
    int goal = atoi(args[2]);
    Problem* problem =
        new SailingProblem(0, 0, 1, goal , goal, size, size, costs, windTransition);
    Heuristic* heuristic = new SailingNoWindHeuristic((SailingProblem*) problem);
    problem->setHeuristic(heuristic);

    problem->generateAll();

    cerr << problem->states().size() << " states" << endl;

    double tol = 1.0e-6;
    clock_t startTime = clock();
    if (strcmp(args[3], "wlao") == 0) {
        LAOStarSolver wlao(problem, tol, 1000000, atof(args[4]));
        wlao.solve(problem->initialState());
    } else if (strcmp(args[3], "lao") == 0) {
        LAOStarSolver lao(problem, tol, 1000000);
        lao.solve(problem->initialState());
    } else if (strcmp(args[3], "lrtdp") == 0) {
        LRTDPSolver lrtdp(problem, 1000000000, tol);
        lrtdp.solve(problem->initialState());
    } else if (strcmp(args[3], "vi") == 0) {
        VISolver vi(problem, 1000000000, tol);
        vi.solve();
        for (State* s : problem->states()) {
            if (s->cost() > heuristic->cost(s)) {
                cerr << "ERROR!!" << s << " " <<
                    s->cost() << " " << heuristic->cost(s) << endl;
            }
        }
    } else if (strcmp(args[3], "det") != 0) {
        cerr << "Unknown algorithm: " << args[3] << endl;
        return -1;
    }
    clock_t endTime = clock();
    double costTime = (double(endTime - startTime) / CLOCKS_PER_SEC);
    cerr << "Planning Time: " <<  costTime << endl;
    cerr << "Expected cost " << problem->initialState()->cost() << endl;

    double expectedCost = 0.0;
    int numSims = 250;
    for (int i = 0; i < numSims; i++) {
        State* tmp = problem->initialState();
        while (!problem->goal(tmp)) {
            Action* a = greedyAction(problem, tmp);
            expectedCost += problem->cost(tmp, a);
            tmp = randomSuccessor(problem, tmp, a);
        }
    }

    cerr << "Avg. Exec cost " << expectedCost / numSims << endl;
    cerr << "Avg.Total cost " << expectedCost / numSims + costTime << endl;


    delete problem;
}

