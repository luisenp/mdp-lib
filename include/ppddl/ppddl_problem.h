//#ifndef MDPLIB_PPDDLPROBLEM_H
//#define MDPLIB_PPDDLPROBLEM_H
//
//#include "problem.h"
//#include "mdpsim-2.2.2/problems.h"
//
//class PPDDLProblem : public mlcore::Problem
//{
//private:
//    Problem ppddlProblem_;
//public:
//    PPDDLProblem();
//    virtual ~PPDDLProblem();
//
//    /**
//     * Overrides method from Problem.
//     */
//    virtual bool goal(mlcore::State* s) const;
//
//    /**
//     * Overrides method from Problem.
//     */
//    virtual std::list<mlcore::Successor> transition(mlcore::State* s, mlcore::Action* a);
//
//    /**
//     * Overrides method from Problem.
//     */
//    virtual Rational cost(mlcore::State* s, mlcore::Action* a) const;
//
//    /**
//     * Overrides method from Problem.
//     */
//    virtual bool applicable(mlcore::State* s, mlcore::Action* a) const;
//};
//
//#endif // MDPLIB_PPDDLPROBLEM_H
