#include "../../../include/solvers/mobj/CMDPSolver.h"

#include <algorithm>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

namespace mdplib_mobj_solvers
{

double CMDPSolver::solve(mlcore::State* s0)
{
    return solvePrimal(s0);
//    return CMDPSolver::solveDual(s0);
}

void CMDPSolver::createVariablesAndObjFunForPrimal(
    int numVariables, int numActions, GRBModel& model,
    GRBVar* variables, GRBLinExpr& objFun)
{
    for (mlcore::State* s : problem_->states()) {
        int idxAction = -1;
        int idxState = stateIndex_[s];
        for (mlcore::Action* a : problem_->actions()) {
            idxAction++;
            int varIdx = idxState * numActions + idxAction;
            std::ostringstream oss;
            oss << s << "_" << a;
            std::string name = oss.str();
            std::replace(name.begin(), name.end(), ' ', '_');
            variables[varIdx] =
                model.addVar(1.0e-8, GRB_INFINITY, 0.0, GRB_CONTINUOUS, name);
            if (!problem_->applicable(s, a)) {
                objFun += 100 * variables[varIdx];
            }
            else {
                objFun +=
                    problem_->cost(s, a, indexObjFun_) * variables[varIdx];
            }
        }
    }
    model.update();
    model.setObjective(objFun, GRB_MINIMIZE);
    model.update();
}

void CMDPSolver::createLPConstraintsLHSForPrimal(GRBVar* variables,
                                                 int numActions,
                                                 GRBLinExpr* MDPConstLHS,
                                                 GRBLinExpr* CMDPConstLHS)
{
    for (mlcore::State* s : problem_->states()) {
        int idxAction = -1;
        int idxState = stateIndex_[s];
        for (mlcore::Action* a : problem_->actions()) {
            idxAction++;
            if (!problem_->applicable(s, a))
                continue;
            int varIdx = idxState * numActions + idxAction;
            // setting up MDP constraints left-hand side
            MDPConstLHS[idxState] += variables[varIdx];
            for (mlcore::Successor su : problem_->transition(s, a)) {
                int idxSucc = stateIndex_[su.su_state];
                GRBLinExpr& aux2 = MDPConstLHS[idxSucc];
                aux2 -= problem_->gamma() * su.su_prob * variables[varIdx];
            }
            // setting up CMDP constraints left-hand side
            for (int i = 0; i < constTargets_.size(); i++) {
                GRBLinExpr& aux3 = CMDPConstLHS[i];
                aux3 +=
                    problem_->cost(s, a, constIndices_[i]) * variables[varIdx];
            }
        }
    }
}

void CMDPSolver::ComputePolicyFromPrimalSolution(
    GRBVar* variables, int numStates, int numActions)
{
    policy_ = new mlsolvers::RandomPolicy(problem_, numStates);
    for (mlcore::State* s : problem_->states()) {
        std::vector <double> actionProbs;
        int idxState = stateIndex_[s];
        double total = 0.0;
        int idxAction = -1;
        for (mlcore::Action* a : problem_->actions()) {
            idxAction++;
            if (!problem_->applicable(s, a)) {
                actionProbs.push_back(0.0);
                continue;
            }
            int varIdx = idxState * numActions + idxAction;
            double varValue = variables[varIdx].get(GRB_DoubleAttr_X);
            if (varValue < 0.0)
                varValue = 0.0;
            actionProbs.push_back(varValue);
            total += varValue;
        }

        idxAction = -1;
        for (mlcore::Action* a : problem_->actions()) {
            idxAction++;
            if (!problem_->applicable(s, a))
                continue;
            if (total == 0) {
                actionProbs[idxAction] = 0.0;
            }
            else {
                actionProbs[idxAction] /= total;
            }
        }
        policy_->addActionsState(s, actionProbs);
    }   // end build policy
}

double CMDPSolver::solvePrimal(mlcore::State* s0)
{
    problem_->generateAll();

    GRBEnv env = GRBEnv();
    GRBModel model = GRBModel(env);

    int idx = 0;

    for (mlcore::State* s : problem_->states())
        stateIndex_[s] = idx++;

    int numStates = idx;
    int numActions = problem_->actions().size();
    int numVariables = numStates * numActions;

    // Creating the variables and setting up objective function
    GRBVar* variables = new GRBVar[numVariables];
    GRBLinExpr objFun = 0.0;
    createVariablesAndObjFunForPrimal(numVariables,
                                      numActions,
                                      model,
                                      variables,
                                      objFun);

    // Creating LP constraints
    int K = constTargets_.size();
    GRBLinExpr* MDPConstLHS = new GRBLinExpr[numStates];
    GRBLinExpr* CMDPConstLHS = new GRBLinExpr[K];
    createLPConstraintsLHSForPrimal(variables,
                                    numActions,
                                    MDPConstLHS,
                                    CMDPConstLHS);

    // Adding the linear program constraints
    for (mlcore::State* s : problem_->states()) {
        int idxState = stateIndex_[s];
        double b = (s == problem_->initialState()) ? 1.0 : 0.0;
        GRBLinExpr rhs = /* (1 - problem_->gamma()) * */ b;
        std::ostringstream oss; oss << s;
        std::string name = oss.str();
        std::replace(name.begin(), name.end(), ' ', '_');
        model.addConstr(MDPConstLHS[idxState], GRB_EQUAL, rhs, name);
    }
    for (int i = 0; i < K; i++) {
        GRBLinExpr rhs = constTargets_[i];
        std::ostringstream oss;
        oss << "CMDP" << i;
        model.addConstr(CMDPConstLHS[i], GRB_LESS_EQUAL, rhs, oss.str());
    }
    model.update();

    try {
        model.write("file.lp");
    } catch(GRBException e) {
        std::cerr << "Error code = " << e.getErrorCode() << std::endl;
        std::cerr << e.getMessage() << std::endl;
    }

    // Solving the model
    model.getEnv().set(GRB_IntParam_Presolve, 2);
    model.getEnv().set(GRB_IntParam_Method, -1);
    model.getEnv().set(GRB_DoubleParam_FeasibilityTol, 1.0e-9);
    model.getEnv().set(GRB_IntParam_LogToConsole, 1);
    model.getEnv().set(GRB_IntParam_DisplayInterval, 100);
    model.optimize();

    // Extracting the policy
    ComputePolicyFromPrimalSolution(variables, numStates, numActions);

    delete[] variables;
    delete[] MDPConstLHS;
    delete[] CMDPConstLHS;

    return objFun.getValue();
}   // solve


double CMDPSolver::solveDual(mlcore::State* s0)
{
    problem_->generateAll();

    GRBEnv env = GRBEnv();
    GRBModel model = GRBModel(env);

    int idx = 0;
    for (mlcore::State* s : problem_->states())
        stateIndex_[s] = idx++;

    int numStates = idx;
    int numActions = problem_->actions().size();
    int K = constTargets_.size();
    int numVariables = numStates + K;

    // Creating the variables and setting up objective function
    GRBVar* variables = new GRBVar[numVariables];
    GRBLinExpr objFun = 0.0;
    for (mlcore::State* s : problem_->states()) {
        int idxState = stateIndex_[s];
        int varIdx = idxState;
        variables[varIdx] =
            model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
        if (s == problem_->initialState())
            objFun += variables[varIdx];
    }
    for (int i = 0; i < K; i++) {
        variables[numStates + i] =
            model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS);
        objFun -= constTargets_[i] * variables[numStates + i];
    }

    model.update();
    model.setObjective(objFun, GRB_MAXIMIZE);
    model.update();

    // Creating LP constraints
    int numConstraints = numStates * numActions;
    GRBLinExpr* MDPConstLHS = new GRBLinExpr[numConstraints];
    for (mlcore::State* s : problem_->states()) {
        int idxAction = -1;
        int idxState = stateIndex_[s];
        for (mlcore::Action* a : problem_->actions()) {
            idxAction++;
            if (!problem_->applicable(s, a))
                continue;

            // setting up MDP constraints left-hand side
            int idxConst = idxState * numActions + idxAction;
            GRBLinExpr& aux1 = MDPConstLHS[idxConst];
            aux1 += variables[idxState];
            for (mlcore::Successor su : problem_->transition(s, a)) {
                int idxSucc = stateIndex_[su.su_state];
                aux1 -= problem_->gamma() * su.su_prob * variables[idxSucc];
            }
            for (int i = 0; i < K; i++) {
                aux1 -=
                    problem_->cost(s, a, constIndices_[i]) *
                        variables[numStates + i];
            }

            GRBLinExpr rhs = problem_->cost(s, a, indexObjFun_);
            model.addConstr(MDPConstLHS[idxConst], GRB_LESS_EQUAL, rhs);
        }
    }
    model.update();

    try {
        model.write("file.lp");
    } catch(GRBException e) {
        std::cerr << "Error code = " << e.getErrorCode() << std::endl;
        std::cerr << e.getMessage() << std::endl;
    }

    // solving the model
    model.getEnv().set(GRB_IntParam_Method, 0);
    model.optimize();

    // Extracting the policy
    if (policy_ != nullptr)
        delete policy_;
    policy_ = new mlsolvers::RandomPolicy(problem_, numStates);
    for (mlcore::State* s : problem_->states()) {
        int idxState = stateIndex_[s];
        double bestCost = -1.0e10;
        mlcore::Action* bestAction = nullptr;
        std::vector <double> actionProbs(problem_->actions().size());
        int idxAction = -1;
        for (mlcore::Action* a : problem_->actions()) {
            idxAction++;
            if (!problem_->applicable(s, a))
                continue;
            int idxConst = idxState * numActions + idxAction;
            double qvalue = MDPConstLHS[idxConst].getValue();
            if (qvalue > bestCost || bestAction == nullptr) {
                bestAction = a;
                bestCost = qvalue;
            }
        }
        idxAction = -1;
        for (mlcore::Action* a : problem_->actions()) {
            idxAction++;
            if (!problem_->applicable(s, a))
                continue;
            actionProbs[idxAction] = (a == bestAction)? 1.0 : 0.0;
        }
        policy_->addActionsState(s, actionProbs);
    }   // end build policy

    delete[] variables;
    delete[] MDPConstLHS;

    return objFun.getValue();
}   // solve

} // namespace mlsolvers

