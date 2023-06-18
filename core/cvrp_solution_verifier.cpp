#include "cvrp_solution_verifier.hpp"

#include <vector>
#include <iostream>

namespace CVRP {
    bool Verify(const CVRP::InstanceData& instance_data, const CVRP::Solution& solution) {
        const auto& requests = instance_data.requests;
        const auto vehicle = instance_data.vehicle;
        int n = instance_data.nodes.size();
        std::vector<float> quantities(n);
        for (const auto& request : requests) {
            quantities[request.node_id] += request.quantity;
        }
        std::vector<bool> visited(n, false);
        for (const auto& route : solution.routes) {
            float load = 0;
            for (int v : route) {
                visited[v] = true;
                load += quantities[v];
            }
            if (load > vehicle.capacity) {
                return false;
            }
        }
        for (int i = 1; i < n; ++i) {
            if (!visited[i]) {
                return false;
            }
        }
        return true;
    }
}