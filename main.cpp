#include <iostream>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <unordered_map>

#include "core/cvrp_loader.hpp"
#include "core/cvrp_solver_savings.hpp"

void TestCVRP() {
    if (!std::filesystem::is_directory("test_data/CVRP")) {
        throw std::runtime_error("Test data folder not found");
    }

    for (auto it_dataset : std::filesystem::directory_iterator("test_data/CVRP")) {
        if (it_dataset.is_directory()) {
            std::unordered_map<std::string,float> name_bks;
            std::unordered_map<std::string,std::unique_ptr<const CVRP::InstanceData>> instance_data;
            
            std::ifstream bks_iff(it_dataset.path() / "BKS.txt");
            while(!bks_iff.eof()) {
                std::string name;
                bks_iff >> name;
                bks_iff >> name_bks[name];
            }

            auto data_dir = std::filesystem::directory_entry(it_dataset.path() / "data");
            for (auto it_instance : std::filesystem::directory_iterator(data_dir)) {
                if (it_instance.path().extension() == ".xml") {
                    auto instance_data_ptr = CVRP::LoadVRPREP(it_instance.path());
                    instance_data[instance_data_ptr->name] = std::move(instance_data_ptr);
                }
            }

            std::cout << "Dataset: " << instance_data.begin()->second->dataset_name << std::endl; 
            for (const auto& p : instance_data) {
                const auto& name = p.first;
                const auto& bks = name_bks[name];
                float cost = CVRP::SolveSavings(*p.second);
                std::cout << name << std::endl;
                std::cout << "  Savings algorithm: " << cost << ", " << 100 * cost / bks << "% of best known solution" << std::endl;
                std::cout << "  Best known solution: " << bks << std::endl;
            }
        }
    }
}

int main() {
    TestCVRP();
}