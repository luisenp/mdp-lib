#include <map>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <typeinfo>
#include <unistd.h>


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
        cout << "parser:" << name <<
            ": " << strerror( errno ) << endl;
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
            cout << exception << endl;
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
string getAtomsString(string stateString,
                      unordered_map<string, ushort_t>& stringAtomMap)
{
    string atomsString = "";
    for (int i = 0; i < stateString.size(); i++) {
        if (stateString[i] == '(') {
            int j;
            for (j = i; stateString[j] != ')'; j++)
                atomsString += stateString[j];
            atomsString += stateString[j];
            atomsString +=  " ";
            i = j;
        }
    }
    return atomsString;
}


/* Simulates receiving an action from the client. */
mlcore::Action*
simulateActionFromClient(mlcore::State* state,
                         Solver* solver,
                         unordered_map<string, ushort_t>& stringAtomMap) {
    ostringstream oss;
    oss << state;
    cout << getAtomsString(oss.str(), stringAtomMap) << endl;
    // SEND OSS.STR() TO CLIENT
    // RECEIVE HERE FROM CLIENT
    return solver->solve(state);
}



int main(int argc, char **argv)
{
    mdplib_debug = true;

    string file;
    string prob;
    problem_t *problem = NULL;
    pair<state_t *,Rational> *initial = NULL;

    if (argc < 2) {
        cout << "Usage: testClient [file] [problem]\n";
        return -1;
    }

    file = argv[1];
    prob = argv[2];

    if( !read_file( file.c_str() ) ) {
        cout <<
            "<main>: ERROR: couldn't read problem file `" << file << endl;
        return( -1 );
    }
    problem = (problem_t*) problem_t::find( prob.c_str() );
    if( !problem ) {
        cout << "<main>: ERROR: problem `" << prob <<
            "' is not defined in file '" << file << "'" << endl;
        return( -1 );
    }

    /* Initializing problem */
    MLProblem = new mlppddl::PPDDLProblem(problem);
    mlppddl::PPDDLHeuristic* heuristic =
        new mlppddl::PPDDLHeuristic(MLProblem, mlppddl::FF);
    MLProblem->setHeuristic(heuristic);

    mlcore::State* currentState = MLProblem->initialState();

    /* Setting up the socket to communicate with server. */
    int portno = 1234;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cerr << "ERROR: couldn't open socket." << endl;
        exit(-1);
    }
    struct hostent *server = gethostbyname("localhost");
    if (server == NULL) {
        cout << "No host: localhost." << endl;
        exit(0);
    }
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        cerr << "ERROR: couldn't connect." << endl;
        exit(-1);
    }

    /* Sending a state to the server. */
    char buffer[1024];
    bzero(buffer, 1024);
    sprintf(buffer, "state:(somegarbagestate) (moregarbage)");
    int n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) {
        cerr << "ERROR: couldn't write to socket." << endl;
        exit(-1);
    }

    FLARESSolver solver(MLProblem, 1000, 1.0e-3, 1);

    /* Initializing a map from atom names to atom indices. */
    unordered_map<string, ushort_t> stringAtomMap;
    initStringAtomMap(problem, stringAtomMap);

    while (!MLProblem->goal(currentState)) {
        if (currentState->deadEnd()) {
            cout << "DEAD-END" << endl;
            break;
        }
        mlcore::Action* action =
            simulateActionFromClient(currentState, &solver, stringAtomMap);
        cout << currentState << " " << action << endl;
        currentState = randomSuccessor(MLProblem, currentState, action);
    }

    delete heuristic;
}


