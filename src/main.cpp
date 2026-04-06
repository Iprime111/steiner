#include "types.hpp"
#include "log.hpp"

#include <exception>
#include <nlohmann/json.hpp>
#include <CLI/CLI.hpp>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace {
steiner::AlgoCtx read_ctx(const fs::path& input_path);
void write_ctx(const fs::path& path, const steiner::AlgoCtx& ctx);
} // namespace

int main(int argc, char** argv) try {
    
    CLI::App app{};
    
    fs::path input_path;
    auto *input_opt = app.add_option("-i,--input,input", input_path, "Input file")
        ->required()->check(CLI::ExistingFile);
   
    fs::path output_path;
    auto *output_opt = app.add_option("-o,--output", output_path, "Output file");
    
    app.callback([&]() {
        if (output_opt->count() > 0) {
            return;
        }
        
        output_path = input_path;
        auto new_filename = input_path.stem().string() + "_out" + input_path.extension().string();
        output_path.replace_filename(new_filename);
    });
    
    CLI11_PARSE(app, argc, argv);
    
    auto ctx = read_ctx(input_path);
    
    // TODO actual algo
    
    write_ctx(output_path, ctx);
} catch(const std::exception& ex) {
    LOG_MESSAGE("CRITICAL ERROR\n\t{}", ex.what());
}

namespace {
steiner::AlgoCtx read_ctx(const fs::path& input_path) {
        std::ifstream file{input_path};
    if (!file) {
        LOG_CRITICAL("Unable to open: ", input_path.string());
    }
    
    json j;
    file >> j;
    return j.get<steiner::AlgoCtx>();
}

void write_ctx(const fs::path& path, const steiner::AlgoCtx& ctx) {
    std::ofstream file{path};
    if (!file) {
        LOG_CRITICAL("Unable to open: ", path.string());
    }
    
    json j = ctx;
    file << j << std::endl;
}
} // namespace