#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include "../include/solvers/solver.h"
#include "../include/solvers/VISolver.h"
#include "../include/solvers/LRTDPSolver.h"
#include "../include/util/general.h"
#include "../include/util/graph.h"
#include "../include/util/rational.h"
#include "../include/domains/ctp/CTPProblem.h"


using namespace std;

int main(int argc, char* args[])
{
    int nvertices, nedges;
    ifstream myfile (args[1]);
    Graph* g;
    vector< vector<double> > probs;
    if (myfile.is_open()) {
        char x;
        string line;
        getline(myfile, line);
        istringstream iss(line);
        iss >> x >> nvertices >> nedges;
        g = new Graph(nvertices);
        for (int i = 0; i < nvertices; i++)
            probs.push_back(vector<double> (nvertices));
        while ( getline (myfile, line) ) {
            istringstream iss(line);
            int u, v;
            double p, w;
            iss >> x >> u >> v >> p >> w;
            probs[u - 1][v - 1] = p;
            g->connect(u - 1, v - 1, w);
            g->connect(v - 1, u - 1, w);
        }
        myfile.close();
    }

    Problem* problem = new CTPProblem(*g, probs, 0, nvertices - 1);


    LRTDPSolver lrtdp(problem);
    lrtdp.solve(100, Rational(1,1000));

    cout << "LRTDP Estimates" << endl;
    cout << problem->initialState()->cost() << endl;

    int nsim = 100;
    int ngood = 0;
    Rational eCost(0.0);
    for (int i = 0; i < nsim; i++) {
        State* tmp = problem->initialState();
        Rational costSim(0.0);
        while (true) {
            if (problem->goal(tmp)) {
                CTPState* ctps = (CTPState*) tmp;
                if (!ctps->badWeather()) {
                    eCost  = eCost + costSim;
                    ngood++;
                }
                break;
            }
            Action* a = tmp->bestAction();
            costSim = costSim + problem->cost(tmp, a);
            tmp = randomSuccessor(problem, tmp, a);
        }
    }

    cout << eCost.value() / ngood << " " << ngood << endl;

    delete ((CTPProblem*) problem);
    delete g;
    return 0;
}

