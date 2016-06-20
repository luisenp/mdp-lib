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

    cout << "INITIAL: " << MLProblem->initialState() << " ";


    list<mlcore::State*> Q;
    Q.push_back(MLProblem->initialState());
    int cnt = 0;
    while (cnt < 10) {
        mlcore::State* cur = Q.back();
        Q.pop_back();
        cnt++;
        cout << cur << endl;
        for (mlcore::Action* a : MLProblem->actions()) {
            if (MLProblem->applicable(cur, a)) {
                for (auto const & scc : MLProblem->transition(cur, a))
                    Q.push_back(scc.su_state);
            }
        }
    }

    Domain dom = problem->domain();
    PredicateTable& preds = dom.predicates();
    TermTable& terms = problem->terms();

    pair<Predicate, bool> pr = preds.find_predicate("vehicle-at");
    pair<Object, bool> tr = terms.find_object("l-1-1");

    TermList tl;
    tl.push_back(tr.first);
    Atom att = Atom::make_atom(pr.first, tl);
    cout << "Atom:" << endl;
    att.print(cout, preds, dom.functions(), terms);
    cout << endl;

    state_t* sss = new state_t();
    sss->add(problem_t::atom_hash_get(att));

    cout << "State:" << endl;
    sss->full_print(cout, problem);
    cout << endl;

    delete sss;
    delete heuristic;
}

