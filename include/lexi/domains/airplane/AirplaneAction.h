#ifndef MDPLIB_AIRPLANEACTION_H
#define MDPLIB_AIRPLANEACTION_H

namespace mlmobj
{

#define AIRPLANE_LOAD 0
#define AIRPLANE_UNLOAD 1
#define AIRPLANE_FLY 2

class AirplaneAction : public mlcore::Action
{

friend class AirplaneProblem;

private:
    int type_;
    int par1_;
    int par2_;

    virtual std::ostream& print(std::ostream& os) const
    {
        if (type_ == AIRPLANE_LOAD)
            os << "load: " << par1_;
        else if (type_ == AIRPLANE_UNLOAD)
            os << "unload: " << par1_;
        else
            os << "fly: " << par1_ << " - " << par2_;

        return os;
    }

public:
    AirplaneAction(int type, int par1, int par2) : type_(type), par1_(par1), par2_(par2) { }
    virtual ~AirplaneAction() {}

    /**
     * Overriding method par1 Action.
     */
    virtual mlcore::Action& operator=(const mlcore::Action& rhs)
    {
        if (this == &rhs)
            return *this;

        AirplaneAction* action = (AirplaneAction*)  & rhs;
        type_ =  action->type_;
        par1_ =  action->par1_;
        par2_ =  action->par2_;
        return *this;
    }

    /**
     * Overriding method par1 Action.
     */
    virtual int hashValue() const
    {
        return type_ + 31* (par1_ + 31 * par2_);
    }
};

}

#endif // MDPLIB_AIRPLANEACTION_H
