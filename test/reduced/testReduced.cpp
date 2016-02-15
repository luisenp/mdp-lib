#include <cassert>
#include <ctime>
#include <sstream>
#include <string>
#include <typeinfo>

#include "../../include/Problem.h"
#include "../../include/domains/racetrack/RacetrackProblem.h"
#include "../../include/domains/racetrack/RTrackDetHeuristic.h"
#include "../../include/ppddl/PPDDLHeuristic.h"
#include "../../include/ppddl/PPDDLProblem.h"
#include "../include/ppddl/mini-gpt/states.h"
#include "../include/ppddl/mini-gpt/problems.h"
#include "../include/ppddl/mini-gpt/domains.h"
#include "../include/ppddl/mini-gpt/states.h"
#include "../include/ppddl/mini-gpt/exceptions.h"

#include "../../include/reduced/LeastLikelyOutcomeReduction.h"
#include "../../include/reduced/MostLikelyOutcomeReduction.h"
#include "../../include/reduced/RacetrackObviousReduction.h"
#include "../../include/reduced/ReducedHeuristicWrapper.h"
#include "../../include/reduced/ReducedModel.h"
#include "../../include/reduced/ReducedTransition.h"

#include "../../include/solvers/Solver.h"
#include "../../include/solvers/LAOStarSolver.h"

#include "../../include/util/flags.h"
#include "../../include/util/general.h"


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
static int k = 0;

mlcore::Problem* problem = nullptr;
mlcore::Heuristic* heuristic = nullptr;
mlcore::Problem* reducedModel = nullptr;
mlcore::Heuristic* reducedHeuristic = nullptr;


double triggerReplan(Solver& solver,
                     mlcore::State** currentState,
                     ReducedState** reducedState)
{
    (*reducedState)->exceptionCount(0);
    *currentState = reducedModel->addState(*reducedState);

    // addState might delete reducedState if it was already there.
    *reducedState = (ReducedState *) *currentState;
    clock_t startTime = clock();
    solver.solve(*currentState);
    clock_t endTime = clock();
    return (double(endTime - startTime) / CLOCKS_PER_SEC);
}


pair<double, double> simulate(mlcore::Problem* reducedModel, Solver & solver)
{
    double cost = 0.0;
    double totalPlanningTime = 0.0;
    mlcore::State* currentState = reducedModel->initialState();
    ReducedState* reducedState = (ReducedState *) currentState;
    while (!reducedModel->goal(currentState)) {
        if (verbosity > 100)
            cout << currentState << "  " << currentState->bestAction() << endl;

        int exceptionCount = reducedState->exceptionCount();

        // In the simulation we want to use the full transition function.
        // To do this we set the exception counter of the current state to -1
        // so that it's guaranteed to be lower than the exception bound k
        // and the reduced model is forced to use the full transition.
        // We don't use reducedModel->useFullTransition(true) because we still
        // want to know if the outcome was an exception or not.
        ReducedState* auxState =
            new ReducedState(reducedState->originalState(), -1, reducedModel);
        mlcore::Action* bestAction = currentState->bestAction();
        cost += reducedModel->cost(reducedState, bestAction);
        currentState = randomSuccessor(reducedModel, auxState, bestAction);

        // Adjusting to the correct the exception count.
        reducedState = (ReducedState *) currentState;
        if (reducedState->exceptionCount() == -1)
            reducedState->exceptionCount(exceptionCount);
        else
            reducedState->exceptionCount(exceptionCount + 1);
        currentState = reducedModel->getState(currentState);

        if (currentState == nullptr) {
            assert(k == 0); // Only determinization should reach here.
            if (verbosity > 100)
                cout << "No plan for this state. Re-planning." << endl;
            totalPlanningTime +=
              triggerReplan(solver, &currentState, &reducedState);
              assert(currentState != nullptr);
        } else if (reducedState->exceptionCount() == k) {
            if (verbosity > 100)
                cout << "Pro-active re-planning." << endl;
            totalPlanningTime +=
              triggerReplan(solver, &currentState, &reducedState);
        }
    }

    return make_pair(cost, totalPlanningTime);
}


void initRacetrack(string trackName, int mds)
{
    mlcore::Problem* problem = new RacetrackProblem(trackName.c_str());
    ((RacetrackProblem*) problem)->pError(0.05);
    ((RacetrackProblem*) problem)->pSlip(0.10);
    ((RacetrackProblem*) problem)->mds(mds);
    heuristic = new RTrackDetHeuristic(trackName.c_str());
    problem->generateAll();
    if (verbosity > 100)
        cout << "Generated " << problem->states().size() << " states." << endl;

    ReducedTransition* obviousReduction =
        new RacetrackObviousReduction((RacetrackProblem *) problem);
    reducedModel = new ReducedModel(problem, obviousReduction, k);
    reducedHeuristic = new ReducedHeuristicWrapper(heuristic);
    reducedModel->setHeuristic(reducedHeuristic);
    reducedModel->generateAll();
    if (verbosity > 100)
        cout << "Generated " << reducedModel->states().size() <<
            " reduced states." << endl;

    // Testing the code that tries different reductions of the racetrack.
    ReducedTransition* lloReduction =
        new LeastLikelyOutcomeReduction((RacetrackProblem *) problem);
    list<ReducedTransition *> reductions;
    reductions.push_back(obviousReduction);
    reductions.push_back(lloReduction);
    ReducedTransition* bestReduction =
        ReducedModel::getBestReduction(
          problem, reductions, k, (ReducedHeuristicWrapper *) reducedHeuristic);
    assert(bestReduction == obviousReduction);


    // Testing the reachable states code
    mlcore::StateSet reachableStates;
    getReachableStates(problem, 5, reachableStates);
    cout << reachableStates.size() << endl;
}


/*
 * Parses the given PPDDL file, and returns true on success.
 */
static bool read_file( const char* ppddlFileName )
{
    yyin = fopen( ppddlFileName, "r" );
    if( yyin == NULL ) {
        cout << "parser:" << ppddlFileName <<
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
            cout << exception << endl;
            return( false );
        }
        fclose( yyin );
        return( success );
    }
}


bool initPPDDL(string ppddlArgs, problem_t* internalPPDDLProblem)
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
    internalPPDDLProblem = (problem_t*) problem_t::find( prob.c_str() );
    if( !internalPPDDLProblem ) {
        cerr << "<main>: ERROR: problem `" << prob <<
            "' is not defined in file '" << file << "'" << endl;
        return false;
    }

    problem = new PPDDLProblem(internalPPDDLProblem);
    heuristic =
        new mlppddl::PPDDLHeuristic((PPDDLProblem*) problem,
                                    mlppddl::atomMin1Forward);
    problem->setHeuristic(heuristic);

    mdplib_debug = true;
    ReducedTransition* lloReduction =
        new LeastLikelyOutcomeReduction(problem);
    ReducedTransition* mloReduction =
        new MostLikelyOutcomeReduction(problem);
    list<ReducedTransition *> reductions;
    reductions.push_back(mloReduction);
    reductions.push_back(lloReduction);
    reducedHeuristic = new ReducedHeuristicWrapper(heuristic);
    ReducedTransition* bestReduction =
        ReducedModel::getBestReduction(
          problem, reductions, k, (ReducedHeuristicWrapper *) reducedHeuristic);

    reducedModel = new ReducedModel(problem, bestReduction, k);
}


int main(int argc, char* args[])
{
    register_flags(argc, args);

    // Reading flags.
    assert(flag_is_registered_with_value("domain"));
    string domainName = flag_value("domain");

    assert(flag_is_registered_with_value("problem"));

    if (flag_is_registered_with_value("v"))
        verbosity = stoi(flag_value("v"));


    if (flag_is_registered_with_value("k"))
        k = stoi(flag_value("k"));

    // Creating problem
    if (domainName == "racetrack") {
        int mds = -1;
        if (flag_is_registered_with_value("mds"))
            mds = stoi(flag_value("mds"));
        string trackName = flag_value("problem");
        initRacetrack(trackName, mds);
    } else if (domainName == "ppddl") {
        string ppddlArgs = flag_value("problem");
        problem_t *internalPPDDLProblem = nullptr;
        initPPDDL(ppddlArgs, internalPPDDLProblem);
    }

    if (flag_is_registered("use_full"))
        ((ReducedModel *)reducedModel)->useFullTransition(true);

    // Solving reduced model using LAO*
    double totalPlanningTime = 0.0;
    clock_t startTime = clock();
    LAOStarSolver solver(reducedModel, 1.0e-3);
    solver.solve(reducedModel->initialState());
    clock_t endTime = clock();
    totalPlanningTime += (double(endTime - startTime) / CLOCKS_PER_SEC);

    // Running a trial of the continual planning approach using the reduced model.
    pair<double, double> costAndTime = simulate(reducedModel, solver);

    if (verbosity > 100) {
        cout << "Total cost " << costAndTime.first << endl;
        cout << "Total planning time " <<
            costAndTime.second + totalPlanningTime << endl;
    } else {
        cout << costAndTime.first << " "
            << costAndTime.second + totalPlanningTime << endl;
    }

    return 0;
}
