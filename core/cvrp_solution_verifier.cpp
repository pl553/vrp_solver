#include "cvrp_solution_verifier.hpp"

#include <vector>
#include <iostream>
#include <limits>

namespace CVRP {
    bool Verify(const CVRP::InstanceData& instance_data, const CVRP::Solution& solution) {
        const auto vehicle = instance_data.vehicle;
        int n = instance_data.nodes.size();
        const auto& quantities = instance_data.quantities;

        float total_cost = 0;
        std::vector<bool> visited(n, false);
        for (const auto& route : solution.routes) {
            float load = 0;
            int prev_v = 0;
            for (int v : route.nodes) {
                visited[v] = true;
                load += quantities[v];
                total_cost += Core::EdgeCost(instance_data.nodes[prev_v], instance_data.nodes[v]);
                prev_v = v;
            }
            total_cost += Core::EdgeCost(instance_data.nodes[prev_v], instance_data.nodes[0]);
            if (load > vehicle.capacity) {
                std::cout << load << ' ' << vehicle.capacity << ' ' << std::endl;
                return false;
            }
        }
        if (std::abs(total_cost - solution.total_cost) > 0.01f) {
            return false;
        }
        for (int i = 1; i < n; ++i) {
            if (!visited[i]) {
                return false;
            }
        }
        return true;
    }
}