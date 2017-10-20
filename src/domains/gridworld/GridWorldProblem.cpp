#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>

#include "../../../include/Problem.h"
#include "../../../include/domains/gridworld/GridWorldProblem.h"
#include "../../../include/domains/gridworld/GridWorldState.h"
#include "../../../include/domains/gridworld/GridWorldAction.h"

void GridWorldProblem::addAllActions()
{
    mlcore::Action* up = new GridWorldAction(gridworld::UP);
    mlcore::Action* down = new GridWorldAction(gridworld::DOWN);
    mlcore::Action* left = new GridWorldAction(gridworld::LEFT);
    mlcore::Action* right = new GridWorldAction(gridworld::RIGHT);
    actions_.push_front(up);
    actions_.push_front(down);
    actions_.push_front(left);
    actions_.push_front(right);
}


GridWorldProblem::GridWorldProblem() :
                    width_(0), height_(0), x0_(0), y0_(0),
                    goals_(0), actionCost_(0.03)
{
    absorbing = new GridWorldState(this, -1, -1);
    addAllActions();
}


GridWorldProblem::GridWorldProblem(const char* filename,
                                   double actionCost,
                                   double holeCost,
                                   bool allDirections)
{
    std::ifstream myfile (filename);

    goals_ = new PairDoubleMap();

    // Once the file is finished parsing, these will have correct values
    width_ = 0, height_ = 0;
    if (myfile.is_open()) {
        std::string line;
        while ( std::getline (myfile, line) ) {
            for (width_ = 0; width_ < line.size(); width_++) {
                if (line.at(width_) == 'x') {
                    walls.insert(std::pair<int, int>(width_, height_));
                } else if (line.at(width_) == '@') {
                    holes.insert(std::pair<int, int>(width_, height_));
                } else if (line.at(width_) == 'G') {
                    goals_->insert(
                        std::make_pair(
                            std::pair<int,int> (width_, height_), 0.0));
                } else if (line.at(width_) == 'D') {
                    dead_ends.insert(std::pair<int, int> (width_, height_));
                } else if (line.at(width_) == 'S') {
                    x0_ = width_;
                    y0_ = height_;
                } else {
                    assert(line.at(width_) == '.');
                }
            }
            height_++;
        }
        myfile.close();
    } else {
        std::cerr << "Invalid file " << filename << std::endl;
        exit(-1);
    }
    actionCost_ = actionCost;
    holeCost_ = holeCost;
    allDirections_ = allDirections;
    s0 = new GridWorldState(this, x0_, y0_);
    absorbing = new GridWorldState(this, -1, -1);
    this->addState(s0);
    this->addState(absorbing);
    addAllActions();
}


GridWorldProblem::GridWorldProblem(
    int width, int height, int x0, int y0,
    PairDoubleMap* goals, double actionCost) :
        width_(width), height_(height), x0_(x0), y0_(y0),
        actionCost_(actionCost)
{
    goals_ = new PairDoubleMap();
    for (auto const & goalEntry : *goals)
        (*goals_)[goalEntry.first] = goalEntry.second;
    s0 = new GridWorldState(this, x0_, y0_);
    absorbing = new GridWorldState(this, -1, -1);
    this->addState(s0);
    addAllActions();
}


GridWorldProblem::GridWorldProblem(int width, int height,
                                   int x0, int y0,
                                   PairDoubleMap* goals, mlcore::Heuristic* h)
                                   : width_(width), height_(height),
                                     x0_(x0), y0_(y0)
{
    goals_ = new PairDoubleMap();
    for (auto const & goalEntry : *goals)
        (*goals_)[goalEntry.first] = goalEntry.second;
    s0 = new GridWorldState(this, x0_, y0_);
    absorbing = new GridWorldState(this, -1, -1);
    this->addState(s0);
    heuristic_ = h;
    gamma_ = 1.0;
    addAllActions();
}


bool GridWorldProblem::gridGoal(GridWorldState* gws) const
{
    std::pair<int,int> pos(gws->x(),gws->y());
    return goals_->find(pos) != goals_->end();
}


bool GridWorldProblem::goal(mlcore::State* s) const
{
    return s == absorbing;
}


std::list<mlcore::Successor>
GridWorldProblem::transition(mlcore::State *s, mlcore::Action *a)
{
    GridWorldState* state = static_cast<GridWorldState *> (s);
    GridWorldAction* action = static_cast<GridWorldAction *> (a);

    std::list<mlcore::Successor> successors;

    if (s == absorbing || gridGoal(state)) {
        successors.push_front(mlcore::Successor(absorbing, 1.0));
        return successors;
    }

    if (dead_ends.count(std::pair<int, int>(state->x(), state->y()))) {
        s->markDeadEnd();
        successors.push_front(mlcore::Successor(s, 1.0));
        return successors;
    }

    double probForward = 0.8;
    int numSuccessors = allDirections_ ? 3 : 2;
    double probSides = 0.2 / numSuccessors;
    if (action->dir() == gridworld::UP) {
        addSuccessor(state, successors, height_ - 1, state->y(),
                     state->x(), state->y() + 1, probForward);

        addSuccessor(state, successors, state->x(), 0,
                     state->x() - 1, state->y(), probSides);

        addSuccessor(state, successors, width_ - 1, state->x(),
                     state->x() + 1, state->y(), probSides);

        if (allDirections_) {
            addSuccessor(state, successors, state->y(), 0,
                         state->x(), state->y() - 1, probSides);
        }
    } else if (action->dir() == gridworld::DOWN) {
        addSuccessor(state, successors, state->y(), 0,
                     state->x(), state->y() - 1, probForward);

        addSuccessor(state, successors, state->x(), 0,
                     state->x() - 1, state->y(), probSides);

        addSuccessor(state, successors, width_ - 1, state->x(),
                     state->x() + 1, state->y(), probSides);

        if (allDirections_) {
            addSuccessor(state, successors, height_ - 1, state->y(),
                         state->x(), state->y() + 1, probSides);
        }
    } else if (action->dir() == gridworld::LEFT) {
        addSuccessor(state, successors, state->x(), 0,
                     state->x() - 1, state->y(), probForward);

        addSuccessor(state, successors, state->y(), 0,
                     state->x(), state->y() - 1, probSides);

        addSuccessor(state, successors, height_ - 1, state->y(),
                     state->x(), state->y() + 1, probSides);

        if (allDirections_) {
            addSuccessor(state, successors, width_ - 1, state->x(),
                         state->x() + 1, state->y(), probSides);
        }
    } else if (action->dir() == gridworld::RIGHT) {
        addSuccessor(state, successors, width_ - 1, state->x(),
                     state->x() + 1, state->y(), probForward);

        addSuccessor(state, successors, state->y(), 0,
                     state->x(), state->y() - 1, probSides);

        addSuccessor(state, successors, height_ - 1, state->y(),
                     state->x(), state->y() + 1, probSides);

        if (allDirections_) {
            addSuccessor(state, successors, state->x(), 0,
                         state->x() - 1, state->y(), probSides);
        }
    }
    return successors;
}


double GridWorldProblem::cost(mlcore::State* s, mlcore::Action* a) const
{
    if (s == absorbing)
        return 0.0;
    GridWorldState* gws = static_cast<GridWorldState *> (s);
    if (gridGoal(gws)) {
        std::pair<int,int> pos(gws->x(),gws->y());
        return (*goals_)[pos];
    }
    if (holes.count(std::pair<int, int> (gws->x(), gws->y())) != 0)
        return holeCost_ * actionCost_;
    return actionCost_;
}


bool GridWorldProblem::applicable(mlcore::State* s, mlcore::Action* a) const
{
    return true;
}


void GridWorldProblem::addSuccessor(
    GridWorldState* state, std::list<mlcore::Successor>& successors,
    int val, int limit, int newx, int newy, double prob)
{
    bool isWall = (walls.count(std::pair<int, int> (newx, newy)) != 0);
    if (val > limit && !isWall) {
        GridWorldState *next = new GridWorldState(this, newx, newy);
        successors.push_front(mlcore::Successor(this->addState(next), prob));
    } else {
        successors.push_front(mlcore::Successor(state, prob));
    }
}
