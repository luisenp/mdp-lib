#include <sstream>
#include <typeinfo>

#include "../include/ppddl/mini-gpt/states.h"
#include "../include/ppddl/mini-gpt/problems.h"
#include "../include/ppddl/mini-gpt/domains.h"
#include "../include/ppddl/mini-gpt/states.h"
#include "../include/ppddl/mini-gpt/exceptions.h"

#include "../include/State.h"
#include "../include/ppddl/PPDDLProblem.h"
#include "../include/ppddl/PPDDLHeuristic.h"

#include "../include/solvers/Solver.h"
#include "../include/solvers/SoftFLARESSolver.h"
#include "../include/solvers/HMinHeuristic.h"
#include "../include/solvers/LRTDPSolver.h"
#include "../include/solvers/LAOStarSolver.h"
#include "../include/solvers/UCTSolver.h"

#include "../include/util/flags.h"

using namespace mdplib;
using namespace mlsolvers;
using namespace std;

extern int yyparse();
extern FILE* yyin;
string current_file;
int warning_level = 0;
int verbosity = 0;
bool using_soft_flares = false;

/* Parses the given file, and returns true on success. */
static bool read_file( const char* name )
{
    yyin = fopen( name, "r" );
    if( yyin == NULL ) {
        std::cout << "parser:" << name <<
            ": " << strerror( errno ) << std::endl;
        return( false );
    }
    else {
        current_file = name;
        bool success;
        try {
            success = (yyparse() == 0);
        }
        catch( Exception exception ) {
            fclose( yyin );
            std::cout << exception << std::endl;
            return( false );
        }
        fclose( yyin );
        return( success );
    }
}

bool mustReplan(Solver* solver,
                mlcore::State* current_state,
                mlcore::Action* current_action) {
    if (current_action == nullptr)
        return true;
    if (using_soft_flares) {
        return !static_cast<SoftFLARESSolver*>(solver)->
                    labeledSolved(current_state);
    }
}

int main(int argc, char *args[])
{
    std::string file;
    std::string prob;
    problem_t *problem = NULL;
    std::pair<state_t *,Rational> *initial = NULL;

    if (argc < 2) {
        std::cout << "Usage: testPPDDL [file] [problem]\n";
        return -1;
    }

    file = args[1];
    prob = args[2];

    if( !read_file( file.c_str() ) ) {
        std::cout <<
            "<main>: ERROR: couldn't read problem file `" << file << std::endl;
        return( -1 );
    }
    problem = (problem_t*) problem_t::find( prob.c_str() );
    if( !problem ) {
        std::cout << "<main>: ERROR: problem `" << prob <<
            "' is not defined in file '" << file << "'" << std::endl;
        return( -1 );
    }

    /* Initializing problem */
    mlppddl::PPDDLProblem* MLProblem = new mlppddl::PPDDLProblem(problem);
    mlppddl::PPDDLHeuristic* heuristic =
//        new mlppddl::PPDDLHeuristic(MLProblem, mlppddl::atomMin1Forward);
//        new mlppddl::PPDDLHeuristic(MLProblem, mlppddl::atomMinMForward);
        new mlppddl::PPDDLHeuristic(MLProblem, mlppddl::FF);
    MLProblem->setHeuristic(heuristic);

    cout << "HEURISTIC s0: " << MLProblem->initialState()->cost() << endl;

    int ntrials = 5000;
    if (argc > 3) {
        ntrials = atoi(args[3]);
    }

    cout << "INITIAL: " << MLProblem->initialState() << " ";
    Solver* solver;
    register_flags(argc, args);
    if (flag_is_registered("algorithm") &&
            flag_value("algorithm") == "soft-flares") {
        using_soft_flares = true;
        double depth = 4;
        double alpha = 0.10;
        double tol = 1.0e-3;
        int trials = 1000;
        if (flag_is_registered_with_value("depth"))
            depth = stoi(flag_value("depth"));
        TransitionModifierFunction mod_func = kLogistic;
        DistanceFunction dist_func = kStepDist;
        if (flag_is_registered_with_value("alpha"))
            alpha = stof(flag_value("alpha"));
        // Distance functions
        if (flag_is_registered("dist")) {
            string dist_str = flag_value("dist");
            if (dist_str == "traj") {
                dist_func = kTrajProb;
            } else if (dist_str == "plaus") {
                dist_func = kPlaus;
            } else if (dist_str == "depth") {
                dist_func = kStepDist;
            } else {
                cerr << "Error: unknown distance function." << endl;
                exit(0);
            }
        }
        // Labeling functions
        if (flag_is_registered("labelf")) {
            string labelf_str = flag_value("labelf");
            if (labelf_str == "exp") {
                mod_func = kExponential;
            } else if (labelf_str == "step") {
                mod_func = kStep;
            } else if (labelf_str == "linear") {
                mod_func = kLinear;
            } else if (labelf_str == "logistic") {
                mod_func = kLogistic;
            } else {
                cerr << "Error: unknown labeling function." << endl;
                exit(0);
            }
        }
        solver = new SoftFLARESSolver(
            MLProblem, trials, tol, depth, mod_func, dist_func, alpha);
        static_cast<SoftFLARESSolver*>(solver)->maxPlanningTime(1000);
    } else {
        solver = new LRTDPSolver(MLProblem, ntrials, 0.0001);
    }


    mdplib_debug = true;
    solver->solve(MLProblem->initialState());

    cout << MLProblem->initialState()->cost() << endl;


    int nsims = argc > 4 ? atoi(args[4]) : 1;
    int verbosity = argc > 5 ? atoi(args[5]) : 0;

    int totalSuccess = 0;
    double expectedCost = 0.0;
    for (int i = 0; i < nsims; i++) {
        mlcore::State* tmp = MLProblem->initialState();
        double cost = 0.0;
        while (true) {
            mlcore::Action* a = tmp->bestAction();

            if (verbosity > 100)
                cout << tmp << " " << tmp->cost() << endl;

            if (MLProblem->goal(tmp)) {
                expectedCost += cost;
                totalSuccess++;
                if (verbosity > 1) {
                    cout << "GOAL :-)"
                        << " Num. Successes " << totalSuccess << endl;
                }
                break;
            }
            if (mustReplan(solver, tmp, a)) {
                if (verbosity > 10)
                    cout << "REPLANNING..." << endl;
                solver->solve(tmp);
                a = tmp->bestAction();
                if (tmp->deadEnd() || a == nullptr) {
                    if (verbosity > 1)
                      cout << "DEAD END!! giving up :-( " << endl;
                    break;
                }
            }
            cost += MLProblem->cost(tmp, a);

            if (cost > mdplib::dead_end_cost
                    || tmp->cost() >= mdplib::dead_end_cost) {
                cout << "Too long... giving up " << endl;
                break;
            }

            if (verbosity > 100)
                cout << tmp->bestAction() << endl;
            tmp = mlsolvers::randomSuccessor(MLProblem, tmp, a);
        }
    }
    cout << "Expected Cost: " << expectedCost / totalSuccess << endl;
    cout << "Total Successes " << totalSuccess << "/" << nsims << endl;

    state_t::finalize();
    problem_t::unregister_use(problem);
    problem_t::clear();

    delete heuristic;
    delete solver;
}
