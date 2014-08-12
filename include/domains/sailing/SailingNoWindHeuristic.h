#ifndef MDPLIB_SAILNOWINDHEUR_H
#define MDPLIB_SAILNOWINDHEUR_H

#include "SailingProblem.h"
#include "../../heuristic.h"

class SailingProblem;
class SailingState;

class SailingNoWindHeuristic : public mlcore::Heuristic
{
private:
    SailingProblem* problem_;
    SailingState* tmp_;
public:
    SailingNoWindHeuristic();
    virtual ~SailingNoWindHeuristic() {}

    SailingNoWindHeuristic(SailingProblem* problem);

    virtual double cost(const mlcore::State* s) const;
};

#endif // MDPLIB_SAILNOWINDHEUR_H

