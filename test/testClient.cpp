#include <map>
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
    string file;
    string prob;
    problem_t *problem = NULL;
    pair<state_t *,Rational> *initial = NULL;

    if (argc < 2) {
        cout << "Usage: testClient [file] [problem].\n";
        exit(0);
    }

    file = argv[1];
    prob = argv[2];

    if( !read_file( file.c_str() ) ) {
        cout <<
            "<main>: ERROR: couldn't read problem file. `" << file << endl;
        exit(-1);
    }
    problem = (problem_t*) problem_t::find( prob.c_str() );
    if( !problem ) {
        cout << "<main>: ERROR: problem `" << prob <<
            "' is not defined in file. '" << file << "'" << endl;
        exit(-1);
    }

    /* Initializing problem. */
    mlppddl::PPDDLProblem* MLProblem = new mlppddl::PPDDLProblem(problem);
    mlppddl::PPDDLHeuristic* heuristic =
        new mlppddl::PPDDLHeuristic(MLProblem, mlppddl::FF);
    MLProblem->setHeuristic(heuristic);

    /* Setting up the server. */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cerr << "ERROR: couldn't open socket." << endl;
        exit(-1);
    }
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    int portno = 1234;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "ERROR: couldn't bind the socket." << endl;
        exit(-1);
    }
    struct sockaddr_in cli_addr;
    listen(sockfd, 5);
    socklen_t clilen = sizeof(cli_addr);

    /* Reading communication from the client. */
    int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
        cerr << "ERROR: problems accepting connection." << endl;
        exit(-1);
    }
    char buffer[1024];
    bzero(buffer, 1024);
    int n = read(newsockfd, buffer, 1023);
    if (n < 0) {
        cerr << "ERROR: couldn't read from socket." << endl;
        exit(-1);
    }
    string msg(buffer);
    if (msg.substr(0, 6) == "state:") {    // Received a state to plan for
        cout << msg.substr(6, msg.size()) << endl;
    }

    /* Initializing a map from atom names to atom indices. */
    unordered_map<string, ushort_t> stringAtomMap;
    initStringAtomMap(problem, stringAtomMap);

    FLARESSolver solver(MLProblem, 1000, 1.0e-3, 1);
    mlcore::State* currentState = MLProblem->initialState();
    string receivedString = simulateServerComm(currentState);
    while (!MLProblem->goal(currentState)) {
        mlcore::Action* action = solver.solve(currentState);
        if (currentState->deadEnd()) {
            cout << "DEAD-END!" << endl;
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

