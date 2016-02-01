#include <sstream>
#include <typeinfo>

#include "../include/ppddl/mini-gpt/states.h"
#include "../include/ppddl/mini-gpt/problems.h"
#include "../include/ppddl/mini-gpt/domains.h"
#include "../include/ppddl/mini-gpt/states.h"
#include "../include/ppddl/mini-gpt/exceptions.h"

#include "../include/state.h"
#include "../include/ppddl/ppddl_problem.h"

#include "../include/solvers/solver.h"
#include "../include/solvers/LRTDPSolver.h"
#include "../include/solvers/LAOStarSolver.h"
#include "../include/solvers/UCTSolver.h"

using namespace std;

extern int yyparse();
extern FILE* yyin;
string current_file;
int warning_level = 0;
int verbosity = 0;

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

int main(int argc, char **argv)
{
    std::string file;
    std::string prob;
    problem_t *problem = NULL;
    std::pair<state_t *,Rational> *initial = NULL;

    if (argc < 2) {
        std::cout << "Usage: testPPDDL [file] [problem]\n";
        return -1;
    }

    file = argv[1];
    prob = argv[2];

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
    mlppddl::Problem* MLProblem = new mlppddl::Problem(problem);

    int ntrials = 5000;
    if (argc > 3) {
        ntrials = atoi(argv[3]);
    }
    mlsolvers::LRTDPSolver solver(MLProblem, ntrials, 0.0001);
//    mlsolvers::LAOStarSolver solver(MLProblem);

    solver.solve(MLProblem->initialState());

    cerr << "MAIN: " << MLProblem->initialState() << " "
                << MLProblem->initialState()->cost() << endl;


    int nsims = argc > 4 ? atoi(argv[4]) : 1;
    int verbosity = argc > 5 ? atoi(argv[5]) : 0;

    int totalSuccess = 0;
    double expectedCost = 0.0;
    for (int i = 0; i < nsims; i++) {
        mlcore::State* tmp = MLProblem->initialState();
        double cost = 0.0;
        while (true) {
            mlcore::Action* a = tmp->bestAction();

            if (verbosity > 100)
                cerr << tmp << " " << tmp->cost() << endl;

            if (MLProblem->goal(tmp)) {
                if (verbosity > 1)
                    cerr << "GOAL :-)" << endl;
                expectedCost += cost;
                totalSuccess++;
                break;
            }
            if (a == nullptr) {
                if (verbosity > 1)
                    cerr << "REPLANNING..." << endl;
                solver.solve(tmp);
                a = tmp->bestAction();
                if (tmp->deadEnd() || a == nullptr) {
                    if (verbosity > 100)
                      cerr << "DEAD END!! giving up :-( " << endl;
                    break;
                }
            }
            cost += MLProblem->cost(tmp, a);

            if (cost > mdplib::dead_end_cost) {
                cerr << "Too long... giving up " << endl;
                break;
            }

            if (verbosity > 100)
                cerr << tmp->bestAction() << endl;
            tmp = mlsolvers::randomSuccessor(MLProblem, tmp, a);
        }
    }
    cerr << "Expected Cost: " << expectedCost / totalSuccess << endl;
    cerr << "Total Successes " << totalSuccess << "/" << nsims << endl;

    state_t::finalize();
    problem_t::unregister_use(problem);
    problem_t::clear();
}
