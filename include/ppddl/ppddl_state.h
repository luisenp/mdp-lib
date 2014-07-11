//#ifndef MDPLIB_PPDDLSTATE_H
//#define MDPLIB_PPDDLSTATE_H
//
//#include "../state.h"
//#include "mini-gpt/states.h"
//
//class PPDDLState : public mlcore::State
//{
//private:
//    state_t* pState_;
//
//public:
//    PPDDLState(state_t* pState) : pState_(pState) { }
//
//    virtual ~PPDDLState();
//
//    /**
//     * Overrides method from State.
//     */
//    virtual mlcore::State& operator=(const mlcore::State& rhs)
//    {
//        if (this == &rhs)
//            return *this;
//
//        PPDDLState* state = (PPDDLState*)  & rhs;
//        pState_ =  state->pState_;
//        return *this;
//    }
//
//    /**
//     * Overrides method from State.
//     */
//    virtual bool operator==(const mlcore::State& rhs) const
//    {
//        PPDDLState* state = (PPDDLState*)  & rhs;
//        return pState_ == state->pState_;
//    }
//
//    /**
//     * Overrides method from State.
//     */
//    virtual bool equals(mlcore::State* rhs) const
//    {
//        PPDDLState* state = (PPDDLState*)  & rhs;
//        return *this == *rhs;
//    }
//
//    /**
//     * Overrides method from State.
//     */
//    virtual int hashValue() const;
//};
//
//#endif // MDPLIB_PPDDLSTATE_H
