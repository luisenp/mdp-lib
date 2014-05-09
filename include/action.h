#ifndef MDPLIB_ACTION_H
#define MDPLIB_ACTION_H


class Action
{
public:
    Action();
    virtual ~Action();

    /**
    * Returns a hash value for the action.
    */
    virtual int hash_value() const=0;
};

#endif // MDPLIB_ACTION_H
