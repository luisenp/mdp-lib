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


/* Returns the state corresponding to the given string. */
mlcore::State* getStatefromString(
    string stateString,
    mlppddl::PPDDLProblem* MLProblem,
    unordered_map<string, ushort_t>& stringAtomMap)
{
    state_t* pState = new state_t();
    for (int i = 0; i < stateString.size(); i++) {
        if (stateString[i] == '(') {
            string atomString = "";
            int j;
            for (j = i; stateString[j] != ')'; j++)
                atomString += stateString[j];
            atomString += stateString[j];
            pState->add(stringAtomMap[atomString]);
            i = j;
        }
    }
    mlppddl::PPDDLState* newState = new mlppddl::PPDDLState(MLProblem);
    newState->setPState(*pState);
    return MLProblem->addState(newState);
}


/* Simulates receiving the state description of the given state from server. */
string simulateServerComm(mlcore::State* state)
{
    ostringstream oss;
    oss << state;
    return oss.str();
}


int main(int argc, char **argv)
{
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
    mlppddl::PPDDLProblem* MLProblem = new mlppddl::PPDDLProblem(problem);
    mlppddl::PPDDLHeuristic* heuristic =
        new mlppddl::PPDDLHeuristic(MLProblem, mlppddl::FF);
    MLProblem->setHeuristic(heuristic);

    /* Initializing a map from atom names to atom indices. */
    unordered_map<string, ushort_t> stringAtomMap;
    initStringAtomMap(problem, stringAtomMap);

    FLARESSolver solver(MLProblem, 1000, 1.0e-3, 1);
    mlcore::State* currentState = MLProblem->initialState();
    string receivedString = simulateServerComm(currentState);
    while (!MLProblem->goal(currentState)) {
        mlcore::Action* action = solver.solve(currentState);
        if (currentState->deadEnd()) {
            cout << "DEAD-END" << endl;
            break;
        }
        cout << currentState << " " << action << endl;
        mlcore::State* successorState =
            randomSuccessor(MLProblem, currentState, action);
        receivedString = simulateServerComm(successorState);
        currentState =
            getStatefromString(receivedString, MLProblem, stringAtomMap);
    }

    delete heuristic;
}

