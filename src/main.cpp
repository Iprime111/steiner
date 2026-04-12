#include "CLI/CLI.hpp"
#include "algo.hpp"
#include "fmt/base.h"
#include "graph.hpp"
#include "log.hpp"
#include "timer.hpp"
#include "types.hpp"

#include <CLI/CLI.hpp>
#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <ratio>
#include <unordered_map>

namespace fs = std::filesystem;
using json = nlohmann::json;

enum class UsedAlgo {
    kBaseAlgo,
};

namespace {
steiner::JsonGraph read_json(const fs::path& input_path);
void write_json(const fs::path& path, const steiner::JsonGraph& ctx);
}  // namespace

int main(int argc, char** argv) try {
    CLI::App app{};

    bool verify = false;
    app.add_flag("-v,--verify", verify, "Check output graph.");

    bool measure_time = false;
    app.add_flag("-t,--time", measure_time, "Measure execution time.");

    std::unordered_map<UsedAlgo, std::string> algo_option{{UsedAlgo::kBaseAlgo, "Base"}};

    auto used_algo = UsedAlgo::kBaseAlgo;
    app.add_option("-a,--algorithm", used_algo, "Algorithm to be used for computation.")
        ->capture_default_str()
        ->ignore_case()
        ->transform(CLI::CheckedTransformer{algo_option});

    size_t repeats = 1;
    app.add_option("-r,--repeat", repeats, "Repeats count for better time measurement")
        ->capture_default_str();

    fs::path input_path;
    auto* input_opt =
        app.add_option("-i,--input,input", input_path, "Input file.")->required()->check(CLI::ExistingFile);

    fs::path output_path;
    auto* output_opt = app.add_option("-o,--output", output_path, "Output file.");

    app.callback([&]() {
        if (output_opt->count() > 0) {
            return;
        }

        output_path = input_path;
        auto new_filename = input_path.stem().string() + "_out" + input_path.extension().string();
        output_path.replace_filename(new_filename);
    });

    CLI11_PARSE(app, argc, argv);

    auto json_graph = read_json(input_path);
    auto graph = steiner::json_to_graph(json_graph);
    auto graph_backup = graph;

    steiner::Timer<std::chrono::duration<double, std::milli>> timer;

    {
        auto lock = timer.measure_scope();
        for (size_t repeat = 0; repeat < repeats; ++repeat) {
            graph = graph_backup;
            steiner::Basic1SteinerAlgo steiner{graph};
            steiner.compute();
        }
    }

    if (verify) {
        auto review = steiner::GraphVerifier{graph}.verify();
        LOG_MESSAGE("{}", review);

        if (!review.passed) {
            return -1;
        }
    }

    if (measure_time) {
        fmt::println("Elapsed time: {:.3f} ms", timer.result().count());
    }

    write_json(output_path, steiner::graph_to_json(graph));
} catch (const std::exception& ex) {
    LOG_MESSAGE("CRITICAL ERROR\n\t{}", ex.what());
}

namespace {
steiner::JsonGraph read_json(const fs::path& input_path) {
    std::ifstream file{input_path};
    if (!file) {
        LOG_CRITICAL("Unable to open: ", input_path.string());
    }

    json j;
    file >> j;
    return j.get<steiner::JsonGraph>();
}

void write_json(const fs::path& path, const steiner::JsonGraph& ctx) {
    std::ofstream file{path};
    if (!file) {
        LOG_CRITICAL("Unable to open: ", path.string());
    }

    json j = ctx;
    file << j << std::endl;
}
}  // namespace
