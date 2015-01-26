#include "../../include/solvers/solver.h"

namespace mlsolvers
{
    std::mutex bellman_mutex;

    std::random_device rand_dev;

    std::mt19937 gen(rand_dev());

    std::uniform_real_distribution<> dis(0, 1);

    bool set_cost_v_eta = false;
}
