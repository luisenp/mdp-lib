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

mlcore::Action* FFReducedModelSolver::getActionFromName(string actionName)
{
    ostringstream oss;
    for (mlcore::Action* a : problem_->actions()) {
        oss.str("");
        oss.clear();
        oss << a;
        if (oss.str().find(actionName) != string::npos)
            return a;
    }
    return nullptr;
}


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
                    for (Successor sccr : problem_->transition(s, a)) {
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
                    for (Successor sccr : problem_->transition(s, prevAction)) {
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

    mlreduced::ReducedState* reducedState = (mlreduced::ReducedState* ) s;
    if (useFF_ && reducedState->exceptionCount() == maxHorizon_) {
        // For exceptionCount = k we just call FF.
        PPDDLState* pState =
            static_cast<PPDDLState*> (reducedState->originalState());
        string stateAtoms = extractStateAtoms(
            static_cast<PPDDLState*> (pState));
        replaceInitStateInProblemFile(templateProblemFilename_,
                                      stateAtoms + removedInitAtoms_,
                                      currentProblemFilename_);
        mlcore::Action* stateFFAction;
        int stateFFCost;
        if (ffStateActions_.count(s)) {
            stateFFAction = ffStateActions_[s];
            stateFFCost = ffStateCosts_[s];
        } else {
            pair<string, int> actionNameAndCost =
                getActionNameAndCostFromFF(ffExecFilename_,
                                           determinizedDomainFilename_,
                                           currentProblemFilename_,
                                           startingPlanningTime_,
                                           maxPlanningTime_);

            // If FF finds this state is a dead-end,
            // getActionNameAndCostFromFF() returns "__mdplib-dead-end__"
            // and getActionFromName() returns a nullptr.
            stateFFAction = getActionFromName(actionNameAndCost.first);
            if (stateFFAction == nullptr) {
                s->markDeadEnd();
            }
            ffStateActions_[s] = stateFFAction;
            ffStateCosts_[s] = actionNameAndCost.second;
        }
        s->setCost(ffStateCosts_[s]);
        s->setBestAction(ffStateActions_[s]);
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
