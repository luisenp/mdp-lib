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

#include "../include/State.h"


#define BUFFER_SIZE 1024


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


/* Simulates receiving an action description from the client. */
string
getActionFromServer(int sockfd,
                    mlcore::State* state,
                    unordered_map<string, ushort_t>& stringAtomMap) {

    /* Sending the state description to the planning server. */
    ostringstream oss;
    oss << "state:" << state;
    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    sprintf(buffer, "%s", oss.str().c_str());
    cout << "SENDING: " << oss.str() << endl;
    int n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) {
        cerr << "ERROR: couldn't write to socket." << endl;
        return "";
    }
    bzero(buffer, BUFFER_SIZE);
    n = read(sockfd, buffer, BUFFER_SIZE - 1);
    if (n < 0) {
        cerr << "ERROR: couldn't read from socket." << endl;
        return "";
    }
    return string(buffer);
}


/* Simulates receiving an action description from the client. */
void stopServer(int sockfd) {

    /* Sending the state description to the planning server. */
    ostringstream oss;
    oss << "stop:";
    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    sprintf(buffer, "%s", oss.str().c_str());
    cout << "SENDING: " << oss.str() << endl;
    int n = write(sockfd, buffer, strlen(buffer));
    if (n < 0)
        cerr << "ERROR: couldn't write to socket." << endl;
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
        exit(0);
    }

    file = argv[1];
    prob = argv[2];

    if( !read_file( file.c_str() ) ) {
        cout <<
            "<main>: ERROR: couldn't read problem file `" << file << endl;
        exit(-1);
    }
    problem = (problem_t*) problem_t::find( prob.c_str() );
    if( !problem ) {
        cout << "<main>: ERROR: problem `" << prob <<
            "' is not defined in file '" << file << "'" << endl;
        exit(-1);
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
        close(sockfd);
        exit(-1);
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
        close(sockfd);
        exit(-1);
    }

    /* Initializing a map from atom names to atom indices. */
    unordered_map<string, ushort_t> stringAtomMap;
    initStringAtomMap(problem, stringAtomMap);

    /* Simulating a run of the plan computed by the planning server. */
    while (!MLProblem->goal(currentState)) {
        cout << currentState << " ";
        if (currentState->deadEnd()) {
            cout << "DEAD-END" << endl;
            break;
        }
        string actionDescription =
            getActionFromServer(sockfd, currentState, stringAtomMap);
        mlcore::Action* action = nullptr;
        for (mlcore::Action* a : MLProblem->actions()) {
            ostringstream oss;
            oss << a;
            if (oss.str() == actionDescription)
                action = a;
        }
        cout << action << endl;
        currentState = randomSuccessor(MLProblem, currentState, action);
    }
    stopServer(sockfd);
    close(sockfd);
    delete heuristic;
    return 0;
}


