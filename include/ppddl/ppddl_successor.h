#ifndef MDPLIB_PPDDLSUCCESSOR_H
#define MDPLIB_PPDDLSUCCESSOR_H

#include <cassert>
#include <iostream>
#include <list>

#include "mdpsim/config.h"
#include "mdpsim/problems.h"
#include "mdpsim/actions.h"
#include "mdpsim/formulas.h"
#include "mdpsim/expressions.h"

namespace mlppddl
{


 // TODO: The class description needs to be improved
 // TODO: For efficiency, all of this should be done at the PPDDL description level instead of the
 //          the ground level

/**
 * This class represents a "node" of a PPDDL effect description.
 *
 * A PPDDL effect description can be seen as a tree where inner nodes are either
 * AND or OR nodes, corresponding to conjuctive or probabilistic PPDDL sub-effects,
 * respectively. The leaves of the tree correspond to atomic effects such as add,
 * delete and update effects.
 *
 * The primary purpose of this class it to provide methods to expand a list with
 * all successor states of a given state-action pair, according to the corresponding
 * PPDDL description. This is useful for methods that require full Bellman backups,
 * such as Value Iteration, LAO* and LRTDP.
 */
class Node
{
private:
    bool isAtomic_;
    std::list< std::pair<Node *, double> > children_;

    AtomList adds_;
    AtomList deletes_;
    UpdateList updates_;

    Node* productAtomic(Node* left, Node* right)
    {
        assert(left->isAtomic_ && right->isAtomic_);

        AtomList resAdds;
        AtomList resDeletes;
        UpdateList resUpdates;

        Node* result = new Node(resAdds, resDeletes, resUpdates);

        for (const Atom* atom : left->adds_)
            result->adds_.push_back(atom);
        for (const Atom* atom : right->adds_)
            result->adds_.push_back(atom);

        for (const Atom* atom : left->deletes_)
            result->deletes_.push_back(atom);
        for (const Atom* atom : right->deletes_)
            result->deletes_.push_back(atom);

        for (const Update* atom : left->updates_)
            result->updates_.push_back(atom);
        for (const Update* atom : right->updates_)
            result->updates_.push_back(atom);

        return result;
    }

public:

    Node(std::list< std::pair<Node *, double> > children)
    {
        children_ = children;
        isAtomic_ = false;
    }

    Node(AtomList adds, AtomList deletes, UpdateList updates)
    {
        adds_ = adds;
        deletes_ = deletes;
        updates_ = updates;
        isAtomic_ = true;
    }

    virtual ~Node()
    {
        if (!isAtomic_) {
            for (std::pair<Node *, double> child : children_)
                delete child.first;
        }
    }

    bool isAtomic() { return isAtomic_; }

    std::list< std::pair<Node *, double> > children() { return children_; }

    AtomList adds() { return adds_; }

    AtomList deletes() { return deletes_; }

    UpdateList updates() { return updates_; }

    /**
     * Performs a factor product of two nodes. The result is stored in the children
     * list of the returned node.
     *
     * This method assumes that both nodes are either atomic or their children are
     * atomic. Otherwise it throws an error.
     */
    Node* product(Node* left, Node* right)
    {
        Node* result;
        if (left->isAtomic_ && right->isAtomic_) {
            result = productAtomic(left, right);
        } else if (left->isAtomic_) {
            std::list< std::pair< Node *, double> > resChildren;
            for (std::pair<Node*, double> child : right->children_) {
                Node* childResult = productAtomic(left, child.first);
                resChildren.push_back(std::make_pair(childResult, child.second));
            }
            result = new Node(resChildren);
        } else if (right->isAtomic_) {
            std::list< std::pair< Node *, double> > resChildren;
            for (std::pair<Node*, double> child : left->children_) {
                Node* childResult = productAtomic(right, child.first);
                resChildren.push_back(std::make_pair(childResult, child.second));
            }
            result = new Node(resChildren);
        } else {
            std::list< std::pair< Node *, double> > resChildren;
            for (std::pair<Node*, double> childLeft : left->children_) {
                for (std::pair<Node*, double> childRight : right->children_) {
                    Node* childResult = productAtomic(childLeft.first, childRight.first);
                    double p = childLeft.second * childRight.second;
                    resChildren.push_back(std::make_pair(childResult, p));
                }
            }
            result = new Node(resChildren);
        }

        delete left;
        delete right;

        return result;
    }

    /**
     * For any children X of this node that is not atomic, all childrens of X are
     * converted into childrens of this node and the probabilities ajusted to reflect
     * the nested probabilities. This method assumes that all children of X are
     * atomic and will throw an error otherwise.
     */
    void flatten()
    {
        if (isAtomic_)
            return;
        std::list < std::pair<Node*, double> > newChildren;
        for (std::pair<Node*, double> child : children_) {
            if (child.first->isAtomic_) {
                newChildren.push_back(child);
                continue;
            }
            for (std::pair<Node*, double> grandChild : child.first->children_) {
                assert(grandChild.first->isAtomic_);
                double p = child.second * grandChild.second;
                Node* tmp = grandChild.first;
                newChildren.push_back(std::make_pair(tmp, p));
                grandChild.first = NULL;
            }
            delete child.first;
        }
        children_ = newChildren;
    }
};

}
#endif // MDPLIB_PPDDLSUCCESSOR_H
