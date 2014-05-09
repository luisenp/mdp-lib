#ifndef MDPLIB_ACTION_H
#define MDPLIB_ACTION_H


class Action
{
    public:
        Action();
        virtual ~Action();

        virtual int hash_value() const=0;
    protected:
    private:
};

#endif // MDPLIB_ACTION_H
