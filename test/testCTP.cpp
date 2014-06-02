#include <iostream>
#include <unistd.h>

#include "../include/solvers/solver.h"
#include "../include/solvers/VISolver.h"
#include "../include/solvers/LRTDPSolver.h"
#include "../include/util/general.h"
#include "../include/util/graph.h"
#include "../include/util/rational.h"
#include "../include/domains/ctp/CTPProblem.h"


using namespace std;

int main()
{
    double graph1[8][8] = {{0.0, 2.0, 5.0, 4.0 , 0.0, 0.0, 0.0, 0.0},   //O
                            {2.0, 0.0, 2.0, 0.0, 7.0, 0.0, 12.0, 0.0},   //A
                            {5.0, 2.0, 0.0, 1.0, 4.0, 3.0, 0.0, 0.0},    //B
                            {4.0, 0.0, 1.0, 0.0, 0.0, 4.0, 0.0, 0.0},    //C
                            {0.0, 7.0, 4.0, 0.0, 0.0, 1.0, 0.0, 5.0},    //D
                            {0.0, 0.0, 3.0, 4.0, 1.0, 0.0, 0.0, 7.0},    //E
                            {0.0, 12.0, 0.0, 0.0, 0.0, 0.0, 0.0, 3.0},   //F
                            {0.0, 0.0, 0.0, 0.0, 5.0, 7.0, 3.0, 0.0}};   //T

    Graph g1(8);
    vector< vector<double> > probs;
    for (int i = 0; i < 8; i++) {
        probs.push_back(vector<double> (8));
        for (int j = 0; j < 8; j++) {
            if (graph1[i][j] != 0.0)
                g1.connect(i, j, graph1[i][j]);
            probs[i][j] = 0.5;
        }
    }

    Problem* problem = new CTPProblem(g1, probs, 0, 7);


    LRTDPSolver lrtdp(problem);
    lrtdp.solve(100, Rational(1,1000));

    std::cout << "LRTDP Estimates" << std::endl;
    for (State* s : problem->states()) {
        cout << s << " " << s->cost() << endl;
        if (s->bestAction() != nullptr)
            cout << s->bestAction() << std::endl;
    }
    delete ((CTPProblem*) problem);

    return 0;
}

