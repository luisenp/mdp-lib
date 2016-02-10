#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include "../include/solvers/Solver.h"
#include "../include/solvers/VISolver.h"
#include "../include/solvers/LRTDPSolver.h"
#include "../include/solvers/UCTSolver.h"

#include "../include/util/general.h"
#include "../include/util/graph.h"

#include "../include/domains/binarytree/BinaryTreeProblem.h"

using namespace std;
using namespace mlcore;
using namespace mlsolvers;

int main(int argc, char* args[])
{
    int levels = 4;
    if (argc > 2)
        levels = atoi(args[2]);

    Problem* problem = new BinaryTreeProblem(levels);
    problem->generateAll();

    int nrolls = 100;
    if (argc > 1)
        nrolls = atoi(args[1]);
    UCTSolver uct(problem, 0, nrolls, 100);
    Action* a = uct.solve(problem->initialState());
    StateActionDoubleMap& qvalues = uct.qvalues();
    StateActionIntMap& counterSA = uct.counterSA();

    for (pair <State*, ActionDoubleMap> entry : qvalues) {
        for (pair <Action*, double> entry2 : entry.second) {
            State* s = entry.first;
            Action* a = entry2.first;
            dprint2(s,a);
            dprint2("QVALUE ", qvalues[s][a]);
            dprint2("COUNTER ", counterSA[s][a]);
            dprint2("UCB1COST ", uct.ucb1Cost(s,a,qvalues[s][a]));
        }
    }

    delete ((BinaryTreeProblem *) problem);
    return 0;
}


