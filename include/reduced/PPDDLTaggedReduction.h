#ifndef PPDDLTAGGEDREDUCTION_H
#define PPDDLTAGGEDREDUCTION_H

#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>


#include "../ppddl/PPDDLAction.h"

#include "../State.h"

#include "ReducedTransition.h"


namespace mlreduced
{

class PPDDLTaggedReduction : public ReducedTransition
{
private:
    mlcore::Problem* problem_;

    std::string reductionDescriptionFilename_;

    std::unordered_map<std::string, int> actionPrimaryOutcomeIndex_;

public:
    PPDDLTaggedReduction(mlcore::Problem* problem,
                         std::string reductionDescriptionFilename) :
        problem_(problem),
        reductionDescriptionFilename_(reductionDescriptionFilename)
    {
        std::ifstream ifs(reductionDescriptionFilename_);
        if (ifs.is_open()) {
            std::string line;
            while (getline(ifs, line)) {
                std::string actionName;
                int primaryOutcomeIndex;
                std::istringstream iss(line);
                iss >> actionName >> primaryOutcomeIndex;
                actionPrimaryOutcomeIndex_[actionName] = primaryOutcomeIndex;
            }
        } else {
            std::cerr << "Unable to open " <<
                reductionDescriptionFilename_ << std::endl;
        }
    }

    virtual ~PPDDLTaggedReduction() { }

    /**
     * Overrides method from ReducedTransition.
     */
    virtual void setPrimary(mlcore::State* s,
                            mlcore::Action *a,
                            std::vector<bool>& primaryIndicator) const
    {
        mlppddl::PPDDLAction* pAction = static_cast<mlppddl::PPDDLAction*> (a);
        // Name of the action including the parameters (e.g., (pick-up b2 b1)).
        std::string fullActionName(pAction->pAction()->name());
        // Name of the action schema (e.g. pick-up).
        int idx = fullActionName.find_first_of(" )");
        std::string actionName = fullActionName.substr(1,  idx - 1);
        int thisActionPrimaryOutcomeIndex = 0;
        if (actionPrimaryOutcomeIndex_.count(actionName))
            thisActionPrimaryOutcomeIndex =
                actionPrimaryOutcomeIndex_.at(actionName);
        int i = 0;
        for (auto const & successors : problem_->transition(s, a)) {
            if (thisActionPrimaryOutcomeIndex == i) {
                primaryIndicator.push_back(true);
            } else {
                primaryIndicator.push_back(false);
            }
            i++;
        }
    }
};

}
#endif // PPDDLTAGGEDREDUCTION_H
