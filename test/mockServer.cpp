#include <map>
#include <sstream>
#include <typeinfo>

#include "../include/ppddl/mini-gpt/domains.h"
#include "../include/ppddl/mini-gpt/exceptions.h"
#include "../include/ppddl/mini-gpt/formulas.h"
#include "../include/ppddl/mini-gpt/problems.h"
#include "../include/ppddl/mini-gpt/states.h"

#include "../include/ppddl/PPDDLHeuristic.h"
#include "../include/ppddl/PPDDLProblem.h"
#include "../include/ppddl/PPDDLState.h"

#include "../include/solvers/Solver.h"
#include "../include/solvers/FLARESSolver.h"

#include "../include/State.h"


using namespace std;
using namespace mlsolvers;


extern int yyparse();
extern FILE* yyin;
string current_file;
int warning_level = 0;
int verbosity = 0;


mlppddl::PPDDLProblem* MLProblem;

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


/* Initializes the map from strings (describing atoms) to atoms ids. */
void initStringAtomMap(problem_t* problem,
                        unordered_map<string, ushort_t>& stringAtomMap)
{
    Domain dom = problem->domain();
    PredicateTable& preds = dom.predicates();
    TermTable& terms = problem->terms();
    for (auto const & atom : problem_t::atom_hash()) {
        ostringstream oss;
        atom.first->print(oss, preds, dom.functions(), terms);
        stringAtomMap[oss.str()] = atom.second;
    }
}


/* Simulates receiving an action from the client. */
mlcore::Action* simulateActionFromClient(mlcore::State* state, Solver* solver) {
    ostringstream oss;
    oss << state;
    // SEND OSS.STR() TO CLIENT
    // RECEIVE HERE FROM CLIENT
    return solver->solve(state);
}

int main(int argc, char **argv)
{
    mdplib_debug = true;

    std::string file;
    std::string prob;
    problem_t *problem = NULL;
    std::pair<state_t *,Rational> *initial = NULL;

    if (argc < 2) {
        std::cout << "Usage: testClient [file] [problem]\n";
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
    MLProblem = new mlppddl::PPDDLProblem(problem);
    mlppddl::PPDDLHeuristic* heuristic =
        new mlppddl::PPDDLHeuristic(MLProblem, mlppddl::FF);
    MLProblem->setHeuristic(heuristic);

    FLARESSolver solver(MLProblem, 1000, 1.0e-3, 1);

    /* Initializing a map from atom names to atom indices. */
    unordered_map<string, ushort_t> stringAtomMap;
    initStringAtomMap(problem, stringAtomMap);

    mlcore::State* currentState = MLProblem->initialState();
    while (!MLProblem->goal(currentState)) {
        if (currentState->deadEnd()) {
            cout << "DEAD-END" << endl;
            break;
        }
        mlcore::Action* action = simulateActionFromClient(currentState, &solver);
        cout << currentState << " " << action << endl;
        currentState = randomSuccessor(MLProblem, currentState, action);
    }

    delete heuristic;
}


