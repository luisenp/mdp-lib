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

#include "../../include/reduced/BestDeterminizationReduction.h"
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
ReducedModel* reducedModel = nullptr;
ReducedHeuristicWrapper* reducedHeuristic = nullptr;
WrapperProblem* wrapperProblem = nullptr;
list<ReducedTransition *> reductions;


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
    heuristic = new mlppddl::PPDDLHeuristic(static_cast<PPDDLProblem*>(problem),
                                            mlppddl::FF);
//                                            mlppddl::atomMin1Forward);
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
        initPPDDL(ppddlArgs);
    }

    bool useFullTransition = flag_is_registered("use_full");

    double totalReductionTime = 0.0;
    ReducedTransition* bestReduction = nullptr;
    wrapperProblem = new WrapperProblem(problem);
    mlcore::StateSet reachableStates, tipStates;
////////////////////////////////////////////////
    // Testing the code that evaluates the reductions on a small sub-problem
    // We use this wrapper problem to generate small sub-problems for
    // learning the best reduced model for the original problem.
    if (!useFullTransition) {
                                                                                mdplib_debug = true;
        getReachableStates(wrapperProblem,
                           wrapperProblem->initialState(),
                           5,
                           reachableStates,
                           tipStates);

        cout << "reachable " << reachableStates.size() <<
            " tip " << tipStates.size() << endl;
        wrapperProblem->overrideGoals(&tipStates);


      wrapperProblem->overrideStates(&reachableStates);
      BestDeterminizationReduction* best =
          new BestDeterminizationReduction(wrapperProblem);

//        wrapperProblem->setHeuristic(nullptr);
//        clock_t startTimeReduction = clock();
//        bestReduction = ReducedModel::getBestReduction(
//              wrapperProblem, reductions, k, nullptr);
//        clock_t endTimeReduction = clock();
//        totalReductionTime =
//            double(endTimeReduction - startTimeReduction) / CLOCKS_PER_SEC;

        for (mlcore::State* s : wrapperProblem->states())
            s->reset(); // Make sure the stored values/actions are cleared.
    }
/////////////////////////////////////////////////

    reducedModel = new ReducedModel(problem, bestReduction, k);
    reducedHeuristic = new ReducedHeuristicWrapper(heuristic);
    reducedModel->setHeuristic(reducedHeuristic);
    static_cast<ReducedModel*>(reducedModel)->
        useFullTransition(useFullTransition);

    // We will now use the wrapper for the pro-active re-planning approach. It
    // will allow us to plan in advance for the set of successors of a
    // state-action.
    wrapperProblem->setNewProblem(reducedModel);

    // Solving reduced model using LAO*
    double totalPlanningTime = 0.0;
    clock_t startTime = clock();
    LAOStarSolver solver(wrapperProblem);
    solver.solve(wrapperProblem->initialState());
    clock_t endTime = clock();
    totalPlanningTime += (double(endTime - startTime) / CLOCKS_PER_SEC);
    cout << "cost " << wrapperProblem->initialState()->cost() <<
        " time " << totalPlanningTime << endl;


    // Running a trial of the continual planning approach.
    double expectedCost = 0.0;
    int nsims = 30;
    for (int i = 0; i < nsims; i++) {
        pair<double, double> costAndTime =
            reducedModel->trial(solver, wrapperProblem);
        expectedCost += costAndTime.first;
    }
    cout << expectedCost / nsims << endl;
    cout << totalPlanningTime + totalReductionTime << endl;
//    if (verbosity > 100) {
//        cout << "Total cost " << costAndTime.first << endl;
//        cout << "Total planning time " <<
//            costAndTime.second + totalPlanningTime << endl;
//    } else {
//        cout << costAndTime.first << " "
//            << costAndTime.second + totalPlanningTime << endl;
//    }

    // Releasing memory
    for (auto reduction : reductions)
        delete reduction;
    reducedModel->cleanup();
    delete reducedModel;
    wrapperProblem->cleanup();
    delete wrapperProblem;
    delete problem;
    return 0;
}
