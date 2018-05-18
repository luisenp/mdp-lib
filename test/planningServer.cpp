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

#include "../include/solvers/FLARESSolver.h"
#include "../include/solvers/HDPSolver.h"
#include "../include/solvers/LAOStarSolver.h"
#include "../include/solvers/SoftFLARESSolver.h"
#include "../include/solvers/Solver.h"
#include "../include/solvers/SSiPPSolver.h"

#include "../include/util/flags.h"

#include "../include/State.h"

#define BUFFER_SIZE 65536


using namespace mdplib;
using namespace mlsolvers;
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
mlcore::State* getStateFromString(
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


int main(int argc, char *args[])
{
    string file;
    string prob;
    string algorithm;
    problem_t *problem = NULL;
    pair<state_t *,Rational> *initial = NULL;

    if (argc < 4) {
        cout << "Usage: planserv [file] [problem] [algorithm].\n";
        exit(0);
    }

    file = args[1];
    prob = args[2];
    algorithm = args[3];


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
        close(sockfd);
        exit(-1);
    }
    /* Connect to the client. */
    struct sockaddr_in cli_addr;
    listen(sockfd, 5);
    socklen_t clilen = sizeof(cli_addr);
    int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
        cerr << "ERROR: problems accepting connection." << endl;
        close(sockfd);
        exit(-1);
    }

    /* Initializing a map from atom names to atom indices. */
    unordered_map<string, ushort_t> stringAtomMap;
    initStringAtomMap(problem, stringAtomMap);

    /* Planner to use. */
    Solver* solver = nullptr;
    if (algorithm == "flares") {
        int horizon = 1;
        if (argc > 4) {
            horizon = atoi(args[4]);
        }
        solver = new FLARESSolver(MLProblem, 100, 1.0e-3, horizon);
    } else if (algorithm == "soft-flares") {
        register_flags(argc, args);
        if (flag_is_registered("debug"))
            mdplib_debug = true;
        double depth = 4;
        double alpha = 0.10;
        double tol = 1.0e-3;
        int trials = 1000;
        if (flag_is_registered_with_value("depth"))
            depth = stoi(flag_value("depth"));
        if (flag_is_registered_with_value("trials"))
            trials = stoi(flag_value("trials"));
        TransitionModifierFunction mod_func = kLogistic;
        DistanceFunction dist_func = kStepDist;
        HorizonFunction horizon_func = kFixed;
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
        // Labeling functions
        if (flag_is_registered("horf")) {
            string labelf_str = flag_value("horf");
            if (labelf_str == "exp") {
                horizon_func = kExponentialH;
            } else if (labelf_str == "fixed") {
                horizon_func = kFixed;
            } else if (labelf_str == "bern") {
                horizon_func = kBernoulli;
            } else {
                cerr << "Error: unknown labeling function." << endl;
                exit(0);
            }
        }
        solver = new SoftFLARESSolver(
            MLProblem, trials, tol, depth,
            mod_func, dist_func, horizon_func, alpha);
        solver->maxPlanningTime(5000);
    } else if (algorithm == "ssipp") {
        int horizon = 2;
        if (argc > 4) {
            horizon = atoi(args[4]);
        }
        solver = new SSiPPSolver(MLProblem, 1.0e-3, horizon);
    } else if (algorithm == "hdp") {
        solver =  new HDPSolver(MLProblem, 1.0e-3, 0);
    }
    else {
        solver = new LAOStarSolver(MLProblem);
    }

    /* Solving states on demand. */
    mlcore::StateIntMap count_seen;
    long maxPlanningTime = 5000;
    double costTrial = 0.0;
    int roundIndex = 0;
    int planningTimes[] =
        {5000, 3760, 2820, 2100, 1600, 1200, 880, 660, 500, 380};
    double timeFactor = 1.0;
    double costEstimate = 50;
    while (true) {
        /* Reading communication from the client. */
        char buffer[BUFFER_SIZE];
        bzero(buffer, BUFFER_SIZE);
        int n = read(newsockfd, buffer, BUFFER_SIZE - 1);
        if (n < 0) {
            cerr << "ERROR: couldn't read from socket." << endl;
            break;
        }
        cout << buffer << endl;
        string msg(buffer);
        string atomsString;
        if (msg.substr(0, 6) == "state:") // Received a state to plan for.
            atomsString = msg.substr(6, msg.size());
        else if (msg.substr(0, 5) == "stop:") // Stop the program.
            break;
        mlcore::State* state =
            getStateFromString(atomsString, MLProblem, stringAtomMap);
                                                                                // Rudimentary dead-end detection
                                                                                if (count_seen.count(state) == 0)
                                                                                    count_seen[state] = 0;
                                                                                count_seen[state]++;
//                                                                                cerr << count_seen[state] << endl;
        mlcore::Action* action;
        cout << "msg:" << msg << endl;
        if (msg == "end-round") {
            cout << "Round Cost: " << costTrial << endl;
            double ratio = std::min(1.0, costEstimate / costTrial);
            costEstimate = costTrial;
            timeFactor *= ratio;
            cout << "New time Factor: " << timeFactor << endl;
            costTrial = 0.0;
            action = nullptr;
            roundIndex++;
                                                                                count_seen.clear();
        } else {
            long maxPlanningTime = 120;
            if (roundIndex < 10)
                maxPlanningTime = planningTimes[roundIndex] * timeFactor;
            solver->maxPlanningTime(maxPlanningTime);
            cout << "Planning for " << maxPlanningTime << " ms." << endl;
            action = solver->solve(state); // Solving for state.
        }
        if (state->deadEnd())
            action = nullptr;
                                                                                if (count_seen[state] == 10)
                                                                                    action = nullptr;

        /* Sending the action to the client. */
        ostringstream oss;
        if (action != nullptr)
            oss << action;
        else {
            oss << "(done)";
                                                                                count_seen.clear();
        }
        costTrial += MLProblem->cost(state, action);
        bzero(buffer, BUFFER_SIZE);
        sprintf(buffer, "%s", oss.str().c_str());
        cout << "action: " << buffer << "." << endl;
        n = write(newsockfd, buffer, strlen(buffer));
        if (n < 0) {
            cerr << "ERROR: couldn't write to socket." << endl;
            break;
        }
    }
    close(sockfd);
    if (newsockfd >= 0)
        close(newsockfd);
    delete heuristic;
    return 0;
}
