#pragma once

#include "core_model.hpp"

#include <vector>
#include <memory>
#include <iostream>

namespace CVRP {
    struct Request {
        int node_id;
        float quantity;
    };

    struct Vehicle {
        float capacity;
    };

    struct InstanceData {
        std::string name;
        std::string dataset_name;
        std::vector<Core::Point> nodes;
        std::vector<float> quantities;
        Vehicle vehicle;
    };

    struct Route {
        std::vector<int> nodes;
        float load;
    };

    struct Solution {
        std::vector<Route> routes;
        float total_cost;
    };
}