#include "../../include/solvers/CMDPLinProgSolver.h"

#include <string>
#include <ostream>
#include <sstream>
#include <iostream>

namespace mlsolvers
{

void CMDPLinProgSolver::solve(mlcore::State* s0)
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
    for (mlcore::State* s : problem_->states()) {
        int idxAction = -1;
        int idxState = stateIndex_[s];
        for (mlcore::Action* a : problem_->actions()) {
            idxAction++;
            if (!problem_->applicable(s, a))
                continue;

            int varIdx = idxState * numActions + idxAction;
            std::ostringstream oss;
            oss << s << "_" << a;
//            dprint5(s, " ", a, " ", varIdx);
            variables[varIdx] = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, oss.str());
            objFun += problem_->cost(s, a, 0) * variables[varIdx];
        }
    }
    model.update();
    model.setObjective(objFun, GRB_MINIMIZE);
    model.update();
//    dprint2("Objective Function ", objFun);

    // Creating LP constraints
    GRBLinExpr* MDPConstLHS = new GRBLinExpr[numStates];
    GRBLinExpr* CMDPConstLHS = new GRBLinExpr[problem_->size() - 1];
    for (mlcore::State* s : problem_->states()) {
        int idxAction = -1;
        int idxState = stateIndex_[s];
        for (mlcore::Action* a : problem_->actions()) {
            idxAction++;
            if (!problem_->applicable(s, a))
                continue;

            int varIdx = idxState * numActions + idxAction;

            // setting up MDP constraints left-hand side
            GRBLinExpr& aux1 = MDPConstLHS[idxState];
            aux1 += variables[varIdx];

            for (mlcore::Successor su : problem_->transition(s, a)) {
                int idxSucc = stateIndex_[su.su_state];
//                dprint3(varIdx, " ", idxSucc);
                GRBLinExpr& aux2 = MDPConstLHS[idxSucc];
                aux2 -= problem_->gamma() * su.su_prob * variables[varIdx];
            }

            // setting up CMDP constraints left-hand side
            for (int i = 1; i < problem_->size(); i++) {
                GRBLinExpr& aux3 = CMDPConstLHS[i - 1];
                aux3 += problem_->cost(s, a, i) * variables[varIdx];
            }
        }
//        dprint1(MDPConstLHS[idxState]);
    }

    // adding LP constraints
//    dprint1("\n MDP constraints");
    for (mlcore::State* s : problem_->states()) {
        int idxState = stateIndex_[s];
        double b = (s == problem_->initialState()) ? 1.0 : 0.0;
        GRBLinExpr rhs = b * (1 - problem_->gamma());
        model.addConstr(MDPConstLHS[idxState], GRB_GREATER_EQUAL, rhs);
        model.update();
    }
//    dprint1("\n CMDP constraints");
//    for (int i = 1; i < problem_->size(); i++) {
//        GRBLinExpr rhs = constTargets_[i - 1];
//        model.addConstr(CMDPConstLHS[i - 1], GRB_LESS_EQUAL, rhs);
//        model.update();
//    }

    try {
        model.write("file.lp");
    } catch(GRBException e) {
        std::cerr << "Error code = " << e.getErrorCode() << std::endl;
        std::cerr << e.getMessage() << std::endl;
    }

    // solving the model
    model.optimize();

    // Extracting the policy
    policy_ = new RandomPolicy(problem_, numStates);
    double totalSum = 0.0;
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
            dprint5(s, " ", a, " ", varValue);
            actionProbs.push_back(varValue);
            total += varValue;
            totalSum += varValue;
        }

        if (total == 0) {
//            dprint1(s);
            int idxAction = -1;
            for (mlcore::Action* a : problem_->actions()) {
                idxAction++;
                if (!problem_->applicable(s, a)) {
                    actionProbs.push_back(0.0);
                    continue;
                }
                int varIdx = idxState * numActions + idxAction;
                double varValue = variables[varIdx].get(GRB_DoubleAttr_X);
//                dprint3(a, " ", varValue);
            }
        }

        idxAction = -1;
        for (mlcore::Action* a : problem_->actions()) {
            idxAction++;
            if (!problem_->applicable(s, a))
                continue;
//            if (total == 0)
//                actionProbs[idxAction] = 1.0 / numActions;
//            else
                actionProbs[idxAction] /= total;
        }
        policy_->addActionsState(s, actionProbs);
    }   // states
    dprint1(totalSum);

    delete[] variables;
    delete[] MDPConstLHS;
    delete[] CMDPConstLHS;

}   // solve

} // namespace mlsolvers

