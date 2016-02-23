#include <list>
#include <utility>

#include "../../include/solvers/EpicSolver.h"

using namespace std;
using namespace mlcore;

namespace mlsolvers
{

StateDoubleMap
EpicSolver::computeProbabilityTerminals(State* start,
                                        StateSet& terminals)
{
    StateSet visited;
    StateDoubleMap sdMap1, sdMap2;
    StateDoubleMap* previousProbabilities = &sdMap1;
    StateDoubleMap* nextProbabilities = &sdMap2;
    (*previousProbabilities)[start] = 1.0;
    double maxDiff = 1.0;
    while (maxDiff > mdplib::epsilon) {
        maxDiff = 0.0;
        list<State*> queue;
        queue.push_front(start);
        while (!queue.empty()) {
            State* currentState = queue.back();
            queue.pop_back();
            if (!visited.insert(currentState).second)
                continue;
            if (terminals.count(currentState) > 0 ||
                    problem_->goal(currentState) ||
                    currentState->deadEnd()) {
                (*nextProbabilities)[currentState] +=
                    (*previousProbabilities)[currentState];
                continue;
            }

            for (auto const & successor :
                    problem_->transition(currentState,
                                         currentState->bestAction())) {
                (*nextProbabilities)[successor.su_state] +=
                    (*previousProbabilities)[currentState] * successor.su_prob;
                queue.push_front(successor.su_state);
            }
        }

        for (auto s : visited) {
            maxDiff = max(maxDiff,
                          fabs((*nextProbabilities)[s] -
                              (*previousProbabilities)[s]));

        }
//                                                                                dprint3(maxDiff, maxDiff > mdplib::epsilon, previousProbabilities->size());
//                                                                                double total = 0.0;
//                                                                                for (auto const & SD : *nextProbabilities) {
//                                                                                    dprint3(SD.first, SD.second, (*previousProbabilities)[SD.first]);
//                                                                                    total += SD.second;
//                                                                                }
//                                                                                dprint1(total);
        swap(previousProbabilities, nextProbabilities);
        nextProbabilities->clear();
        visited.clear();
    }
    return *nextProbabilities;
}

} //namespace mlsolvers
