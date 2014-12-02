#include <iostream>
#include <vector>

#include "../include/solvers/solver.h"
#include "../include/solvers/VISolver.h"
#include "../include/solvers/LRTDPSolver.h"
#include "../include/solvers/LAOStarSolver.h"

#include "../include/util/general.h"

#include "../include/domains/gridworld/GridWorldState.h"
#include "../include/domains/gridworld/GridWorldAction.h"

#include "../include/lexi/lexi_problem.h"
#include "../include/lexi/domains/LexiGridWorldProblem.h"

using namespace std;
using namespace mlcore;
using namespace mllexi;
using namespace mlsolvers;

int main(int argc, char* args[])
{
    int n = 10;
    vector<PairDoubleMap> goals(2);
    goals[0].insert(make_pair(pair<int,int> (n - 1, n - 1), 0.0));
    goals[1].insert(make_pair(pair<int,int> (0, n - 1), 0.0));

    LexiProblem* problem = new LexiGridWorldProblem(n, n, 0, 0, goals, 1.0);
    GridWorldState* gws = (GridWorldState *) problem->initialState();

    list<State *> queue;
    queue.push_front(problem->initialState());
    while (!queue.empty()) {
        State* cur = queue.front();
        queue.pop_front();
        if (cur->checkBits(mdplib::VISITED))
            continue;
        cur->setBits(mdplib::VISITED);
        dprint1(cur);
        for (Action* a : problem->actions()) {
            if (!problem->applicable(cur, a))
                continue;
            std::list<Successor> successors = problem->transition(cur, a, 0);
            for (Successor sccr : successors) {
                queue.push_front(sccr.first);
            }
        }
    }

    delete problem;
}

