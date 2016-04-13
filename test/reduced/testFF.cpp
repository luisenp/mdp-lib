#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <list>
#include <sstream>
#include <string>
#include <typeinfo>


#include "../include/State.h"

#include "../include/solvers/Solver.h"

#include "../include/ppddl/mini-gpt/states.h"
#include "../include/ppddl/mini-gpt/problems.h"
#include "../include/ppddl/mini-gpt/domains.h"
#include "../include/ppddl/mini-gpt/states.h"
#include "../include/ppddl/mini-gpt/exceptions.h"
#include "../../include/ppddl/PPDDLHeuristic.h"
#include "../../include/ppddl/PPDDLProblem.h"
#include "../../include/ppddl/PPDDLState.h"

#include "../../include/solvers/FFReducedModelSolver.h"

#include "../../include/util/flags.h"
#include "../../include/util/general.h"

#include "../../include/Heuristic.h"
#include "../../include/Problem.h"



using namespace std;
using namespace mdplib;
using namespace mlsolvers;
using namespace mlppddl;


extern int yyparse();
extern FILE* yyin;
string current_file;
int warning_level = 0;

static int verbosity = 0;
static int k = 0;

mlcore::Problem* problem = nullptr;
mlcore::Heuristic* heuristic = nullptr;

ifstream problemTemplateFile;
string templateFilename = "/home/lpineda/Desktop/problem_template.pddl";
string problemFilename = "/home/lpineda/Desktop/problem.pddl";

string ffExec = "/home/lpineda/Desktop/FF-v2.3/ff";
string ffDomain = "-o /home/lpineda/Desktop/domain.pddl";
string ffProblem = "-f /home/lpineda/Desktop/problem.pddl";
string ffCommand = ffExec + " " + ffDomain + " " + ffProblem;


/*
 * Parses the given PPDDL file, and returns true on success.
 */
static bool read_file( const char* ppddlFileName )
{
    yyin = fopen( ppddlFileName, "r" );
    if( yyin == NULL ) {
        cout << "parser:" << ppddlFileName <<
            ": " << strerror( errno ) << endl;
        return( false );
    }
    else {
        current_file = ppddlFileName;
        bool success;
        try {
            success = (yyparse() == 0);
        }
        catch( Exception exception ) {
            fclose( yyin );
            cout << exception << endl;
            return( false );
        }
        fclose( yyin );
        return( success );
    }
}


/*
 * Creates a PPDDL problem from a string describing the domain and problem file.
 */
bool initPPDDL(string ppddlArgs)
{
    size_t pos_equals = ppddlArgs.find(":");
    assert(pos_equals != string::npos);
    string file = ppddlArgs.substr(0, pos_equals);
    string prob =
        ppddlArgs.substr(pos_equals + 1, ppddlArgs.size() - pos_equals);

    pair<state_t *,Rational> *initial = nullptr;

    if( !read_file( file.c_str() ) ) {
        cerr << "<main>: ERROR: couldn't read problem file `" << file << endl;
        return false;
    }
    problem_t* internalPPDDLProblem =
        (problem_t *)(problem_t::find(prob.c_str()));
    if( !internalPPDDLProblem ) {
        cerr << "<main>: ERROR: problem `" << prob <<
            "' is not defined in file '" << file << "'" << endl;
        return false;
    }

    problem = new PPDDLProblem(internalPPDDLProblem);
    heuristic =
        new mlppddl::PPDDLHeuristic(static_cast<PPDDLProblem*>(problem),
                                     mlppddl::FF);
    problem->setHeuristic(heuristic);
}


int main(int argc, char* args[])
{
//    mdplib_debug = true;
//    register_flags(argc, args);
//
//    // The PPDDL problem to solve.
//    assert(flag_is_registered_with_value("problem"));;
//    string ppddlArgs = flag_value("problem");
//
//    // The PDDL determinization to use.
//    assert(flag_is_registered_with_value("det_problem"));
//    string detProblem = flag_value("det_problem");
//
//    //The directory where the determinization is stored.
//    assert(flag_is_registered_with_value("dir"));
//    string directory = flag_value("dir");
//
//    //The horizon to use.
//    assert(flag_is_registered_with_value("horizon"));
//    int horizon = stoi(flag_value("horizon"));
//
//    // The verbosity level.
//    if (flag_is_registered_with_value("v"))
//        verbosity = stoi(flag_value("v"));
//
//    // The number of simulations for the experiments.
//    int nsims = 100;
//    if (flag_is_registered_with_value("nsims"))
//        nsims = stoi(flag_value("nsims"));
//
//    initPPDDL(ppddlArgs);
//
//    FFReducedModelSolver ffSolver(problem,
//                                  ffExec,
//                                  directory + "/" + detProblem,
//                                  directory + "/p01.pddl",
//                                  horizon);
//
//    mlcore::StateActionMap stateActions;
//
//    double expectedCost = 0.0;
//    int countSuccesses = 0;
//    mdplib_debug = true;
//    for (int i = 0; i < nsims; i++) {
//        double cost = 0.0;
//        mlcore::State* currentState =  problem->initialState();
//        int currentHorizon = horizon;
//        while (true) {
//            if (problem->goal(currentState)) {
//                                                                                    dprint1("GOAL!!!");
//                countSuccesses++;
//                break;
//            }
//            if (cost > mdplib::dead_end_cost) {
//                                                                                    dprint1("TOO LONG! :(");
//                break;
//            }
//            ffSolver.maxHorizon(currentHorizon);
//
//                                                                                    mdplib_debug = false;
//            mlcore::Action* action = ffSolver.solve(currentState);
//                                                                                    mdplib_debug = true;
//                                                                                    dprint1(currentState);
//                                                                                    if (action != nullptr)
//                                                                                        dprint1(action);
//                                                                                    dprint1("*********************");
//            if (action == nullptr) {
//                cost = mdplib::dead_end_cost;
//                                                                                    dprint1("DEAD-END!! #(");
//                break;
//            }
//            currentState = randomSuccessor(problem, currentState, action);
//            cost += problem->cost(currentState, action);
//
//            if (currentHorizon == 0) {
//                currentHorizon = horizon;
//            } else {
//                currentHorizon--;
//            }
//        }
//        expectedCost += cost;
//    }
//    cout << "Expected cost " << expectedCost / nsims << endl;
//    cout << "Count successes " << countSuccesses << endl;
//
//    delete problem;
//    return 0;
}
