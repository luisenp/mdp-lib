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
        std::ostringstream oss("");
        oss << pAction;
        for (auto const & successors : problem_->transition(s, a)) {
            primaryIndicator.push_back(true);
            return;
        }
    }
};

}
#endif // PPDDLTAGGEDREDUCTION_H
