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

#include "../include/State.h"


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

    Domain dom = problem->domain();
    PredicateTable& preds = dom.predicates();
    TermTable& terms = problem->terms();

    map<const Atom*,ushort_t>& atom_hash = problem_t::atom_hash();
    unordered_map<string, ushort_t> stringAtomMap;

    cout << " *******************************" << endl;
    for (auto const & foo : atom_hash) {
        ostringstream oss;
        foo.first->print(oss, preds, dom.functions(), terms);
        stringAtomMap[oss.str()] = foo.second;
    }
    for (auto const & foo : stringAtomMap) {
        cout << foo.first << " " << stringAtomMap[foo.first] << endl;
    }
    cout << "*******************************" << endl;


    list<mlcore::State*> Q;
    Q.push_back(MLProblem->initialState());
    int cnt = 0;
    while (cnt < 10) {
        mlcore::State* cur = Q.back();
        Q.pop_back();
        cnt++;
        ostringstream oss;
        oss << cur;
        string stateString = oss.str();
        state_t* bar = new state_t();
        for (int i = 0; i < stateString.size(); i++) {
            if (stateString[i] == '(') {
                string atomString = "";
                int j;
                for (j = i; stateString[j] != ')'; j++)
                    atomString += stateString[j];
                atomString += stateString[j];
                cout << atomString << " " << stringAtomMap[atomString] << ", ";
                bar->add(stringAtomMap[atomString]);
                i = j;
            }
        }
        cout << endl;
        cout << cur << endl;
        cout << "created "; bar->print(cout); cout << endl;
        mlppddl::PPDDLState* newState = new mlppddl::PPDDLState(MLProblem);
        newState->setPState(*bar);
        cout << "new " << newState << " " << (void *) newState << endl;
        cout << "fetched " << (void *) MLProblem->addState(newState) <<
            " " << (void *) cur << endl;
        for (mlcore::Action* a : MLProblem->actions()) {
            if (MLProblem->applicable(cur, a)) {
                for (auto const & scc : MLProblem->transition(cur, a))
                    Q.push_back(scc.su_state);
            }
        }
    }
    delete heuristic;
}

