#ifndef MDPLIB_THTSSOLVER_H
#define MDPLIB_THTSSOLVER_H

#include "../Problem.h"

#include "Solver.h"

namespace
{

// A node in the search tree.
class THTSNode
{
protected:
    // The parent node.
    THTSNode* parent_;

    // The set of successor nodes.
    std::vector<THTSNode*> successors_;

    // The depth at which this node is found.
    int depth_;

    // Whether this node has been solved or not.
    bool solved_;

    // The counter of how many times this node has been backed up.
    int backup_counter_;

    // The counter of how many times this node has been visited.
    int selection_counter_;

public:
    THTSNode* parent() { return parent_; }

    std::vector<THTSNode*>& successors() { return successors_; }

    int depth() { return depth_; }

    bool solved() { return solved_; }

    int backup_counter() { return backup_counter_; }

    int selection_counter() { return selection_counter_; }

    void increase_backup_counter() { backup_counter_++; }

    void increase_selection_counter() { selection_counter_++; }
};

// A chance node in the search tree, representing a state-action pair.
class ChanceNode : public THTSNode
{
private:
    double action_value_;

    std::vector<DecisionNode*> explicated_successors_;
public:

    std::vector<DecisionNode*>& explicated_successors()
    {
        return explicated_successors_;
    }
};

// A decision node in the search tree, representing a state.
class DecisionNode : public THTSNode
{
private:
    double state_value_;
public:
    DecisionNode(mlcore::Problem* problem, mlcore::State* state);
};


// A Trial-based Heuristic Tree Search solver.
// See http://ai.cs.unibas.ch/papers/keller-dissertation.pdf.
class THTSSolver : public Solver
{
private:
    mlcore::Problem* problem_;

    int num_trials_;

public:
    THTSSolver(mlcore::Problem* problem, int num_trials) :
        problem_(problem), num_trials_(num_trials) {}

    virtual ~THTSSolver() {}

    /**
     * Overrides method from Solver.
     */
    virtual mlcore::Action* solve(mlcore::State* s0);
};

} // namespace mlsolvers

#endif // MDPLIB_THTSSOLVER_H
