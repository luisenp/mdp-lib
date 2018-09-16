#include <cassert>
#include <ctime>
#include <iostream>
#include <list>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <typeinfo>
#include <unistd.h>

#include "../../include/domains/racetrack/RacetrackProblem.h"
#include "../../include/domains/racetrack/RTrackDetHeuristic.h"
#include "../../include/domains/DummyState.h"

#include "../include/ppddl/mini-gpt/states.h"
#include "../include/ppddl/mini-gpt/problems.h"
#include "../include/ppddl/mini-gpt/domains.h"
#include "../include/ppddl/mini-gpt/states.h"
#include "../include/ppddl/mini-gpt/exceptions.h"
#include "../../include/ppddl/PPDDLHeuristic.h"
#include "../../include/ppddl/PPDDLProblem.h"

#include "../../include/reduced/PPDDLTaggedReduction.h"
#include "../../include/reduced/ReducedHeuristicWrapper.h"
#include "../../include/reduced/ReducedModel.h"
#include "../../include/reduced/ReducedState.h"
#include "../../include/reduced/ReducedTransition.h"

#include "../../include/solvers/FFReducedModelSolver.h"
#include "../../include/solvers/FFReplanSolver.h"
#include "../../include/solvers/RFFSolver.h"
#include "../../include/solvers/SSiPPFFSolver.h"

#include "../../include/util/flags.h"
#include "../../include/util/general.h"

#include "../../include/Problem.h"

#define BUFFER_SIZE 65536


using namespace std;
using namespace mdplib;
using namespace mlppddl;
using namespace mlreduced;
using namespace mlsolvers;


extern int yyparse();
extern FILE* yyin;
string current_file;
int warning_level = 0;

static int verbosity = 0;
static int k_reduced = 0;

mlcore::Problem* problem = nullptr;
mlcore::Heuristic* heuristic = nullptr;
ReducedModel* reducedModel = nullptr;
ReducedHeuristicWrapper* reducedHeuristic = nullptr;

string ffExec = "/home/lpineda/Desktop/FF-v2.3/ff";


/*
 * Parses the given PPDDL file, and returns true on success.
 */
static bool read_file( const char* ppddlFileName )
{
    yyin = fopen( ppddlFileName, "r" );
    if( yyin == NULL ) {
        cerr << "parser:" << ppddlFileName <<
            ": " << strerror( errno ) << endl;
        return( false );
    }
    else {
        current_file = ppddlFileName;
        bool success;
        try {
            success = (yyparse() == 0);
        }
        catch( Exception exception ) {
            fclose( yyin );
            cerr << exception << endl;
            return( false );
        }
        fclose( yyin );
        return( success );
    }
}


/*
 * Initializes the PPDDL problem with the given arguments.
 */
bool initPPDDL(string ppddlArgs)
{
    size_t pos_equals = ppddlArgs.find(":");
    assert(pos_equals != string::npos);
    string file = ppddlArgs.substr(0, pos_equals);
    string prob =
        ppddlArgs.substr(pos_equals + 1, ppddlArgs.size() - pos_equals);

    pair<state_t *,Rational> *initial = nullptr;

    if( !read_file( file.c_str() ) ) {
        cerr << "<main>: ERROR: couldn't read problem file `" << file << endl;
        return false;
    }
    problem_t* internalPPDDLProblem =
        (problem_t *)(problem_t::find(prob.c_str()));
    if( !internalPPDDLProblem ) {
        cerr << "<main>: ERROR: problem `" << prob <<
            "' is not defined in file '" << file << "'" << endl;
        return false;
    }

    problem = new PPDDLProblem(internalPPDDLProblem);
    if (flag_is_registered("heuristic") && flag_value("heuristic") == "zero")
            problem->setHeuristic(nullptr);
    else
        heuristic = new mlppddl::PPDDLHeuristic(
            static_cast<PPDDLProblem*>(problem), mlppddl::FF);
    problem->setHeuristic(heuristic);
}


/*
 * Fills the given map from atom_strings to atom_ids according to the
 * atom_hash of the given problem_t. This map can be used to recover a state
 * from a list of atoms.
 */
void fillStringAtomMap(problem_t* problem,
                       unordered_map<string, ushort_t>& stringAtomMap)
{
    const Domain& dom = problem->domain();
    const PredicateTable& preds = dom.predicates();
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


/*
 * Server used to connect to the MDPSIM client program and compute plans for
 * states sent by the MDPSIM server. The planner to choose is specified by the
 * "planner" argument flag. Options are:
 *
 *    --"ff-lao": FF-LAO*
 *    --"rff": RFF
 *    --"ff-replan": FF-Replan
 *    --"ssipp-ff": SSiPP-FF
 *
 * Additionally, the following flags are required:
 *
 *    --problem: the PPDDL domain/problem to solve. The format is
 *        ppddlFilename:problemName
 *    --dir: the directory to use for FF. This directory must contain the file
 *        "det_problem" as well as a file called "ff-template.pddl"
 *        containing only the PPDDL problem description (i.e., the (problem ...)
 *        list but not the domain).
 *        The init state must be described in a single line.
 *        The user of planserv_det.out is responsible for ensuring consistency
 *        between ff-template.pddl and the file passed in the "problem" flag.
 *    --det_problem: the path to a PDDL file describing which deterministic
 *        problem to be passed to FF.
 *    --det_descriptor: a configuration file describing the determinization
 *        in a simple format. The user of testReducedFF is responsible for
 *        ensuring consistency between the files det_problem and
 *        det_descriptor. The format of this file will be one action name per
 *        line followed by a number specifying which of the outcomes is primary.
 *
 * Other optional flags:
 *    --k: The maximum number of exceptions to use for FF-LAO* (default = 0).
 *    --heuristic: The heuristic to use (default = FF). Options are:
 *        -"zero": Zero heuristic.
 *        -"ff": FF heuristic.
 *
 * Other required command line arguments are:
 */
int main(int argc, char* args[])
{
    register_flags(argc, args);

    /* *********************** PROBLEM INITIALIZATION ********************** */

    if (flag_is_registered("debug"))
        mdplib_debug = true;

    // Reading flags.
    register_flags(argc, args);

    // The PPDDL problem to solve.
    assert(flag_is_registered_with_value("problem"));;
    string ppddlArgs = flag_value("problem");

    // The PDDL determinization to use.
    assert(flag_is_registered_with_value("det_problem"));
    string detProblem = flag_value("det_problem");

    // The configuration file for this determinization.
    assert(flag_is_registered_with_value("det_descriptor"));
    string detDescriptor = flag_value("det_descriptor");

    //The directory where the determinization is stored.
    assert(flag_is_registered_with_value("dir"));
    string directory = flag_value("dir");

    // The verbosity level.
    if (flag_is_registered_with_value("v"))
        verbosity = stoi(flag_value("v"));

    // The planner to use.
    string planner = "ff-lao";
    if (flag_is_registered("planner"))
        planner = flag_value("planner");

    //The maximum number of exceptions for the reduced model solver.
    int k_reduced = 0;
    if (flag_is_registered_with_value("k"))
        k_reduced = stoi(flag_value("k"));

    // The maximum planning time allowed to the planner.
    time_t maxPlanningTime = 60 * 5;
    if (flag_is_registered("max-time"))
        maxPlanningTime = stoi(flag_value("max-time"));
    time_t remainingPlanningTime = maxPlanningTime;

    // If true, FF will be used for the states with exception_counter = 0.
    bool useFF = true;
    if (flag_is_registered("reduced-no-ff"))
        useFF = false;

    // The number of simulations for the experiments.
    int nsims = 100;
    if (flag_is_registered_with_value("n"))
        nsims = stoi(flag_value("n"));

    initPPDDL(ppddlArgs);

    // Creating the reduced model.
    ReducedTransition* reduction = nullptr;
    if (planner == "ff-lao") {
        reduction = new PPDDLTaggedReduction(problem, detDescriptor);
        reducedModel = new ReducedModel(problem, reduction, k_reduced);
        reducedHeuristic = new ReducedHeuristicWrapper(heuristic);
        reducedModel->setHeuristic(reducedHeuristic);
    }

    /* *********************** SERVER INITIALIZATION ********************** */

    // Setting up the server to connect to MDP-SIM.
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

    // Connect to the MDP-SIM client.
    struct sockaddr_in cli_addr;
    listen(sockfd, 5);
    socklen_t clilen = sizeof(cli_addr);
    int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
        cerr << "ERROR: problems accepting connection." << endl;
        close(sockfd);
        exit(-1);
    }

    /* *********************** SOLVING THE PROBLEM ********************** */
    // Solving reduced model using LAO* + FF.
    time_t totalPlanningTime = 0.0;
    time_t startTime = time(nullptr);
     // using half of the time fot the initial plan (last argument)

    cerr << "SOLVING" << endl;
    Solver* solver = nullptr;
    if (planner == "ff-lao") {
        solver = new FFReducedModelSolver(reducedModel,
                                          ffExec,
                                          directory + "/" + detProblem,
                                          directory + "/ff-template.pddl",
                                          k_reduced,
                                          1.0e-3,
                                          useFF,
                                          maxPlanningTime / 2);
        solver->solve(reducedModel->initialState());
    } else if (planner == "rff") {
        solver = new RFFSolver(static_cast<mlppddl::PPDDLProblem*> (problem),
                               ffExec,
                               directory + "/" + detProblem,
                               directory + "/ff-template.pddl",
                               0.2,
                               100,
                               maxPlanningTime / 2);
        solver->solve(problem->initialState());
    } else if (planner == "ff-replan") {
        solver = new FFReplanSolver(
            static_cast<mlppddl::PPDDLProblem*> (problem),
            ffExec,
            directory + "/" + detProblem,
            directory + "/ff-template.pddl",
            maxPlanningTime / 2);
        solver->solve(problem->initialState());
    } else if (planner == "ssipp-ff") {
        solver = new SSiPPFFSolver(
            static_cast<mlppddl::PPDDLProblem*> (problem),
            ffExec,
            directory + "/" + detProblem,
            directory + "/ff-template.pddl",
            3,
            1.0e-3,
            maxPlanningTime / 2);
        solver->solve(problem->initialState());
    }
    time_t endTime = time(nullptr);
    totalPlanningTime += endTime - startTime;
    remainingPlanningTime -= totalPlanningTime;

    /* *********************** SEVER LOOP ********************** */
    // Initializing a map from atom names to atom indices.
    unordered_map<string, ushort_t> stringAtomMap;
    fillStringAtomMap(static_cast<PPDDLProblem*> (problem)->pProblem(),
                      stringAtomMap);
    double cost = 0.0;
    while (true) {
        // Reading communication from the client.
        char buffer[BUFFER_SIZE];
        bzero(buffer, BUFFER_SIZE);
        int n = read(newsockfd, buffer, BUFFER_SIZE - 1);
        if (n < 0) {
            cerr << "ERROR: couldn't read from socket." << endl;
            break;
        }
        cerr << "RECEIVED: " << buffer << endl;
        string msg(buffer);
        string atomsString;
        if (msg.substr(0, 6) == "state:") { // Received a state to plan for.
            atomsString = msg.substr(6, msg.size());
        } else if (msg.substr(0, 5) == "stop") { // Stop the program.
            break;
        } else if (msg.substr(0, 9) == "end-round") {
            cost = 0;
            bzero(buffer, BUFFER_SIZE);
            sprintf(buffer, "%s", "round-ended");
            cerr << "SENDING: " << buffer << "." << endl;
            n = write(newsockfd, buffer, strlen(buffer));
            if (n < 0) {
                cerr << "ERROR: couldn't write to socket." << endl;
                break;
            }
            continue;
        } else {
            cerr << "ERROR: unknown message. Terminating." << endl;
            break;
        }
        // Getting the state whose action needs to be found.
        mlcore::State* state =
            getStatefromString(atomsString,
                               static_cast<PPDDLProblem*> (problem),
                               stringAtomMap);

        cerr << "PLANNING." << endl;
        solver->maxPlanningTime(remainingPlanningTime);
        startTime = time(nullptr);
        mlcore::Action* action = nullptr;
        if (planner == "ff-lao") {
            // For now, always set the exception counter to k and re-plan.
            ReducedState* reducedState = static_cast<ReducedState*> (
                reducedModel->addState(
                    new ReducedState(state, k_reduced, reducedModel)));
            action = solver->solve(reducedState);
        } else {
            action = solver->solve(state);
        }
        endTime = time(nullptr);
        remainingPlanningTime -= endTime - startTime;
        cerr << "DONE. Remaining time: " << remainingPlanningTime << endl;

        // Sending the action to the client.
        ostringstream oss;
        if (action != nullptr && cost < mdplib::dead_end_cost) {
            oss << action;
            cost += problem->cost(state, action);
        } else {
            cerr << "DEAD-END." << endl;
            oss << "(done)";
        }
        bzero(buffer, BUFFER_SIZE);
        sprintf(buffer, "%s", oss.str().c_str());
        cerr << buffer << "." << endl;
        n = write(newsockfd, buffer, strlen(buffer));
        if (n < 0) {
            cerr << "ERROR: couldn't write to socket." << endl;
            break;
        }
    }

    // Releasing memory
    if (reducedModel != nullptr) {
        reducedModel->cleanup();
        delete reducedModel;
    }
    delete solver;
    delete problem;
    return 0;
}

