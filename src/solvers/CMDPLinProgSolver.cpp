#include <string>

#include "../../include/solvers/CMDPLinProgSolver.h"

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

    int nStates = idx;
    int nActions = problem_->actions().size();
    int nvars = nStates * nActions;

    GRBVar* variables = new GRBVar[nvars];
    GRBLinExpr* MDPConstLHS = new GRBLinExpr[nStates];
    GRBLinExpr* CMDPConstLHS = new GRBLinExpr[problem_->size() - 1];
    GRBLinExpr objFun = 0.0;

    // Creating the variables and setting up objective function
    for (mlcore::State* s : problem_->states()) {
        int idxAction = -1;
        int idxState = stateIndex_[s];
        for (mlcore::Action* a : problem_->actions()) {
            idxAction++;
            if (!problem_->applicable(s, a))
                continue;

            int stateActionIdx = idxState * nActions + idxAction;
//            dprint5(s, " ", a, " ", stateActionIdx);
            variables[stateActionIdx] = model.addVar(0.0, 1000000.0, 0.0, GRB_CONTINUOUS);
            objFun += problem_->cost(s, a, 0) * variables[stateActionIdx];
        }
    }
    model.update();
    model.setObjective(objFun, GRB_MINIMIZE);
    model.update();
//    dprint2("Objective Function ", objFun);

    // Creating LP constraints
    for (mlcore::State* s : problem_->states()) {
        int idxAction = -1;
        int idxState = stateIndex_[s];
        for (mlcore::Action* a : problem_->actions()) {
            idxAction++;
            if (!problem_->applicable(s, a))
                continue;

            int varIdx = idxState * nActions + idxAction;

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
        GRBLinExpr rhs = b;
        model.addConstr(MDPConstLHS[idxState], GRB_EQUAL, rhs);
        model.update();
    }
//    dprint1("\n CMDP constraints");
    for (int i = 1; i < problem_->size(); i++) {
        GRBLinExpr rhs = constTargets_[i - 1];
        model.addConstr(CMDPConstLHS[i - 1], GRB_LESS_EQUAL, rhs);
        model.update();
    }

    try {
        model.write("file.lp");
    } catch(GRBException e) {
        std::cerr << "Error code = " << e.getErrorCode() << std::endl;
        std::cerr << e.getMessage() << std::endl;
    }


    model.optimize();

    delete[] variables;
    delete[] MDPConstLHS;
    delete[] CMDPConstLHS;

}   // solve

} // namespace mlsolvers

