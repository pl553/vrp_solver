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
    float SolveSavings(const CVRP::InstanceData& data) {
        const auto& nodes = data.nodes;
        const auto& requests = data.requests;
        const auto vehicle = data.vehicle;
        int n = nodes.size();
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
        std::list<std::list<int>> routes;
        for (int i = 1; i < n; ++i) {
            routes.push_back({i});
        }
        for (const auto& saving : savings) {
            auto it_starting_j = routes.end();
            auto it_ending_i = routes.end();
            for (auto it = routes.begin(); it != routes.end(); ++it) {
                if (it->front() == saving.node_j) {
                    it_starting_j = it;
                }
                if (it->back() == saving.node_i) {
                    it_ending_i = it;
                }
            }
            
            if (it_starting_j != routes.end() && it_ending_i != routes.end() && it_starting_j != it_ending_i) {
                it_ending_i->splice(it_ending_i->end(), *it_starting_j);
                routes.erase(it_starting_j);
            }
        }
        std::vector<float> quantities(n);
        for (const auto& req : requests) {
            quantities[req.node_id] += req.quantity;
        }
        float total_cost = 0;
        for (const auto& route : routes) {
            int prev_id = 0;
            float route_cost = 0;
            int trip_count = 0;
            float quantity = 0;
            for (int id : route) {
                route_cost += EdgeCost(nodes[prev_id], nodes[id]);
                prev_id = id;
                quantity += quantities[id];
            }
            trip_count = std::ceil(quantity / vehicle.capacity);
            route_cost += EdgeCost(nodes[prev_id], nodes[0]);
            total_cost += route_cost * trip_count;
        }
        return total_cost;
    }
}