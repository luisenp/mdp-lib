#ifndef GRIDWORLDACTION_H
#define GRIDWORLDACTION_H

#include "../../action.h"

namespace gridworld
{
    const unsigned char UP = 0;
    const unsigned char DOWN = 1;
    const unsigned char LEFT= 2;
    const unsigned char RIGHT = 3;
}

class GridWorldAction : public Action
{
private:
    const unsigned char dir_;
public:
    GridWorldAction() : dir_(-1) {}

    GridWorldAction(const unsigned char dir) : dir_(dir) {}

    virtual int hash_value()
    {
        return (int) dir_;
    }
};

#endif // GRIDWORLDACTION_H
