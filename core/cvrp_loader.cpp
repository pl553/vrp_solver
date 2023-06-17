#include <tinyxml2.h>

#include <algorithm>
#include <string>
#include <stdexcept>
#include <vector>
#include <unordered_map>
#include <utility>

#include "cvrp_loader.hpp"

namespace CVRP {
    const CVRP::InstanceData LoadVRPREP(const std::string& path) {
        tinyxml2::XMLDocument doc;
        doc.LoadFile(path.c_str());

        auto instance = doc.FirstChildElement("instance");

        if (instance == nullptr) {
            throw std::runtime_error("root instance element not found");
        }

        auto network = instance->FirstChildElement("network");

        if (network == nullptr) {
            throw std::runtime_error("network element not found");
        }

        auto nodes = network->FirstChildElement("nodes");

        if (nodes == nullptr) {
            throw std::runtime_error("nodes element not found");
        }

        std::unordered_map<int, int> id_to_index;
        std::vector<std::pair<Core::Point, int>> points_ids;
        auto node_i = nodes->FirstChildElement();
        for (int i = 0; node_i != nullptr; ++i, node_i = node_i->NextSiblingElement()) {
            int id = node_i->IntAttribute("id");
            auto cx = node_i->FirstChildElement("cx");
            auto cy = node_i->FirstChildElement("cy");
            points_ids.push_back(std::make_pair(Core::Point{cx->FloatText(), cy->FloatText()}, id));
            id_to_index[id] = i;
        }

        auto vehicle_profile = instance->FirstChildElement("fleet")->FirstChildElement("vehicle_profile");
        int depot = vehicle_profile->FirstChildElement("departure_node")->IntText();
        float capacity = vehicle_profile->FirstChildElement("capacity")->FloatText();
        Vehicle vehicle { .capacity = capacity };
        
        if (points_ids.size() > 0) {
            std::swap(id_to_index[depot], id_to_index[points_ids[0].second]);
            std::swap(points_ids[id_to_index[depot]], points_ids[0]);    
        }

        std::vector<CVRP::Request> requests;
        auto requests_element = instance->FirstChildElement("requests");
        for (auto request_i = requests_element->FirstChildElement(); request_i != nullptr; request_i = request_i->NextSiblingElement()) {
            int node_id = request_i->IntAttribute("node");
            int node_index = id_to_index[node_id];
            float quantity = request_i->FirstChildElement("quantity")->FloatText();
            requests.push_back({.node_id = node_index, .quantity = quantity});
        }

        std::vector<Core::Point> points;
        for (const auto& p : points_ids) {
            points.push_back(p.first);
        }

        auto info = instance->FirstChildElement("info");
        auto name = info->FirstChildElement("name")->GetText();
        auto dataset_name = info->FirstChildElement("dataset")->GetText();

        return CVRP::InstanceData {
            .name = std::move(name),
            .dataset_name = std::move(dataset_name),
            .nodes = std::move(points),
            .requests = std::move(requests),
            .vehicle = vehicle
        };
    }
}