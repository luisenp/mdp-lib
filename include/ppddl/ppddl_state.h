//#ifndef MDPLIB_PPDDLSTATE_H
//#define MDPLIB_PPDDLSTATE_H
//
//#include "problem.h"
//#include "mdpsim-2.2.2/states.h"
//
//class PPDDLState : public mlcore::State
//{
//private:
//  /* The problem that this state is associated with. */
//  const Problem* problem_;
//
//  /* Atomic state formulas that hold in this state. */
//  AtomSet atoms_;
//
//  /* Fluent values in this state. */
//  ValueMap values_;
//
//  /* Whether this is a goal state. */
//  bool goal_;
//
//public:
//    PPDDLState();
//    virtual ~PPDDLState();
//
//    /**
//     * Overrides method from State.
//     */
//    virtual mlcore::State& operator=(const mlcore::State& rhs);
//
//    /**
//     * Overrides method from State.
//     */
//    virtual bool operator==(const mlcore::State& rhs) const;
//
//    /**
//     * Overrides method from State.
//     */
//    virtual bool equals(mlcore::State* other) const;
//
//    /**
//     * Overrides method from State.
//     */
//    virtual int hashValue() const;
//};
//
//#endif // MDPLIB_PPDDLSTATE_H
//
