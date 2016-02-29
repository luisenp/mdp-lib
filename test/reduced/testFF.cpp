#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <list>
#include <sstream>
#include <string>
#include <typeinfo>


#include "../include/State.h"

#include "../include/solvers/Solver.h"

#include "../include/ppddl/mini-gpt/states.h"
#include "../include/ppddl/mini-gpt/problems.h"
#include "../include/ppddl/mini-gpt/domains.h"
#include "../include/ppddl/mini-gpt/states.h"
#include "../include/ppddl/mini-gpt/exceptions.h"
#include "../../include/ppddl/PPDDLHeuristic.h"
#include "../../include/ppddl/PPDDLProblem.h"

#include "../../include/util/flags.h"
#include "../../include/util/general.h"

#include "../../include/Heuristic.h"
#include "../../include/Problem.h"


using namespace std;
using namespace mdplib;
using namespace mlsolvers;
using namespace mlppddl;


extern int yyparse();
extern FILE* yyin;
string current_file;
int warning_level = 0;

static int verbosity = 0;
static int k = 0;

mlcore::Problem* problem = nullptr;
mlcore::Heuristic* heuristic = nullptr;

ifstream problemTemplateFile;
string templateFilename = "/home/lpineda/Desktop/problem_template.pddl";
string problemFilename = "/home/lpineda/Desktop/problem.pddl";

string ffExec = "/home/lpineda/Desktop/FF-v2.3/ff";
string ffDomain = "-o /home/lpineda/Desktop/domain.pddl";
string ffProblem = "-f /home/lpineda/Desktop/problem.pddl";
string ffCommand = ffExec + " " + ffDomain + " " + ffProblem;


/*
 * Extracts a string with the predicates in the given state.
 */
string extractStatePredicates(mlcore::State* state)
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


/*
 * Replaces the current state in the problem file.
 */
void replaceInitStateInProblemFile(string currentStatePredicates)
{
    problemTemplateFile.open(templateFilename);
    string line;
    string newProblemText = "";
    if (problemTemplateFile.is_open()) {
        while (getline(problemTemplateFile, line)) {
            if (line.find("init") != string::npos) {
                line = "  (:init " + currentStatePredicates + ")";
            }
            newProblemText += line + "\n";
        }
        problemTemplateFile.close();
    }

    ofstream newProblemFile;
    newProblemFile.open(problemFilename);
    newProblemFile << newProblemText;
    newProblemFile.close();
}


/*
 * Calls the FF planner and gets an action for the current state.
 */
string getActionNameFromFFPlanner()
{
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
    }
    for (int i = 0; i < actionName.size(); i++) {
        actionName[i] = tolower(actionName[i]);
    }
    return actionName;
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


/*
 * Returns the action with the given name.
 */
mlcore::Action* getActionFromName(string actionName) {
    ostringstream oss;
    for (mlcore::Action* a : problem->actions()) {
        oss.str("");
        oss.clear();
        oss << a;
        if (oss.str().find(actionName) != string::npos)
            return a;
    }
    return nullptr;
}


/*
 * Creates a PPDDL problem from a string describing the domain and problem file.
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
    heuristic =
        new mlppddl::PPDDLHeuristic(static_cast<PPDDLProblem*>(problem),
                                     mlppddl::FF);
    problem->setHeuristic(heuristic);
}


int main(int argc, char* args[])
{
    mdplib_debug = true;
    register_flags(argc, args);

    assert(flag_is_registered_with_value("problem"));
    string ppddlArgs = flag_value("problem");
//    if (flag_is_registered_with_value("v"))
//        verbosity = stoi(flag_value("v"));

    initPPDDL(ppddlArgs);

    mlcore::StateActionMap stateActions;

    mlcore::State* currentState =  problem->initialState();
    double cost = 0.0;
    while (true) {
        if (problem->goal(currentState))
            break;
        string currentStatePredicates = extractStatePredicates(currentState);
                                                                                dprint1(currentStatePredicates);
        replaceInitStateInProblemFile(currentStatePredicates);
        mlcore::Action* action;
        if (stateActions.count(currentState)) {
            action = stateActions[currentState];
        } else {
            string actionName = getActionNameFromFFPlanner();
            action = getActionFromName(actionName);
            stateActions[currentState] = action;
        }
        currentState = randomSuccessor(problem, currentState, action);
        cost += problem->cost(currentState, action);
                                                                                dprint1(action);
    }
    dprint1(cost);

    delete problem;
    return 0;
}
