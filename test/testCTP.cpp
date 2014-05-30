#include <iostream>

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

    for (double x : dijkstra(g1, 0))
        cout << x << endl;

    Problem* problem = new CTPProblem(g1, probs, 0, 7);


    LRTDPSolver lrtdp(problem);
    lrtdp.solve(10, Rational(1,1000));

    std::cout << "LRTDP Estimates" << std::endl;
    for (State* s : problem->states())
        std::cout << s << " " << s->cost() << std::endl;
//
//    vector< vector <unsigned char> > tmp(2, vector <unsigned char> (2));
//    tmp[0][1] = 1; tmp[0][0] = 1;
//    vector< vector <unsigned char> > tmp2 = tmp;
//    tmp2[0][1] = 2;
//
//    cout << (int) tmp[0][0] << " " << (int) tmp2[0][0] << endl;
//    cout << (int) tmp[0][1] << " " << (int) tmp2[0][1] << endl;
//
//    cout << "COMP " << (tmp == tmp2) << endl;
//    tmp[0][1] = 2;
//    cout << "COMP " << (tmp == tmp2) << endl;
//
//    cout << problem->initialState() << endl;

    delete ((CTPProblem*) problem);

    return 0;
}

