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
    if (ff) {
        char lineBuffer[1024];
        while (fgets(lineBuffer, 1024, ff)) {
            if (strstr(lineBuffer, "step") != nullptr) {
                char *pch = strstr(lineBuffer, "0:") + 3;
                actionName += pch;
                actionName = actionName.substr(0, actionName.size() - 1);
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

}
