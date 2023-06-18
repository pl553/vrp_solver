#include <iostream>
#include <iomanip>
#include <cassert>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <unordered_map>

#include "core/cvrp_loader.hpp"
#include "core/cvrp_solver_savings.hpp"
#include "core/cvrp_solution_verifier.hpp"

void TestCVRP() {
    if (!std::filesystem::is_directory("test_data/CVRP")) {
        throw std::runtime_error("Test data folder not found");
    }

    for (auto it_dataset : std::filesystem::directory_iterator("test_data/CVRP")) {
        if (it_dataset.is_directory()) {
            std::unordered_map<std::string,float> name_bks;
            std::unordered_map<std::string,const CVRP::InstanceData> name_instance_data;
            
            std::ifstream bks_iff(it_dataset.path() / "BKS.txt");
            while(!bks_iff.eof()) {
                std::string name;
                bks_iff >> name;
                bks_iff >> name_bks[name];
            }

            auto data_dir = std::filesystem::directory_entry(it_dataset.path() / "data");
            for (auto it_instance : std::filesystem::directory_iterator(data_dir)) {
                if (it_instance.path().extension() == ".xml") {
                    auto instance_data = CVRP::LoadVRPREP(it_instance.path());
                    name_instance_data.insert(std::make_pair(instance_data.name, std::move(instance_data)));
                }
            }

            std::cout << std::endl;
            std::cout << "Dataset: " << name_instance_data.begin()->second.dataset_name << std::endl; 
            float savings_total_time_ms = 0;
            for (const auto& p : name_instance_data) {
                const auto& name = p.first;
                const auto& bks = name_bks[name];

                auto begin = std::chrono::high_resolution_clock::now();
                auto solution = CVRP::SolveSavings(p.second);
                auto end = std::chrono::high_resolution_clock::now();

                float ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
                savings_total_time_ms += ms;

                float cost = solution.total_cost;
                assert(CVRP::Verify(p.second, solution));
                std::cout << name << std::endl;
                std::cout << "  Savings algorithm: " << cost << std::endl;
                std::cout << "    " << 100 * cost / bks << "% of best known solution" << std::endl;
                std::cout << "    Time: " << ms << " ms" << std::endl;
                std::cout << "  Best known solution: " << bks << std::endl;
            }
            std::cout << "Total savings algorithm time: " << savings_total_time_ms << " ms" << std::endl;
        }
    }
}

int main() {
    TestCVRP();
}