#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../../include/ppddl/PPDDLProblem.h"
#include "../../include/ppddl/mini-gpt/problems.h"
#include "../../include/reduced/ReducedModel.h"
#include "../../include/reduced/ReducedState.h"
#include "../../include/solvers/FFReducedModelSolver.h"
#include "../../include/util/general.h"


using namespace mlcore;
using namespace mlppddl;
using namespace std;


namespace mlsolvers
{

mlcore::Action* FFReducedModelSolver::solve(mlcore::State* s0)
{
    startingPlanningTime_ = time(nullptr);
    this->lao(s0);
    return s0->bestAction();
}


void FFReducedModelSolver::lao(mlcore::State* s0)
{
    // This is a stack based implementation of LAO*.
    // We don't use the existing library implementation because we are going to
    // solve the reduced states with j=k using FF.
    StateSet visited;
    int countExpanded = 0;
    while (true) {
        do {
            visited.clear();
            countExpanded = 0;
            list<mlcore::State*> stateStack;
            stateStack.push_back(s0);
            while (!stateStack.empty()) {
                if (timeHasRunOut(startingPlanningTime_, maxPlanningTime_))
                    return;
                mlcore::State* s = stateStack.back();
                stateStack.pop_back();
                if (!visited.insert(s).second)  // state was already visited.
                    continue;
                if (s->deadEnd() || problem_->goal(s))
                    continue;
                int cnt = 0;
                if (s->bestAction() == nullptr) {
                    // state has never been expanded.
                    this->bellmanUpdate(s);
                    countExpanded++;
                    continue;
                } else {
                    mlcore::Action* a = s->bestAction();
                    mlreduced::ReducedState* reducedState =
                        (mlreduced::ReducedState* ) s;
                    for (Successor sccr : problem_->transition(s, a)) {
                        if (!(useFF_ && reducedState->exceptionCount() == 0))
                            stateStack.push_back(sccr.su_state);
                    }
                }
                this->bellmanUpdate(s);
            }
        } while (countExpanded != 0);
        while (true) {
            visited.clear();
            list<mlcore::State*> stateStack;
            stateStack.push_back(s0);
            double error = 0.0;
            while (!stateStack.empty()) {
                if (timeHasRunOut(startingPlanningTime_, maxPlanningTime_))
                    return;
                mlcore::State* s = stateStack.back();
                stateStack.pop_back();
                if (!visited.insert(s).second)
                    continue;
                if (s->deadEnd() || problem_->goal(s))
                    continue;
                mlcore::Action* prevAction = s->bestAction();
                if (prevAction == nullptr) {
                    // if it reaches this point it hasn't converged yet.
                    error = mdplib::dead_end_cost + 1;
                } else {
                    mlreduced::ReducedState* reducedState =
                        (mlreduced::ReducedState* ) s;
                    for (Successor sccr : problem_->transition(s, prevAction)) {
                        if (!(useFF_ && reducedState->exceptionCount() == 0))
                            stateStack.push_back(sccr.su_state);
                    }
                }
                error = std::max(error, this->bellmanUpdate(s));
                if (prevAction != s->bestAction()) {
                    // it hasn't converged because the best action changed.
                    error = mdplib::dead_end_cost + 1;
                    break;
                }
            }
            if (error < epsilon_)
                return;
            if (error > mdplib::dead_end_cost) {
                break;  // BPSG changed, must expand tip nodes again
            }
        }
    }
}


double FFReducedModelSolver::bellmanUpdate(mlcore::State* s)
{
    if (problem_->goal(s)) {
        s->setCost(0.0);
        for (mlcore::Action* a : problem_->actions()) {
            if (problem_->applicable(s, a)) {
                s->setBestAction(a);
                return 0.0;
            }
        }
    }

    mlreduced::ReducedState* reducedState =
        static_cast<mlreduced::ReducedState*> (s);
    if (useFF_ && reducedState->exceptionCount() == 0) {
        // For exceptionCount = 0 we just call FF.
        PPDDLState* ppddlState =
            static_cast<PPDDLState*> (reducedState->originalState());
        string stateAtoms = extractStateAtoms(ppddlState);
        replaceInitStateInProblemFile(templateProblemFilename_,
                                      stateAtoms + removedInitAtoms_,
                                      currentProblemFilename_);
        mlcore::Action* stateFFAction;
        int stateFFCost;
        if (ffStateActions_.count(s)) {
            stateFFAction = ffStateActions_[s];
            stateFFCost = ffStateCosts_[s];
        } else {
            vector<string> fullPlan;
            pair<string, int> actionNameAndCost =
                getActionNameAndCostFromFF(ffExecFilename_,
                                           determinizedDomainFilename_,
                                           currentProblemFilename_,
                                           startingPlanningTime_,
                                           maxPlanningTime_,
                                           &fullPlan);

            // Extract policy
            mlcore::State* sPrime = s;
            mlppddl::PPDDLProblem* originalProblem =
                static_cast<mlppddl::PPDDLProblem*> (
                    static_cast<mlreduced::ReducedModel*> (problem_)->
                        originalProblem());
            int planLength = fullPlan.size();
            for (string actionName : fullPlan) {
                mlcore::Action* action =
                    originalProblem->getActionFromName(actionName);
                ffStateActions_[sPrime] = action;
                ffStateCosts_[sPrime] = planLength;
                sPrime->setCost(planLength);
                sPrime->setBestAction(action);
                if (action == nullptr) {
                    sPrime->setCost(mdplib::dead_end_cost);
                    sPrime->markDeadEnd();
                    continue;
                }
                int cnt = 0;
                for (auto const succ : problem_->transition(sPrime, action)) {
                    sPrime = succ.su_state;
                    cnt++;
                }
                planLength--;
                assert(cnt <= 1);
            }
        }
        return 0.0;
    }
    std::pair<double, mlcore::Action*> best = bellmanBackup(problem_, s);
    double residual = s->cost() - best.bb_cost;

    if (s->deadEnd()) {
        s->setCost(mdplib::dead_end_cost);
        return 0.0;
    }

    s->setCost(best.bb_cost);
    s->setBestAction(best.bb_action);
    return fabs(residual);
}

}
