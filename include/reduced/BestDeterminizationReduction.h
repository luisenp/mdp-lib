#ifndef MDPLIB_BESTDETERMINIZATIONREDUCTION_H
#define MDPLIB_BESTDETERMINIZATIONREDUCTION_H

#include "../Action.h"
#include "../Problem.h"
#include "../State.h"

#include "ReducedTransition.h"

namespace mlreduced
{

class BestDeterminizationReduction : public ReducedTransition
{
private:
    static const int MAX_SUCCESSORS = 64;

public:
    BestDeterminizationReduction(mlcore::Problem* problem);

    virtual ~BestDeterminizationReduction() { }

    /**
     * Overrides method from ReducedTransition.
     */
    virtual void setPrimary(mlcore::State* s,
                            mlcore::Action *a,
                            std::vector<bool>& primaryIndicator) const;
};

}

#endif // MDPLIB_BESTDETERMINIZATIONREDUCTION_H
