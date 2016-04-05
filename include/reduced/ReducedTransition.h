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
     * outcomes, s', of the transition T(s'|s,a) should be considered
     * primary or not.
     */
    virtual void
    getPrimaryIndicators(mlcore::State* s,
                         mlcore::Action *a,
                         std::vector<bool>& primaryIndicator) const =0;

    /**
     * This method fills a list of booleans indicating whether the
     * outcomes, s', of the transition T(s'|s,a) should increase the
     * reduced transition counter or not.
     *
     * If the method returns an empty list, this should be interpreted
     * as exceptions are the only outcomes that increase the counter.
     */
    virtual void
    getIsCounterIncrementer(mlcore::State* s,
                            mlcore::Action *a,
                            std::vector<bool>& isCounterIncrementer) const =0;
};

}


#endif // MDPLIB_REDUCEDTRANSITIONCONFIG_H
