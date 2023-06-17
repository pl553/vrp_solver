#pragma once

#include <memory>

#include "cvrp_model.hpp"

namespace CVRP {
    std::unique_ptr<const CVRP::InstanceData> LoadVRPREP(const std::string& path);
}