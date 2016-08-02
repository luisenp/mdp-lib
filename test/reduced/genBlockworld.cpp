#include <cassert>
#include <ctime>
#include <list>
#include <sstream>
#include <string>
#include <typeinfo>

#include "../../include/domains/DummyState.h"
#include "../../include/domains/WrapperProblem.h"

#include "../../include/ppddl/mini-gpt/states.h"
#include "../../include/ppddl/mini-gpt/problems.h"
#include "../../include/ppddl/mini-gpt/domains.h"
#include "../../include/ppddl/mini-gpt/states.h"
#include "../../include/ppddl/mini-gpt/exceptions.h"
#include "../../include/ppddl/PPDDLHeuristic.h"
#include "../../include/ppddl/PPDDLProblem.h"

#include "../../include/solvers/LAOStarSolver.h"
#include "../../include/solvers/Solver.h"
#include "../../include/solvers/VISolver.h"

#include "../../include/util/flags.h"
#include "../../include/util/general.h"

#include "../../include/Problem.h"

using namespace std;
using namespace mdplib;
using namespace mlppddl;
using namespace mlsolvers;


extern int yyparse();
extern FILE* yyin;
string current_file;
int warning_level = 0;

static int verbosity = 0;
static int k = 0;

mlcore::Problem* problem = nullptr;
mlcore::Heuristic* heuristic = nullptr;

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
}




mlcore::State* getBestSucc(mlcore::Problem* problem,
                               mlcore::State* s,
                               mlcore::Action* a)
{
double min = 10000.0;
mlcore::State* to_ret;
	for (mlcore::Successor sccr : problem->transition(s, a)) {
        if (sccr.su_prob >0 && sccr.su_state->cost() <min)
	{
		min  = sccr.su_state->cost();
		to_ret = sccr.su_state;
	}
    }
  return to_ret;
}


int main(int argc, char* args[])
{
    register_flags(argc, args);
	
	bool print_states = true;
	bool print_actions = false;
	bool print_trans = false;
    // Reading flags.
    assert(flag_is_registered_with_value("domain"));
    string domainName = flag_value("domain");

    assert(flag_is_registered_with_value("problem"));

    if (flag_is_registered_with_value("v"))
       verbosity = stoi(flag_value("v"));


    if (flag_is_registered_with_value("k"))
        k = stoi(flag_value("k"));

    // Creating problem
   if (domainName == "ppddl") {
        string ppddlArgs = flag_value("problem");

        initPPDDL(ppddlArgs);
   }
 mlcore::StateSet reachableStates, tipStates;

// generate all states : 
	problem->generateAll();

/******** PRINT THE PROBLEM *******************/
if(print_states){	
int states_count =0 ;
cout<<"printing states : \n";
for (mlcore::State* s : problem->states()) {
	states_count++;
}
cout<<"number of states = "<< states_count <<"\n";
}
int action_count =0;
if(print_actions){
cout<<"\n printing actios : \n";
for (mlcore::Action* a : problem->actions()) {
	action_count++;
}
	cout<<"number of actions = " <<action_count <<"\n";
}

if(print_trans){
int trans_count =0 ;
cout<<"\n printing transitions : \n";
for (mlcore::State* s : problem->states()) {
for(mlcore::Action* a: problem->actions()){
if(problem->applicable(s, a)){
 for (mlcore::Successor su : problem->transition(s, a)) {
	trans_count++;
}
}
}
}
	cout<<"trans count = "<< trans_count <<"\n";
}

//solving using lao*

/*    double totalPlanningTime = 0.0;
    clock_t startTime = clock();
    LAOStarSolver solver(problem);
    solver.solve(problem->initialState());
    clock_t endTime = clock();
    totalPlanningTime += (double(endTime - startTime) / CLOCKS_PER_SEC);
    cout << "cost " << problem->initialState()->cost() <<
        " time " << totalPlanningTime << endl;
	cout<<"Printing best action:\n";
	for (mlcore::State* s : problem->states()) {
		if(s->bestAction() != nullptr)
		//cout<<s <<": has no best action\n";
		//else
		cout<<""<<s<< " : "<< s->bestAction() <<"\n";
} */



// solving using VI
 /*double totalPlanningTime = 0.0;
    clock_t startTime = clock();
   VISolver vi(problem);
   vi.solve(problem->initialState());
   clock_t endTime = clock();
    totalPlanningTime += (double(endTime - startTime) / CLOCKS_PER_SEC);
    cout << "cost " << problem->initialState()->cost() <<
        " time " << totalPlanningTime << endl;
   for (mlcore::State* s : problem->states()) {
	std::cout<<""<<s<<":"<<s->cost() <<"\n"; } */
	

/*	 FILE * pFile;
 	pFile = fopen ("blocksworld_cost.txt","w");
  	if (pFile +=NULL)
 	 {
 	  cout <<"error in file opening \n";
 	 } */
//solve using ACAD
 double totalPlanningTime = 0.0;
    clock_t startTime = clock();
	VISolver vi(problem,100000000, 1.0e-6);
   	vi.solve(problem->initialState());
	cout<<"VI Solved \n" << "***********************\n";  
	
	cout<<"Printing cost adjustment:\n";
	int max_count = 0;
	for (mlcore::State* s : problem->states()) {
		max_count++;
		if(max_count <= 500) {
		double bestCost = 0;
		int min = 100000.0;
		double bestQ =0;
		mlcore::Action* bestAction ;
		for (mlcore::Action* a : problem->actions()) {
		if(problem->applicable(s,a)){
		double qAction = std::min(10000.0, qvalue(problem, s, a));
		if(qAction <= min) {
			min = qAction;
			bestAction = a;
			bestQ = qAction;
		}
		}
		}
		mlcore::State* j = getBestSucc(problem, s, bestAction);
		double bestVal = j->cost();
		double new_cost = bestQ- bestVal;
		cout<<""<<s << "best a"<< bestAction <<" : cost = "<< new_cost <<"\n"; 
		//std::string to_write = ""+s+"  , best action =   "+ bestAction+", new cost = "+new_cost+"\n";
		//fputs (to_write,pFile);
 	}
	} 
//fclose(pFile);
/*	cout<<"Printing cost adjustment:\n";
	for (mlcore::State* s : problem->states()) {
		index++;
		for (mlcore::Action* a : problem->actions()) {
		if(problem->applicable(s,a)){
		double qAction = std::min(10000.0, qvalue(problem, s, a));
		mlcore::State* j = getBestSucc(problem, s, a);
		double bestVal = j->cost();
		double new_cost = qAction- bestVal;
		cout<<""<<s << " a = "<< a <<" : cost = "<< new_cost <<"\n"; 	
 		}
		}
	}
cout <<"index = "<< index <<"\n";*/

/////////////////////////////////////////////////

      // Releasing memory
  
    delete problem;
    return 0;
}
