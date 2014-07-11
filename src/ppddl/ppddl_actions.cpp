#include "../../include/ppddl/ppddl_successor.h"
#include "../../include/ppddl/mdpsim/actions.h"

mlppddl::Node* Action::expand(const TermTable& terms, AtomSet& atoms, ValueMap& values) const
{
    AtomList adds;
    AtomList deletes;
    UpdateList updates;
    mlppddl::Node* expandedEffect = effect().expand(adds, deletes, updates, terms, atoms, values);
}
