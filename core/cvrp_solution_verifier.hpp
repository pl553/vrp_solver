#pragma once

#include "cvrp_model.hpp"

namespace CVRP {
    bool Verify(const CVRP::InstanceData& instance_data, const CVRP::Solution& solution);
}