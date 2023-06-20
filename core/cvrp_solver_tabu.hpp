#pragma once

#include "cvrp_model.hpp"

namespace CVRP {
    Solution SolveTabu(const CVRP::InstanceData& data, size_t rng_seed, int iterations = 19, int tabu = 7);
}