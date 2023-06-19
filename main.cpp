#include <iostream>
#include <fstream>
#include <iomanip>
#include <cassert>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <vector>
#include <functional>
#include <unordered_map>

#include "core/cvrp_loader.hpp"
#include "core/cvrp_solver_savings.hpp"
#include "core/cvrp_solution_verifier.hpp"

struct InstanceTestResults {
    float cost;
    float time_ms;
};

struct SolverTestResults {
    std::vector<InstanceTestResults> results;
    float total_time_ms;
};

typedef std::function<CVRP::Solution(const CVRP::InstanceData& data)> Solver;

SolverTestResults TestSolver(Solver solver, const std::vector<CVRP::InstanceData>& dataset) {
    std::vector<InstanceTestResults> results;
    float total_time_ms = 0;

    for (const auto& instance : dataset) {
        auto begin = std::chrono::high_resolution_clock::now();
        auto solution = solver(instance);
        auto end = std::chrono::high_resolution_clock::now();

        assert(CVRP::Verify(instance, solution));

        float ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
        
        results.push_back({
            .cost = solution.total_cost,
            .time_ms = ms
        });
        total_time_ms += ms;
    }

    return SolverTestResults {
        .results = std::move(results),
        .total_time_ms = total_time_ms
    };
}

void TestCVRP() {
    if (!std::filesystem::is_directory("test_data/CVRP")) {
        throw std::runtime_error("Test data folder not found");
    }

    std::ofstream results_out("results.md");

    for (auto it_dataset : std::filesystem::directory_iterator("test_data/CVRP")) {
        if (it_dataset.is_directory()) {
            std::unordered_map<std::string,float> name_bks;
            std::vector<CVRP::InstanceData> dataset;
            
            std::ifstream bks_iff(it_dataset.path() / "BKS.txt");
            while(!bks_iff.eof()) {
                std::string name;
                bks_iff >> name;
                bks_iff >> name_bks[name];
            }

            auto data_dir = std::filesystem::directory_entry(it_dataset.path() / "data");
            for (auto it_instance : std::filesystem::directory_iterator(data_dir)) {
                if (it_instance.path().extension() == ".xml") {
                    dataset.push_back(CVRP::LoadVRPREP(it_instance.path()));
                }
            }

            auto dataset_name = dataset.begin()->dataset_name;
            int n_instances = dataset.size();

            auto savings_results = TestSolver(CVRP::SolveSavings, dataset);

            std::cout << std::endl;
            std::cout << "Dataset: " << dataset_name << std::endl; 

            results_out << std::fixed << std::setprecision(2);
            results_out << "## Dataset: " << dataset_name << std::endl;
            results_out << "| Instance | Best known solution | Savings algorithm |" << std::endl;
            results_out << "| --- | --- | --- |" << std::endl;

            float savings_total_bks_diff = 0;
            for (size_t i = 0; i < dataset.size(); ++i) {
                const auto& name = dataset[i].name;
                float bks = name_bks[name];
                const auto& result = savings_results.results[i];

                float cost = result.cost;
                float time_ms = result.time_ms;

                float bks_diff = 100 * (cost - bks) / bks;
                savings_total_bks_diff += bks_diff;

                std::cout << name << std::endl;
                std::cout << "  Savings algorithm: " << cost << std::endl;
                std::cout << "    " << bks_diff << "% over BKS" << std::endl;
                std::cout << "    Time: " << time_ms << " ms" << std::endl;
                std::cout << "  Best known solution: " << bks << std::endl;

                results_out << "| " << name << " | " << bks << " | " << cost << " <br> " << bks_diff << "% over BKS <br> " << time_ms << " ms |" << std::endl;  
            }
            float savings_average_diff = savings_total_bks_diff / n_instances;
            results_out << "|  |  | Total time: " << savings_results.total_time_ms << "ms <br> Average % over BKS: " << savings_average_diff << "% |" << std::endl;
            std::cout << "Total savings algorithm time: " << savings_results.total_time_ms << " ms" << std::endl;
        }
    }
}

int main() {
    TestCVRP();
}