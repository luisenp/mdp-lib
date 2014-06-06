#ifndef MDPLIB_LAOSTARSOLVER_H
#define MDPLIB_LAOSTARSOLVER_H


/**
 * A SSPP solver using the LAO* algorithm.
 *
 * See http://www.sciencedirect.com/science/article/pii/S0004370201001060
 */
class LAOStarSolver
{
private:
    Problem* problem_;
    StateSet visited;

    /* Expands the BPSG rooted at state s and returns the number of states expanded */
    int expand(State* s, int level);

    /* Test if the BPSG rooted at state s has converged */
    double testConvergence(State* s, int level);
public:
    /**
     * Creates a LAO* solver for the given problem.
     *
     * @param problem The problem to be solved.
     */
    LAOStarSolver(Problem* problem) : problem_(problem) { }

    /**
     * Solves the associated problem using the LAO* algorithm.
     *
     * @param s0 The state to start the search at.
     * @param epsilon The error tolerance.
     */
    void solve(State* s0, Rational epsilon);

};

#endif // MDPLIB_LAOSTARSOLVER_H
