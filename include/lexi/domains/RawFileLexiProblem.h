#ifndef MDPLIB_RAWFILELEXIPROBLEM_H
#define MDPLIB_RAWFILELEXIPROBLEM_H

namespace mllexi
{

class RawFileLexiProblem : public LexiProblem
{
private:
public:
    RawFileLexiProblem();
    virtual ~RawFileLexiProblem();

    /**
     * Overrides method from LexiProblem.
     */
    virtual double cost(mlcore::State* s, mlcore::Action* a, int i) const;

    /**
     * Overrides method from LexiProblem.
     */
    virtual std::list<mlcore::Successor> transition(mlcore::State* s, mlcore::Action* a, int index);

    /**
     * Overrides method from LexiProblem.
     */
    virtual bool goal(mlcore::State* s, int index) const;

    /**
     * Overrides method from LexiProblem.
     */
    virtual bool applicable(mlcore::State* s, mlcore::Action* a) const;
};

}

#endif // MDPLIB_RAWFILELEXIPROBLEM_H
