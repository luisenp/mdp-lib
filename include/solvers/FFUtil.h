#ifndef MDPLIB_FFUTIL_H
#define MDPLIB_FFUTIL_H

/**
 * This file provides some functions that are useful to interact with the
 * FF planner.
 */

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cmath>
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
#include <unordered_set>

#include "../ppddl/PPDDLProblem.h"
#include "../ppddl/PPDDLState.h"


namespace mlsolvers
{


/*
 * Checks the init state in the given PPDDL problem filename and stores the
 * atoms that are are not part of the given PPDDL problem object
 * (problem_t type) atom_hash.
 * The set of atoms is returned as a string.
 */
inline std::string storeRemovedInitAtoms(
    std::string problemFilename, mlppddl::PPDDLProblem* problem)
{
    // Storing all the atoms present in the initial state
    std::ifstream problemFile;
    problemFile.open(problemFilename, std::ifstream::in);
    std::string line;
    std::unordered_set<std::string> initAtoms;
    if (problemFile.is_open()) {
        while (getline(problemFile, line)) {
            size_t idx = line.find("init");
            if (idx != std::string::npos) {
                for (int i = idx + 4; i < line.size(); i++) {
                    if (line[i] == '(') {
                        std::string atom = "";
                        do {
                            atom += line[i];
                        } while (line[i++] != ')');
                        initAtoms.insert(atom);
                        i--;
                    }
                }
            }
        }
        problemFile.close();
    }

    // Figuring out which atoms were removed from the PPDDL parser
    problem_t* pProblem = problem->pProblem();
    const Domain& dom = pProblem->domain();
    const PredicateTable& preds = dom.predicates();
    TermTable& terms = pProblem->terms();
    for (auto const & atom : problem_t::atom_hash()) {
        std::ostringstream oss;
        atom.first->print(oss, preds, dom.functions(), terms);
        if (initAtoms.find(oss.str()) != initAtoms.end())
            initAtoms.erase(oss.str());
    }

    // Storing the removed atoms
    std::string removedInitAtoms = "";
    for (std::string atom : initAtoms)
        removedInitAtoms += atom + " ";
    return removedInitAtoms;
}


/**
 * Extracts the atoms in the given PPDDL state and returns them as a string.
 */
inline std::string extractStateAtoms(mlppddl::PPDDLState* state)
{
    std::ostringstream oss;
    oss << state;
    std::string stateStr = oss.str();
    std::string atomsCurrentState = "";
    for (int i = 0; i < stateStr.size(); i++) {
        // The format of a PPDDLState "tostring" conversion is
        // [ atom_1_Id:(atom_1),
        //   atom_2_Id:(atom_2),
        //   ...,
        //  atom_N_Id:(atom_N) ]
        if (stateStr[i] == ':') {
            i++;
            do {
                atomsCurrentState += stateStr[i];
            } while (stateStr[i++] != ')');
            atomsCurrentState += " ";
        }
    }
    return atomsCurrentState;
}


/**
 * Extracts the atoms that doesn't hold in the given PPDDL state
 * and returns them as a string.
 */
inline std::string extractAtomsNotInState(mlppddl::PPDDLState* state,
                                          mlppddl::PPDDLProblem* problem)
{
    problem_t* pProblem = problem->pProblem();
    const Domain& dom = pProblem->domain();
    const PredicateTable& preds = dom.predicates();
    TermTable& terms = pProblem->terms();
    std::string atomsNotCurrentState = "";
    for (auto const & atom : problem_t::atom_hash()) {
        if (!state->pState()->holds(*atom.first)) {
            std::ostringstream oss;
            atom.first->print(oss, preds, dom.functions(), terms);
            atomsNotCurrentState += "(not " + oss.str() + ") ";
        }
    }
    return atomsNotCurrentState;
}


/**
 * Replaces the initial state in the given template PPDDL filename
 * with the given state atoms. Then writes the result to the given
 * output file.
 *
 * This function assumes the original initial state was described in a single
 * line.
 */
inline void replaceInitStateInProblemFile(
    std::string templateProblemFilename_,
    std::string atomsCurrentState,
    std::string outputProblemFile)
{
    std::ifstream problemTemplateFile;
    problemTemplateFile.open(templateProblemFilename_, std::ifstream::in);
    std::string line;
    std::string newProblemText = "";
    if (problemTemplateFile.is_open()) {
        while (getline(problemTemplateFile, line)) {
            if (line.find("init") != std::string::npos) {
                line = "(:init " + atomsCurrentState + ")";
            }
            newProblemText += line + "\n";
        }
        problemTemplateFile.close();
    }
    std::ofstream os;
    os.open(outputProblemFile);
    os << newProblemText;
    os.close();
}


/**
 * Replaces the goal state in the given template PPDDL filename with additional
 * sub-goal states. Then writes the result to the given output file.
 *
 * This function assumes the original goal state was described in a single
 * line.
 *
 * This functions assumes that the (:goal  ) list in the PPDDL file is described
 * in its own line, without any extra parenthesis at the end (such as the
 * end of the (define ) list.
 */
inline void addSubGoalsToProblemFile(
    std::string templateProblemFilename,
    std::vector<mlcore::State*> additionalGoals,
    mlppddl::PPDDLProblem* problem,
    std::string outputProblemFile)
{
    std::ifstream problemTemplateFile;
    problemTemplateFile.open(templateProblemFilename, std::ifstream::in);
    std::string line;
    std::string newProblemText = "";
    if (problemTemplateFile.is_open()) {
        while (getline(problemTemplateFile, line)) {
            size_t idxGoalText = line.find("goal");
            if (idxGoalText != std::string::npos) {
                std::string newLine = "(:goal (or ";
                // Adding original goal state
                size_t idxStartAtoms = line.find('(', idxGoalText + 4);
                size_t idxEndAtoms = -1;
                bool firstCloseParens = true;
                // The goal state is described as "(:goal (atom1) (atom2) ).
                // Must find the ')' character before the one that closes
                // the goal list
                int parenCount = 0;
                bool firstOpenParen = true;
                for (size_t i = 0; i < line.size(); i++) {
                    if (line[i] == '(') {
                        if (parenCount == 0)
                            firstOpenParen = false;
                        parenCount++;
                    }
                    else if (line[i] == ')') {
                        parenCount--;
                        if (parenCount == 0 && !firstOpenParen)
                            break;
                        idxEndAtoms = i;
                    }
                }
                // These add the original atoms in the goal state as a new
                // subgoal
                newLine += "(and " +
                    line.substr(idxStartAtoms,
                                idxEndAtoms - idxStartAtoms + 1) + ')';
                // Adding additional states
                for (auto const state : additionalGoals) {
                    std::string atoms =
                        extractStateAtoms(
                            static_cast<mlppddl::PPDDLState*> (state));
                    std::string atomsNot = extractAtomsNotInState(
                            static_cast<mlppddl::PPDDLState*> (state),
                            problem);
                    newLine += " (and " + atoms + " " + atomsNot + ')';
                }
                newLine += "))";
                line = newLine;
            }
            newProblemText += line + "\n";
        }
        problemTemplateFile.close();
    }
    std::ofstream newProblemFile;
    newProblemFile.open(outputProblemFile);
    newProblemFile << newProblemText;
    newProblemFile.close();
}


/* Handler for the child process running FF. */
static void sigchld_hdl(int sig)
{
    // the main process will handle the child, no need to do anything
}


/**
 * Runs the FF planner and returns the action name and cost.
 *
 * @param ffExecFilename The name of the executable file for FF.
 * @param determinizedDomainFilename The name of the file where the
 *                                   determinized PPDDL domain is stored.
 * @param currentProblemFilename The name of the file where the PPDDL problem
 *                               is stored.
 * @param startingPlanningTime The time stamp at which the time for planning
 *                             started. Used if a planning limit is desired.
 * @param maxPlanningTime The maximum time allowed for planning, counting from
 *                        startingPlanningTime.
 * @param fullPlan An array to store the full plan computed by FF. If a nullptr
 *                 is passed (default value), then this will be ignored.
 *
 * @return A pair storing the action name and the computed cost.
 */
inline std::pair<std::string, int> getActionNameAndCostFromFF(
    std::string ffExecFilename,
    std::string determinizedDomainFilename,
    std::string currentProblemFilename,
    int startingPlanningTime = 0,
    int maxPlanningTime = 1000000,
    std::vector<std::string>* fullPlan = nullptr)
{
    pid_t child_pid;
    int fds[2];
    int pipe_ret = pipe(fds);
    if (pipe_ret != 0) {
        std::cerr << "Error creating pipe for FF: " <<
            strerror(errno) << std::endl;
        exit(-1);
    }

    std::string actionName = "__mdplib-dead-end__";
    int costFF = floor(mdplib::dead_end_cost);
    time_t timeLeft = 0.0;
    if (timeHasRunOut(startingPlanningTime, maxPlanningTime, &timeLeft)) {
        return std::make_pair(actionName, costFF);
    }

    // setting a handler for the FF child process
    struct sigaction act;
    memset (&act, 0, sizeof(act));
    act.sa_handler = sigchld_hdl;
    if (sigaction(SIGCHLD, &act, 0) == -1) {
        std::cerr << "Error setting handler for FF process signal" << std::endl;
        exit(-1);
    }

    child_pid = fork();
    if (child_pid != 0) {   // parent process (process FF output)
        close(fds[1]);
        int status;
        while (timeLeft > 0) {  // TODO: improve this ugly code hack
            timeHasRunOut(startingPlanningTime, maxPlanningTime, &timeLeft);
            pid_t wait_result = waitpid(child_pid, &status, WNOHANG);
            if (wait_result == -1) {
                std::cerr << "Error ocurred during call to FF: " <<
                    strerror(errno) << std::endl;
                exit(-1);
            } else if (wait_result == 0) {  // FF still running
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            } else {    // FF finished
                break;
            }
        }
        kill(child_pid, SIGTERM); // seems to be safe to use on child processes
        pid_t wait_result = waitpid(child_pid, &status, 0);
        FILE* ff_output = fdopen(fds[0], "r");
        if (ff_output) {
            char lineBuffer[1024];
            // used to check if FF is still returning actions
            // (since they are numbered).
            int currentLineAction = -1;
            while (fgets(lineBuffer, 1024, ff_output)) {
                                                                                std::cerr << lineBuffer;
                if (strstr(lineBuffer, "goal can be simplified to FALSE.") !=
                        nullptr) {
                    if (fullPlan)
                        fullPlan->push_back(actionName);
                    break;  // At this point actionName = "__mdplib-dead-end__"
                }
                if (strstr(lineBuffer, "problem proven unsolvable.") !=
                        nullptr) {
                    if (fullPlan)
                        fullPlan->push_back(actionName);
                    break;  // At this point actionName = "__mdplib-dead-end__"
                }
                if (strstr(lineBuffer, "step") != nullptr) {
                    actionName = "";
                    char *pch = strstr(lineBuffer, "0:");
                    if (pch == nullptr)
                        continue;
                    pch += 3;
                    actionName += pch;
                    // removing the line break character at the end.
                    actionName = actionName.substr(0, actionName.size() - 1);
                    transform(actionName.begin(),
                              actionName.end(),
                              actionName.begin(),
                              ::tolower);
                    if (fullPlan)
                        fullPlan->push_back(actionName);
                    currentLineAction = 0;
                } else if (currentLineAction != -1) {
                    currentLineAction++;
                    std::ostringstream oss("");
                    oss << currentLineAction << ":";
                    char *pch = strstr(lineBuffer, oss.str().c_str());
                    if (pch == nullptr) {
                        costFF = currentLineAction;
                        currentLineAction = -1;
                    } else if (fullPlan) {
                        // ignore the step number
                        // (size = number of digits + 2 characters)
                        pch += int(log10(currentLineAction)) + 1 + 2;
                        std::string thisActionName = "";
                        thisActionName += pch;
                        // removing the line break character at the end.
                        thisActionName =
                            thisActionName.substr(0, thisActionName.size() - 1);
                        transform(thisActionName.begin(),
                                  thisActionName.end(),
                                  thisActionName.begin(),
                                  ::tolower);
                        fullPlan->push_back(thisActionName);
                    }
                }
            }
            pclose(ff_output);
        } else {
            std::cerr << "Error reading the output of FF." << std::endl;
            exit(-1);
        }
        return std::make_pair(actionName, costFF);
    } else {    // child process (the one that calls FF)
        close(fds[0]);
        dup2(fds[1], STDOUT_FILENO);
        const char* ff_args[] = {
            "ff",
            "-o",
            determinizedDomainFilename.c_str(),
            "-f",
            currentProblemFilename.c_str(),
            NULL
        };
        execvp(ffExecFilename.c_str(), const_cast<char**> (ff_args));
        std::cerr << "An error ocurred while calling FF: " <<
            strerror(errno) << std::endl;
        abort();
    }
}


};

#endif // MDPLIB_FFUTIL_H
