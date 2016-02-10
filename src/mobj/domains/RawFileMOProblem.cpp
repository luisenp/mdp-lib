#include <iostream>
#include <fstream>
#include <sstream>

#include "../../../include/State.h"

#include "../../../include/mobj/domains/RawFileMOProblem.h"
#include "../../../include/mobj/domains/RawFileMOState.h"
#include "../../../include/mobj/domains/RawFileMOAction.h"

namespace mlmobj
{

RawFileMOProblem::RawFileMOProblem(char* filename)
{
    std::ifstream myfile (filename);

    if (myfile.is_open()) {
        std::string line;

        int n, m, k, r, idxS0, h;
        double gamma;

        std::getline(myfile, line);
        std::istringstream iss(line);
        iss >> n >> m >> k >> r >> idxS0 >> h >> gamma;

        /* Creating an initial state */
        s0 = new RawFileMOState(-1, m, k);
        indexS0_ = idxS0;
        nActions_ = m;
        size_ = k;
        this->addState(s0);

        /* Creating all states */
        for (int i = 0; i < n; i++)
            this->addState(new RawFileMOState(i, m, k));

        /* Creating all actions */
        for (int i = 0; i < m; i++)
            actions_.push_back(new RawFileMOAction(i));

        /* Reading transition function */
        for (int idState = 0; idState < n; idState++) {
            RawFileMOState* state =
                (RawFileMOState*) this->addState(new RawFileMOState(idState, m, k));
            for (int idAction = 0; idAction < m; idAction++) {
                std::getline(myfile, line);
                std::istringstream issTSA(line);

                for (int idSucc = 0; idSucc < n; idSucc++) {
                    double p;
                    issTSA >> p;
                    if (p > 0.0) {
                        RawFileMOState* succState =
                            (RawFileMOState*) this->addState(new RawFileMOState(idSucc, m, k));
                        state->addSuccessor(idAction, succState, p);
                    }
                }
            }
        }

        /* Reading rewards */
        for (int level = 0; level < k; level++) {
            for (int idState = 0; idState < n; idState++) {
                RawFileMOState* state =
                    (RawFileMOState*) this->addState(new RawFileMOState(idState, m, k));

                for (int idAction = 0; idAction < m; idAction++) {
                    std::getline(myfile, line);
                    std::istringstream issRSA(line);

                    mlcore::SuccessorsList sccrs = state->getSuccessors(idAction);
                    double rSA = 0.0;
                    for (int idSucc = 0; idSucc < n; idSucc++) {
                        double r;
                        issRSA >> r;
                        if (r < 0.0) {
                            for (mlcore::Successor su : sccrs) {
                                RawFileMOState* succState = (RawFileMOState*) su.su_state;
                                if (succState->id() == idSucc) {
                                    rSA -= r * su.su_prob;
                                }
                            }
                        }
                    }
                    state->setCostAction(idAction, level, rSA);
                }
            }

        }

        myfile.close();
    }
}

double RawFileMOProblem::cost(mlcore::State* s, mlcore::Action* a, int i) const
{
    RawFileMOState * state = (RawFileMOState *) s;
    RawFileMOAction * action = (RawFileMOAction *) a;
    return state->getCostAction(action->id(), i);
}

mlcore::SuccessorsList
RawFileMOProblem::transition(mlcore::State* s, mlcore::Action* a, int index)
{
    if (s == s0) {
        mlcore::State* succ = this->addState(new RawFileMOState(indexS0_, nActions_, size_));
        mlcore::SuccessorsList successors;
        successors.push_back(mlcore::Successor(succ, 1.0));
        return successors;
    } else {
        RawFileMOState * state = (RawFileMOState *) s;
        RawFileMOAction * action = (RawFileMOAction *) a;
        return state->getSuccessors(action->id());
    }
}

bool RawFileMOProblem::goal(mlcore::State* s, int index) const
{
    return false;
}

bool RawFileMOProblem::applicable(mlcore::State* s, mlcore::Action* a) const
{
    return true;
}

}
