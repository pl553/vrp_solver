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
    std::vector<InstanceTestResults> instance_results;
    float total_time_ms;
};

typedef std::function<CVRP::Solution(const CVRP::InstanceData& data)> SolverFunction;

struct Solver {
    std::string name;
    SolverFunction fun;
};

SolverTestResults TestSolver(Solver solver, const std::vector<CVRP::InstanceData>& dataset) {
    std::vector<InstanceTestResults> results;
    float total_time_ms = 0;

    for (const auto& instance : dataset) {
        auto begin = std::chrono::high_resolution_clock::now();
        auto solution = solver.fun(instance);
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
        .instance_results = std::move(results),
        .total_time_ms = total_time_ms
    };
}

void TestCVRP(std::vector<Solver> solvers) {
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

            std::vector<SolverTestResults> solver_results(solvers.size());
            for (size_t i = 0; i < solvers.size(); ++i) {
                solver_results[i] = TestSolver(solvers[i], dataset);
            }

            results_out << std::fixed << std::setprecision(2);
            results_out << "## Dataset: " << dataset_name << std::endl;
            results_out << "| Instance | Best known solution |";
            for (auto& s : solvers) {
                results_out << " " << s.name << " |";
            }
            results_out << std::endl;
            results_out << "|";
            for (size_t i = 0; i < 2 + solvers.size(); ++i) {
                results_out << " --- |";
            }
            results_out << std::endl;

            std::vector<float> total_bks_diff(solvers.size(), 0);
            for (size_t i = 0; i < dataset.size(); ++i) {
                const auto& name = dataset[i].name;
                float bks = name_bks[name];
                results_out << "| " << name << " | " << bks << " |";

                for (size_t j = 0; j < solvers.size(); ++j) {
                    const auto& result = solver_results[j].instance_results[i];

                    float cost = result.cost;
                    float time_ms = result.time_ms;

                    float bks_diff = 100 * (cost - bks) / bks;
                    total_bks_diff[j] += bks_diff;

                    results_out << " " << cost << " <br> " << bks_diff << "% over BKS <br> " << time_ms << " ms |";
                }
                results_out << std::endl;
            }
            results_out << "|  |  |";
            for (size_t i = 0; i < solvers.size(); ++i) {
                float average_diff = total_bks_diff[i] / n_instances;
                results_out << " Total time: " << solver_results[i].total_time_ms << "ms <br> Average % over BKS: " << average_diff << "% |";
            }
            results_out << std::endl;
        }
    }
}

int main() {
    Solver savings = {"Savings algorithm", [](const CVRP::InstanceData& data) {
        return CVRP::SolveSavings(data);
    }};
    Solver savings_sample_sort = {"Savings algorithm (sample sort)", [](const CVRP::InstanceData& data) {
        return CVRP::SolveSavings(data, true);
    }};
    std::vector<Solver> solvers = {savings, savings_sample_sort};
    TestCVRP(solvers);
}