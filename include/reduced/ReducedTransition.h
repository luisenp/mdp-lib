#ifndef MDPLIB_REDUCEDTRANSITION_H
#define MDPLIB_REDUCEDTRANSITION_H

#include <vector>

#include "../Action.h"
#include "../State.h"

namespace mlreduced
{

class ReducedTransition
{
public:
    ReducedTransition() {}

    virtual ~ReducedTransition() {}

    /**
     * This method fills a list of booleans indicating whether the
     * outcomes s' of the transition T(s'|s,a) should be considered
     * primary or not.
     *
     * If the method doesn't add any indicators, then the ReducedModel class
     * will assume all outcomes are primary.
     */
    virtual void setPrimary(mlcore::State* s,
                            mlcore::Action *a,
                            std::vector<bool>& primaryIndicator) const =0;
};

}


#endif // MDPLIB_REDUCEDTRANSITIONCONFIG_H
