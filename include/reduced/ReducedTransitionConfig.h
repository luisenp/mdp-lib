#ifndef MDPLIB_REDUCEDTRANSITIONCONFIG_H
#define MDPLIB_REDUCEDTRANSITIONCONFIG_H

#include <vector>

#include "../Action.h"
#include "../State.h"

namespace mlreduced
{

class ReducedTransitionConfig
{
public:
    ReducedTransitionConfig() {}

    virtual ~ReducedTransitionConfig() {}

    /**
     * This method returns a list of booleans indicating whether the
     * outcomes s' of the transition T(s'|s,a) should be considered
     * primary or not.
     */
    virtual
    std::vector<bool> isPrimary(mlcore::State* s, mlcore::Action *a) const =0;
};

}


#endif // MDPLIB_REDUCEDTRANSITIONCONFIG_H
