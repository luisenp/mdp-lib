#include "../../../include/util/general.h"

#include "../../../include/solvers/mobj/CMDPLagrangeSearchSolver.h"

using namespace std;

namespace mdplib_mobj_solvers
{

vector<double> CMDPLagrangeSearchSolver::getExpectedCostOfGreedyPolicy(
    mlcore::State* s0, int numIterations)
{
    int k = problem_->size();
    vector<double> expectedMOCost(k, 0.0);
    for (int i = 0; i < numIterations; i++) {
        vector<double> trialCost = sampleTrial(problem_, s0);
        for (int j = 0; j < k; j++)
            expectedMOCost[j] += trialCost[j];
    }
    for (int i = 0; i < k; i++)
        expectedMOCost[i] /= numIterations;
    return expectedMOCost;
}


vector<double> CMDPLagrangeSearchSolver::setNewWeightsForProblem(
    vector<int> comb, vector<double> weightValues)
{
    vector<double> weights(comb.size() + 1, 1.0);
    for (int i = 0; i < comb.size(); i++)
        weights[i + 1] = 0.05 * weightValues[comb[i]];
    for (mlcore::State* x : problem_->states()) {
        ((mlmobj::MOState * ) x)->resetCost(weightValues, -1);
        ((mlmobj::MOState * ) x)->setBestAction(nullptr);
    }
    problem_->weights(weights);
    return weights;
}


double CMDPLagrangeSearchSolver::evaluateObjectiveFunction(
    vector<double> lambdas, mlcore::State* s0, mlcore::Action** bestAction)
{
    double targets[3] = {0.0, 25.0, 25.0};
    *bestAction = solver_->solve(s0);
    vector<double> expectedMOCost = getExpectedCostOfGreedyPolicy(s0, 100000);
    double totalWeightedCost = 0.0;
    int k = problem_->size();
    double totalWeightedTargets = 0.0;
    for (int i = 0; i < k; i++) {
        cout << lambdas[i] << " ";
        totalWeightedCost += lambdas[i] * (expectedMOCost[i] - targets[i]);
        totalWeightedTargets += lambdas[i] * targets[i];
    }
    for (int i = 0; i < k; i++)
        cout << expectedMOCost[i] << " ";
    cout << totalWeightedCost <<
            " " << (s0->cost() - totalWeightedTargets) << endl;

//    cout << " DEBUG " << (s0->cost()) << endl;
    return totalWeightedCost;
}


mlcore::Action* CMDPLagrangeSearchSolver::solve(mlcore::State* s0)
{
    int k = problem_->size();
    vector<int> comb(k - 1, 0);
    int M = 30;
    vector<double> weightValues;
    for (int i = 0; i < M; i++)
        weightValues.push_back(i * 1.0/(M - 1));
    mlcore::Action* bestAction = nullptr;
    double bestObjFunctionValue = -100000.0;
    vector<double> bestLambdas;
    do  {
        vector<double> lambdas = setNewWeightsForProblem(comb, weightValues);
        double objFuncValue = evaluateObjectiveFunction(lambdas, s0, &bestAction);
        if (objFuncValue > bestObjFunctionValue) {
            bestObjFunctionValue = objFuncValue;
            bestLambdas = lambdas;
        }
    } while (nextCombRep(comb, weightValues.size()));
    return bestAction;

//    int N = 100000;
//    vector<double> weights({1, 0, 0});
//    problem_->weights(weights);
//    mlcore::Action* bestAction = solver_->solve(s0);
//    double expectedCost = 0.0;
//    for (int i = 0; i < N; i++) {
//        vector<double> aux;
//        expectedCost += sampleTrial(problem_, s0, aux);
//    }
//    cout << (expectedCost / N) << " " << s0->cost() << endl;
//
//    weights = vector<double> ({1, 0.2, 0.2});
//    problem_->weights(weights);
//    for (mlcore::State* x : problem_->states()) {
//        ((mlmobj::MOState * ) x)->resetCost(weights, -1);
//        ((mlmobj::MOState * ) x)->setBestAction(nullptr);
//    }
//    bestAction = solver_->solve(s0);
//    expectedCost = 0.0;
//    for (int i = 0; i < N; i++) {
//        vector<double> aux;
//        expectedCost += sampleTrial(problem_, s0, aux);
//    }
//    cout << (expectedCost / N) << " " << s0->cost() << endl;
//
//    weights = vector<double> ({1, 0.5, 0.5});
//    problem_->weights(weights);
//    for (mlcore::State* x : problem_->states()) {
//        ((mlmobj::MOState * ) x)->resetCost(weights, -1);
//        ((mlmobj::MOState * ) x)->setBestAction(nullptr);
//    }
//    bestAction = solver_->solve(s0);
//    expectedCost = 0.0;
//    for (int i = 0; i < N; i++) {
//        vector<double> aux;
//        expectedCost += sampleTrial(problem_, s0, aux);
//    }
//    cout << (expectedCost / N) << " " << s0->cost() << endl;
}


} // namespace mdplib_mobj_solvers
