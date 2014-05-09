#include <iostream>

#include "../include/util/general.h"
#include "../include/util/rational.h"
#include "../include/domains/gridworld/GridWorldState.h"
#include "../include/domains/gridworld/GridWorldProblem.h"
#include "../include/domains/gridworld/GridWorldAction.h"

using namespace std;

int main()
{
    IntPairSet goals;
    goals.insert(pair<int,int> (9,9));
    GridWorldProblem problem(10, 10, 0, 0, &goals);
    GridWorldState* gws = (GridWorldState *) problem.getInitialState();

    GridWorldAction *action = new GridWorldAction(gridworld::UP);
    list<Successor> sccrs = problem.transition(gws, action);

    for (list<Successor>::iterator it = sccrs.begin(); it != sccrs.end(); it++) {
        cout << it->first << " " << it->second << endl;
    }
    cout << gws << endl;
}
