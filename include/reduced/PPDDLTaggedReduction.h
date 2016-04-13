#ifndef PPDDLTAGGEDREDUCTION_H
#define PPDDLTAGGEDREDUCTION_H

#include <fstream>
#include <iostream>
#include <list>
#include <vector>


#include "../ppddl/PPDDLAction.h"
#include "../ppddl/PPDDLProblem.h"
#include "../ppddl/PPDDLState.h"

#include "../State.h"

#include "ReducedTransition.h"


namespace mlreduced
{

class PPDDLTaggedReduction : public ReducedTransition
{
private:
    mlcore::Problem* problem_;

    std::string reductionDescriptionFilename_;

public:
    PPDDLTaggedReduction(mlcore::Problem* problem,
                         std::string reductionDescriptionFilename) :
        problem_(problem),
        reductionDescriptionFilename_(reductionDescriptionFilename) { }

    virtual ~PPDDLTaggedReduction() { }

    /**
     * Overrides method from ReducedTransition.
     */
    virtual void setPrimary(mlcore::State* s,
                       mlcore::Action *a,
                       std::vector<bool>& primaryIndicator) const
    {
        std::ifstream ifs(reductionDescriptionFilename_);
        if (ifs.is_open()) {
            std::string line;
            while (getline(ifs, line)) {
                mlppddl::PPDDLAction* pAction =
                    static_cast<mlppddl::PPDDLAction*> (a);
            }
        } else {
            std::cerr << "Unable to open " <<
                reductionDescriptionFilename_ << std::endl;
        }
    }
};

}
#endif // PPDDLTAGGEDREDUCTION_H
