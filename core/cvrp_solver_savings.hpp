#pragma once

#include "cvrp_model.hpp"

namespace CVRP {
    Solution SolveSavings(const CVRP::InstanceData& data, bool use_sample_sort = false);
}