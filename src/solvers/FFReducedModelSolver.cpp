#include <fstream>
#include <sstream>

#include "../../include/solvers/FFReducedModelSolver.h"


using namespace mlcore;
using namespace mlppddl;
using namespace std;


namespace mlsolvers
{


string FFReducedModelSolver::extractStatePredicates(PPDDLState* state)
{
    ostringstream oss;
    oss << state;
    string stateStr = oss.str();
    string currentStatePredicates = "";
    for (int i = 0; i < stateStr.size(); i++) {
        if (stateStr[i] == ':') {
            i++;
            do {
                currentStatePredicates += stateStr[i];
            } while (stateStr[i++] != ')');
            currentStatePredicates += " ";
        }
    }
    return currentStatePredicates;
}


void FFReducedModelSolver::replaceInitStateInProblemFile(
    string currentStatePredicates)
{
    std::ifstream problemTemplateFile_;
    problemTemplateFile_.open(templateProblemFilename_, ifstream::in);
    string line;
    string newProblemText = "";
    if (problemTemplateFile_.is_open()) {
        while (getline(problemTemplateFile_, line)) {
            if (line.find("init") != string::npos) {
                line = "  (:init " + currentStatePredicates + ")";
            }
            newProblemText += line + "\n";
        }
        problemTemplateFile_.close();
    }
    ofstream newProblemFile;
    newProblemFile.open(currentProblemFilename_);
    newProblemFile << newProblemText;
    newProblemFile.close();
}


pair<string, int> FFReducedModelSolver::getActionNameAndCostFromFF()
{
    string ffDomain = "-o " + determinizedDomainFilename_;
    string ffProblem = "-f " + currentProblemFilename_;
    string ffCommand = ffExecFilename_ + " " + ffDomain + " " + ffProblem;
//                                                                                dprint1(ffCommand);
    string actionName = "__mdplib-dead-end__";
    FILE *ff = popen(ffCommand.c_str(), "r");
    int costFF = floor(mdplib::dead_end_cost);
    if (ff) {
        char lineBuffer[1024];
        int currentLineAction = -1;
        while (fgets(lineBuffer, 1024, ff)) {
//                                                                                cerr << lineBuffer;
            if (strstr(lineBuffer, "goal can be simplified to FALSE.") !=
                    nullptr) {
                break;
            }

            if (strstr(lineBuffer, "step") != nullptr) {
                actionName = "";
                char *pch = strstr(lineBuffer, "0:");
                if (pch == nullptr)
                    continue;
                pch += 3;
                actionName += pch;
                actionName = actionName.substr(0, actionName.size() - 1);
                currentLineAction = 0;
            } else if (currentLineAction != -1) {
                currentLineAction++;
                ostringstream oss("");
                oss << currentLineAction << ":";
                char *pch = strstr(lineBuffer, oss.str().c_str());
                if (pch == nullptr) {
                    costFF = currentLineAction;
                    currentLineAction = -1;
                }
            }
        }
        pclose(ff);
    } else {
        cerr << "Couldn't open FF at " << ffCommand.c_str() << endl;
        exit(-1);
    }
    for (int i = 0; i < actionName.size(); i++) {
        actionName[i] = tolower(actionName[i]);
    }
//                                                                                dprint4("CostFF", costFF, "action", actionName);
    return make_pair(actionName, costFF);
}


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


mlcore::Action*
FFReducedModelSolver::greedyAction_(mlcore::State* s, int horizon)
{
    double qBestAction = mdplib::dead_end_cost + 1;
    mlcore::Action* bestAction = nullptr;
                                                                                bool check = false;
    for (mlcore::Action* a : problem_->actions()) {
        if (!problem_->applicable(s, a))
            continue;
//                                                                                check = true;
//                                                                                dprint2("  APPLICABLE", a);
//                                                                                mdplib_debug = false;
        double qAction = this->qValue_(s, a, horizon);
//                                                                                mdplib_debug = true;
//                                                                                dprint2("  q-action", qAction);
        if (qAction <= qBestAction) {
//                                                                                dprint2("  BETTER", a);
            qBestAction = qAction;
            bestAction = a;
        }
    }
//                                                                                if (!check) {
//                                                                                    dprint1("OMG!!! DEADEND!!");
//                                                                                }
    return bestAction;
}


double
FFReducedModelSolver::qValue_(mlcore::State* s, mlcore::Action* a, int horizon)
{
//                                                                                dprint4("  Q-VALUE", s, a, horizon);
//                                                                                assert(horizon > 0);
    double qv = 0.0;
    for (auto const & successor : problem_->transition(s, a)) {
        qv += successor.su_prob * estimatedCosts_[horizon - 1][successor.su_state];
//                                                                                dprint4("    ",
//                                                                                        successor.su_state,
//                                                                                        successor.su_prob,
//                                                                                        estimatedCosts_[horizon - 1][successor.su_state]);
    }
    return problem_->cost(s, a) + problem_->gamma() * qv;
}


double
FFReducedModelSolver::solve(mlcore::State* s, int horizon, bool& isDeadEnd)
{
                                                                                for(int i = 0; i < 2 * (maxHorizon_ - horizon); i++) cerr << " ";
                                                                                dprint3("SOLVING", s, horizon);
    if (horizon == 0) {
        // For horizon = 0 we just call FF.
        string statePredicates = extractStatePredicates((PPDDLState*) s);
        replaceInitStateInProblemFile(statePredicates);
        mlcore::Action* stateFFAction;
        int stateFFCost;
        if (ffStateActions_.count(s)) {
            stateFFAction = ffStateActions_[s];
            stateFFCost = ffStateCosts_[s];
        } else {
            pair<string, int> actionNameAndCost = getActionNameAndCostFromFF();
            stateFFAction = getActionFromName(actionNameAndCost.first);
            ffStateActions_[s] = stateFFAction;
            ffStateCosts_[s] = actionNameAndCost.second;
            estimatedCosts_[0][s] = actionNameAndCost.second;
        }
        isDeadEnd = (stateFFAction == nullptr);
                                                                                for(int i = 0; i < 2 * (maxHorizon_ - horizon); i++) cerr << " ";
                                                                                dprint2("RESULT", estimatedCosts_[0][s]);
        return 0.0;
    } else {
        double maxResidual = 0.0;
        mlcore::Action* action = this->greedyAction_(s, horizon);

        if (action == nullptr) {
            // This state is a dead-end.
            isDeadEnd = true;
            estimatedCosts_[horizon][s] = mdplib::dead_end_cost;
            return 0.0;
        }
                                                                                for(int i = 0; i < 2 * (maxHorizon_ - horizon); i++) cerr << " ";
                                                                                dprint2("BEST ACTION BEFORE", action);
                                                                                dprint1((void *) action);
        bool newIsDeadEnd = true;
        for (auto const & successor : problem_->transition(s, action)) {
            maxResidual = std::max(maxResidual,
                                   solve(successor.su_state,
                                         horizon - 1,
                                         isDeadEnd));
            newIsDeadEnd &= isDeadEnd;

        }
        isDeadEnd = newIsDeadEnd;

        // Computing the new best action.
        mlcore::Action* prevAction = action;
        action = this->greedyAction_(s, horizon);
                                                                                for(int i = 0; i < 2 * (maxHorizon_ - horizon); i++) cerr << " ";
                                                                                dprint2("BEST ACTION AFTER", action);

        // Computing the new estimated cost of this state.
        double newCost = std::min(this->qValue_(s, action, horizon),
                                   mdplib::dead_end_cost);
        double residual = fabs(estimatedCosts_[horizon][s] - newCost);
        estimatedCosts_[horizon][s] = newCost;

        if (prevAction != action) {
            // The best action changed, can't allow convergence.
            residual = mdplib::dead_end_cost;
        }
                                                                                for(int i = 0; i < 2 * (maxHorizon_ - horizon); i++) cerr << " ";
                                                                                dprint2("RESULT", estimatedCosts_[horizon][s]);
                                                                                for(int i = 0; i < 2 * (maxHorizon_ - horizon); i++) cerr << " ";
                                                                                dprint3("RESIDUAL", residual, isDeadEnd);
        return residual;
    }

}


mlcore::Action* FFReducedModelSolver::solve(mlcore::State* s0)
{
                                                                                dprint2("xxxxxxxxxxxx", s0);
    double residual = mdplib::dead_end_cost;
    while (residual > 1.0e-3) {
        bool isDeadEnd = true;
        residual = solve(s0, maxHorizon_, isDeadEnd);
                                                                                dprint1(estimatedCosts_[maxHorizon_][s0]);
    }
    return this->greedyAction_(s0, maxHorizon_);
}

}
