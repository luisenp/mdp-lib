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


string FFReducedModelSolver::getActionNameFromFFPlanner()
{
    string ffDomain = "-o " + determinizedDomainFilename_;
    string ffProblem = "-f " + currentProblemFilename_;
    string ffCommand = ffExecFilename_ + " " + ffDomain + " " + ffProblem;
                                                                                dprint1(ffCommand);
    string actionName = "";
    FILE *ff = popen(ffCommand.c_str(), "r");
    double costFF = mdplib::dead_end_cost;
    if (ff) {
        char lineBuffer[1024];
        int currentLineAction = -1;
        while (fgets(lineBuffer, 1024, ff)) {
                                                                                cerr << lineBuffer;
            if (strstr(lineBuffer, "step") != nullptr) {
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
                                                                                dprint2("CostFF", costFF);
    for (int i = 0; i < actionName.size(); i++) {
        actionName[i] = tolower(actionName[i]);
    }
    return actionName;
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


mlcore::Action* FFReducedModelSolver::solve(mlcore::State* s0)
{
                                                                                dprint2("**** Solving", s0);
    string currentStatePredicates = extractStatePredicates((PPDDLState*) s0);
    replaceInitStateInProblemFile(currentStatePredicates);
    mlcore::Action* action;
    if (stateActions_.count(s0)) {
        action = stateActions_[s0];
    } else {
        string actionName = getActionNameFromFFPlanner();
        action = getActionFromName(actionName);
        stateActions_[s0] = action;
    }
    return action;
}

}
