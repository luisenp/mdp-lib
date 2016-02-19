#include <cassert>
#include <ctime>
#include <list>
#include <sstream>
#include <string>
#include <typeinfo>

#include "../../include/domains/racetrack/RacetrackProblem.h"
#include "../../include/domains/racetrack/RTrackDetHeuristic.h"
#include "../../include/domains/DummyState.h"
#include "../../include/domains/WrapperProblem.h"

#include "../include/ppddl/mini-gpt/states.h"
#include "../include/ppddl/mini-gpt/problems.h"
#include "../include/ppddl/mini-gpt/domains.h"
#include "../include/ppddl/mini-gpt/states.h"
#include "../include/ppddl/mini-gpt/exceptions.h"
#include "../../include/ppddl/PPDDLHeuristic.h"
#include "../../include/ppddl/PPDDLProblem.h"


#include "../../include/reduced/LeastLikelyOutcomeReduction.h"
#include "../../include/reduced/MostLikelyOutcomeReduction.h"
#include "../../include/reduced/RacetrackObviousReduction.h"
#include "../../include/reduced/ReducedHeuristicWrapper.h"
#include "../../include/reduced/ReducedModel.h"
#include "../../include/reduced/ReducedState.h"
#include "../../include/reduced/ReducedTransition.h"

#include "../../include/solvers/LAOStarSolver.h"
#include "../../include/solvers/Solver.h"
#include "../../include/solvers/VISolver.h"

#include "../../include/util/flags.h"
#include "../../include/util/general.h"

#include "../../include/Problem.h"


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
ReducedHeuristicWrapper* reducedHeuristic = nullptr;
WrapperProblem* wrapperProblem = nullptr;
list<ReducedTransition *> reductions;

/*
 * Triggers re-planning an returns the time spent planning.
 * Proactive re-planning plans for the set of successors of the given state.
 * under the full transition model (with exception counters = 0).
 * More details can be found in
 * http://anytime.cs.umass.edu/shlomo/papers/PZicaps14.pdf
 */
double triggerReplan(Solver& solver, ReducedState* nextState, bool proactive)
{
    if (reducedModel->goal(nextState))
        return 0.0;
    if (proactive) {
        mlcore::Action* bestAction = nextState->bestAction();
        // This action can't be null because we are planning pro-actively.
        assert(bestAction != nullptr);

        // We plan for all successors of the nextState under the full
        // model. The -1 is used to get the full model transition (see comment
        // below in the simulate function).
        list<mlcore::Successor> successorsFullModel =
            reducedModel->transition(
                new ReducedState(
                    nextState->originalState(), -1, reducedModel),
                bestAction);
        list<mlcore::Successor> dummySuccessors;
        for (mlcore::Successor sccr : successorsFullModel) {
            ReducedState* reducedSccrState =
                static_cast<ReducedState*>(
                    reducedModel->addState(new ReducedState(
                        static_cast<ReducedState*>(sccr.su_state)->
                            originalState(),
                        0,
                        reducedModel)));
            dummySuccessors.push_back(
                mlcore::Successor(reducedSccrState, sccr.su_prob));
        }
        wrapperProblem->setDummyAction(bestAction);
        wrapperProblem->dummyState()->setSuccessors(dummySuccessors);
        solver.solve(wrapperProblem->dummyState());
        return 0.0;  // This happens in parallel to action execution.
    } else {
        clock_t startTime = clock();
        solver.solve(nextState);
        clock_t endTime = clock();
        return (double(endTime - startTime) / CLOCKS_PER_SEC);

    }
}


pair<double, double> simulate(Solver & solver)
{
    double cost = 0.0;
    double totalPlanningTime = 0.0;
    ReducedState* currentState =
        static_cast<ReducedState*>(reducedModel->initialState());
    bool resetExceptionCounter = false;
    ReducedState auxState(*currentState);
    while (!reducedModel->goal(currentState)) {
        mlcore::Action* bestAction = currentState->bestAction();
        cost += reducedModel->cost(currentState, bestAction);
        int exceptionCount = currentState->exceptionCount();

        if (verbosity > 100) {
            cerr << currentState << "  ";
            assert(bestAction != nullptr);
            cerr << bestAction << endl;
        }

        // Simulating the action execution using the full model.
        // Since we want to use the full transition function for this,
        // we set the exception counter of the current state to -1
        // so that it's guaranteed to be lower than the
        // exception bound k, thus forcing the reduced model to use the full
        // transition. We don't use reducedModel->useFullTransition(true)
        // because we still want to know if the outcome was an exception or not.
        // TODO: Make a method in ReducedModel that does this because this
        // approach will make reducedModel store the copies with j=-1.
//        auxState = *currentState;
        auxState.originalState(currentState->originalState());
        auxState.exceptionCount(-1);
        ReducedState* nextState = static_cast<ReducedState*>(
            randomSuccessor(reducedModel, &auxState, bestAction));
//        auxState = *nextState;
        auxState.originalState(nextState->originalState());
        auxState.exceptionCount(nextState->exceptionCount());

        // Adjusting the result to the current exception count.
        if (resetExceptionCounter) {
            // We reset the exception counter after pro-active re-planning.
            auxState.exceptionCount(0);
            resetExceptionCounter = false;
        } else {
            if (auxState.exceptionCount() == -1)    // no exception happened.
                auxState.exceptionCount(exceptionCount);
            else
                auxState.exceptionCount(exceptionCount + 1);
        }
        nextState =
            static_cast<ReducedState*>(reducedModel->getState(&auxState));

        // Re-planning
        // Checking if the state has already been considered during planning.
        if (nextState == nullptr) {
            // State wasn't considered before.
            assert(k == 0);  // Only determinization should reach here.
            if (verbosity > 100)
                cout << "No plan for this state. Re-planning." << endl;
            auxState.exceptionCount(0);
            nextState =
                static_cast<ReducedState*>(reducedModel->addState(&auxState));
            totalPlanningTime += triggerReplan(solver, nextState, false);
            assert(nextState != nullptr);
        } else {
            if (nextState->exceptionCount() == k) {
                if (verbosity > 100)
                    cout << "Pro-active re-planning." << endl;
                totalPlanningTime += triggerReplan(solver, nextState, true);
                resetExceptionCounter = true;
            }
        }
        currentState = nextState;
    }

    return make_pair(cost, totalPlanningTime);
}


void initRacetrack(string trackName, int mds)
{
    problem = new RacetrackProblem(trackName.c_str());
    static_cast<RacetrackProblem*>(problem)->pError(0.05);
    static_cast<RacetrackProblem*>(problem)->pSlip(0.10);
    static_cast<RacetrackProblem*>(problem)->mds(mds);
    heuristic = new RTrackDetHeuristic(trackName.c_str());
    problem->generateAll();
    if (verbosity > 100)
        cout << "Generated " << problem->states().size() << " states." << endl;

    reductions.push_back(
        new RacetrackObviousReduction(static_cast<RacetrackProblem*>(problem)));
    reductions.push_back(new LeastLikelyOutcomeReduction(problem));
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
    internalPPDDLProblem = (problem_t *)(problem_t::find(prob.c_str()));
    if( !internalPPDDLProblem ) {
        cerr << "<main>: ERROR: problem `" << prob <<
            "' is not defined in file '" << file << "'" << endl;
        return false;
    }

    problem = new PPDDLProblem(internalPPDDLProblem);
    heuristic = new mlppddl::PPDDLHeuristic(static_cast<PPDDLProblem*>(problem),
                                            mlppddl::atomMin1Forward);
    problem->setHeuristic(heuristic);
    reductions.push_back(new LeastLikelyOutcomeReduction(problem));
    reductions.push_back(new MostLikelyOutcomeReduction(problem));
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

////////////////////////////////////////////////
    // Testing the code that evaluates the reductions on a small sub-problem
    // We use this wrapper problem to generate small sub-problems for
    // learning the best reduced model for the original problem.
    wrapperProblem = new WrapperProblem(problem);
    mlcore::StateSet reachableStates, tipStates;
    getReachableStates(wrapperProblem, 2, reachableStates, tipStates);
    cout << "reachable " << reachableStates.size() <<
        " tip " << tipStates.size() << endl;
    wrapperProblem->overrideStates(reachableStates);
    for (mlcore::State* tip : tipStates) {
        wrapperProblem->addOverrideGoal(tip);
        cerr << "new goal " << tip << endl;
        break;
    }
    wrapperProblem->setHeuristic(nullptr);
    ReducedTransition* bestReduction = ReducedModel::getBestReduction(
          wrapperProblem, reductions, k, reducedHeuristic);
    for (mlcore::State* s : wrapperProblem->states())
        s->reset(); // Make sure the stored values/actions are cleared.
/////////////////////////////////////////////////

    reducedModel = new ReducedModel(problem, bestReduction, k);
    reducedHeuristic = new ReducedHeuristicWrapper(heuristic);
    static_cast<ReducedModel*>(reducedModel)->
        useFullTransition(flag_is_registered("use_full"));

    // We will now use the wrapper for the pro-active re-planning approach. It
    // will allow us to plan in advance for the set of successors of a
    // state-action.
    wrapperProblem->setNewProblem(reducedModel);

    // Solving reduced model using LAO*
    double totalPlanningTime = 0.0;
    clock_t startTime = clock();
    LAOStarSolver solver(wrapperProblem);
    solver.solve(wrapperProblem->initialState());
    cout << "cost " << wrapperProblem->initialState()->cost() << endl;
    clock_t endTime = clock();
    totalPlanningTime += (double(endTime - startTime) / CLOCKS_PER_SEC);

    // Running a trial of the continual planning approach.
    pair<double, double> costAndTime = simulate(solver);
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
