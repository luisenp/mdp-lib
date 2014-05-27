#ifndef MDPLIB_ACTION_H
#define MDPLIB_ACTION_H

#include <iostream>

class Action
{
protected:
    virtual std::ostream& print(std::ostream& os) const =0;

public:
    /**
     * Returns a hash value for the action.
     *
     * @return The hash value of the action.
     */
    virtual int hash_value() const=0;

    friend std::ostream& operator<<(std::ostream& os, Action* a);
};

#endif // MDPLIB_ACTION_H
