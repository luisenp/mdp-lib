#include <cassert>
#include <ctime>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <typeinfo>

#include "../../include/domains/racetrack/RacetrackProblem.h"
#include "../../include/domains/racetrack/RTrackDetHeuristic.h"
#include "../../include/domains/DummyState.h"

#include "../include/ppddl/mini-gpt/states.h"
#include "../include/ppddl/mini-gpt/problems.h"
#include "../include/ppddl/mini-gpt/domains.h"
#include "../include/ppddl/mini-gpt/states.h"
#include "../include/ppddl/mini-gpt/exceptions.h"
#include "../../include/ppddl/PPDDLHeuristic.h"
#include "../../include/ppddl/PPDDLProblem.h"

#include "../../include/solvers/RFFSolver.h"

#include "../../include/util/flags.h"
#include "../../include/util/general.h"

#include "../../include/Problem.h"


using namespace std;
using namespace mdplib;
using namespace mlppddl;
using namespace mlsolvers;


extern int yyparse();
extern FILE* yyin;
string current_file;
int warning_level = 0;

static int verbosity = 0;
static int k = 0;

mlcore::Problem* problem = nullptr;
mlcore::Heuristic* heuristic = nullptr;

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
    heuristic = new mlppddl::PPDDLHeuristic(static_cast<PPDDLProblem*>(problem),
                                            mlppddl::FF);
    problem->setHeuristic(heuristic);
}


/**
 * This program creates a reduced model for a given PPDDL file and solves
 * using the RFF algorithm.
 *
 * The program must receive the following command line arguments:
 *
 *    --det_problem: the path to a PDDL file describing the deterministic
 *        model to be passed to FF after k-exceptions.
 *
 * Other required command line arguments are:
 *    --problem: the PPDDL domain/problem to solve. The format is
 *        ppddlFilename:problemName
 *    --dir: the directory to use for FF. This directory must contain the file
 *        at "det_problem" as well as a file called "ff-template.pddl"
 *        containing only the PPDDL problem description (i.e., init and goal,
 *        not the domain).
 *        The init state must be described in a single line.
 *        The user of testRFF is responsible for ensuring consistency
 *        between ff-template.pddl and the file passed in the "problem" flag.
 */
int main(int argc, char* args[])
{
    register_flags(argc, args);

    if (flag_is_registered("debug"))
        mdplib_debug = true;

    // Reading flags.
    register_flags(argc, args);

    // The PPDDL problem to solve.
    assert(flag_is_registered_with_value("problem"));;
    string ppddlArgs = flag_value("problem");

    // The PDDL determinization to use.
    assert(flag_is_registered_with_value("det_problem"));
    string detProblem = flag_value("det_problem");

    //The directory where the determinization is stored.
    assert(flag_is_registered_with_value("dir"));
    string directory = flag_value("dir");

    // The verbosity level.
    if (flag_is_registered_with_value("v"))
        verbosity = stoi(flag_value("v"));

    // The number of simulations for the experiments.
    int nsims = 100;
    if (flag_is_registered_with_value("n"))
        nsims = stoi(flag_value("n"));

    initPPDDL(ppddlArgs);

    // Solving the problem using RFF.
    double totalPlanningTime = 0.0;
    clock_t startTime = clock();
    RFFSolver solver(static_cast<mlppddl::PPDDLProblem*> (problem),
              ffExec,
              directory + "/" + detProblem,
              directory + "/ff-template.pddl");
    solver.solve(problem->initialState());
    clock_t endTime = clock();
    totalPlanningTime += (double(endTime - startTime) / CLOCKS_PER_SEC);
    cout << "cost " << problem->initialState()->cost() <<
        " time " << totalPlanningTime << endl;


    // Running a trial of the continual planning approach.
    double expectedCost = 0.0;
    int countSuccess = 0;
    for (int i = 0; i < nsims; i++) {
        double cost = 0.0;
        mlcore::State* currentState = problem->initialState();
        mlcore::Action* action = currentState->bestAction();
        while (cost < mdplib::dead_end_cost) {
            cost += problem->cost(currentState, action);
            // The successor state according to the original transition model.
            mlcore::State* nextState =
                randomSuccessor(problem, currentState, action);
            if (problem->goal(nextState)) {
                countSuccess++;
                dprint2("GOAL!", countSuccess);
                break;
            }
            currentState = nextState;
            // Re-planning if needed (TODO, remove, RFF is offline).
            if (currentState->bestAction() == nullptr) {
                solver.solve(currentState);
            }
            if (currentState->deadEnd()) {
                cost = mdplib::dead_end_cost;
            }
            action = currentState->bestAction();
        }
        expectedCost += cost;
        if (verbosity > 10)
            cout << "sim " << i << ", successes " << countSuccess << endl;
    }
    cout << expectedCost / nsims << endl;
    cout << totalPlanningTime << endl;
    cout << countSuccess << endl;

    // Releasing memory
    delete problem;
    return 0;
}


