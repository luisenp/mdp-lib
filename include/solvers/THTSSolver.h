#ifndef MDPLIB_THTSSOLVER_H
#define MDPLIB_THTSSOLVER_H

#include "Solver.h"

namespace
{

class THTSNode
{
protected:
    THTSNode* parent_;
    std::vector<THTSNode*> successors_;
    int depth_;
public:
    THTSNode* parent() { return parent_; }

    std::vector<THTSNode*>& successors() { return successors_; }

    int depth() { return depth_; }
};

class ChanceNode : public THTSNode
{
};

class DecisionNode : public THTSNode
{
};


class THTSSolver : public Solver
{

public:
    THTSSolver();
    virtual ~THTSSolver();

    /**
     * Overrides method from Solver.
     */
    virtual mlcore::Action* solve(mlcore::State* s0);
};

} // namespace mlsolvers

#endif // MDPLIB_THTSSOLVER_H
