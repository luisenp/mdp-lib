#ifndef MDPLIB_VISOLVER_H
#define MDPLIB_VISOLVER_H

#include "../problem.h"

class VISolver
{
    private:
        Problem* problem_;

    public:
        VISolver(Problem* problem);
        void solve(int maxIter);
};

#endif // MDPLIB_VISOLVER_H
