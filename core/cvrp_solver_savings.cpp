#include "cvrp_solver_savings.hpp"

#include <algorithm>
#include <cmath>
#include <cassert>
#include <vector>
#include <list>
#include <iostream>
#include <unordered_set>

#include "cvrp_model.hpp"

std::ostream& operator<<(std::ostream& ostr, const std::list<int>& list)
{
    for (auto& i : list)
        ostr << ' ' << i;
 
    return ostr;
}

struct Saving {
    int node_i;
    int node_j;
    float s;

    bool operator<(const Saving& other) const {
        return s < other.s;
    }
};

float EdgeCost(const Core::Point& a, const Core::Point& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

namespace CVRP {
    Solution SolveSavings(const CVRP::InstanceData& data) {
        const auto& nodes = data.nodes;
        const auto& requests = data.requests;
        const auto vehicle = data.vehicle;
        int n = nodes.size();
        std::vector<float> quantities(n);
        for (const auto& request : requests) {
            quantities[request.node_id] += request.quantity;
        }
        std::vector<Saving> savings;
        savings.reserve(n * (n - 1));
        for (int i = 1; i < n; ++i) {
            for (int j = 1; j < n; ++j) {
                if (i != j) {
                    float s = EdgeCost(nodes[i], nodes[0]) + EdgeCost(nodes[0], nodes[j]) - EdgeCost(nodes[i], nodes[j]);
                    savings.push_back({i, j, s});
                }
            }
        }
        std::sort(savings.begin(), savings.end());
        std::reverse(savings.begin(), savings.end());
        std::list<std::pair<std::list<int>,float>> routes;
        for (int i = 1; i < n; ++i) {
            routes.push_back({{i}, quantities[i]});
        }
        for (const auto& saving : savings) {
            auto it_starting_j = routes.end();
            auto it_ending_i = routes.end();
            for (auto it = routes.begin(); it != routes.end(); ++it) {
                const auto& route = it->first;
                if (route.front() == saving.node_j) {
                    it_starting_j = it;
                }
                if (route.back() == saving.node_i) {
                    it_ending_i = it;
                }
            }
            
            if (it_starting_j != routes.end() && it_ending_i != routes.end() && it_starting_j != it_ending_i) {
                auto& route_i = it_ending_i->first;
                float& route_i_load = it_ending_i->second;
                auto& route_j = it_starting_j->first;
                float& route_j_load = it_starting_j->second;
                if (route_i_load + route_j_load <= vehicle.capacity) {
                    route_i.splice(route_i.end(), route_j);
                    route_i_load += route_j_load;
                    routes.erase(it_starting_j);
                }
            }
        }
        float total_cost = 0;
        std::vector<std::vector<int>> routes_sol; 
        for (const auto& p : routes) {
            routes_sol.emplace_back();
            int prev_id = 0;
            for (int id : p.first) {
                total_cost += EdgeCost(nodes[prev_id], nodes[id]);
                prev_id = id;
                routes_sol.back().push_back(id);
            }
            total_cost += EdgeCost(nodes[prev_id], nodes[0]);
        }
        return Solution {
            .routes = routes_sol,
            .total_cost = total_cost
        };
    }
}