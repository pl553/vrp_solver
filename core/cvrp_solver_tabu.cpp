#include "cvrp_solver_tabu.hpp"

#include "cvrp_solver_savings.hpp"

#include <algorithm>
#include <list>
#include <random>

namespace CVRP {
    bool operator==(const Route& a, const Route& b) {
        return a.nodes == b.nodes;
    }

    bool operator==(const Solution& a, const Solution& b) {
        return a.routes == b.routes;
    }

    float GetAdjacentEdgeCost(size_t i, const std::vector<int>& route, const CVRP::InstanceData& data) {
        auto i_pt = data.nodes[route[i]];
        auto prev_i_pt = (i == 0 ? data.nodes[0] : data.nodes[route[i-1]]);
        auto next_i_pt = (i == route.size() - 1 ? data.nodes[0] : data.nodes[route[i+1]]);
        return Core::EdgeCost(prev_i_pt, i_pt) + Core::EdgeCost(i_pt, next_i_pt);
    }

    std::vector<Solution> GetNeighbors(const Solution& s0, const CVRP::InstanceData& data, std::mt19937& rng) {
        std::vector<Solution> neighbors;
        
        size_t n_routes = s0.routes.size();
        for (size_t i = 0; i < n_routes; ++i) {
            size_t n = s0.routes[i].nodes.size();

            for (size_t j = 0; j < n; ++j) {
                Solution neighbor = s0;

                auto& route_i = neighbor.routes[i];
                auto& other_route = neighbor.routes[i];
                size_t other_route_n = other_route.nodes.size();
                size_t k = rng() % other_route_n;

                float route_i_quantity_j = data.quantities[route_i.nodes[j]];
                float other_route_quantity_k = data.quantities[other_route.nodes[k]];

                if (route_i != other_route) {
                    route_i.load = route_i.load - route_i_quantity_j + other_route_quantity_k;
                    other_route.load = other_route.load - other_route_quantity_k + route_i_quantity_j;

                    if (route_i.load > data.vehicle.capacity || other_route.load > data.vehicle.capacity) {
                        continue;
                    }
                }

                float cost_delta = -GetAdjacentEdgeCost(j, route_i.nodes, data) - GetAdjacentEdgeCost(k, other_route.nodes, data);
                std::swap(route_i.nodes[j], other_route.nodes[k]);
                cost_delta += GetAdjacentEdgeCost(j, route_i.nodes, data) + GetAdjacentEdgeCost(k, other_route.nodes, data);

                neighbor.total_cost += cost_delta;
                neighbors.push_back(std::move(neighbor));
            }
        }

        return neighbors;
    }

    Solution SolveTabu(const CVRP::InstanceData& data,  size_t rng_seed, int iterations, int tabu) {
        auto s0 = CVRP::SolveSavings(data);
        auto s_best = s0;
        
        auto best_candidate = s0;
        std::list<Solution> tabu_list {s0};

        auto rng = std::mt19937(rng_seed);

        while (iterations--) {
            auto s_neighborhood = GetNeighbors(best_candidate, data, rng);
            best_candidate = s_neighborhood.front();

            for (auto s : s_neighborhood) {
                if (std::find(tabu_list.begin(), tabu_list.end(), s) == tabu_list.end() && s.total_cost < best_candidate.total_cost) {
                    best_candidate = s;
                }
            }

            if (best_candidate.total_cost < s_best.total_cost) {
                s_best = best_candidate;
            }
            tabu_list.push_back(best_candidate);
            if (tabu_list.size() > tabu) {
                tabu_list.pop_front();
            }
        }

        return s_best; 
    }
}