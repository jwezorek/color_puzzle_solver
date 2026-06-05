#include "util.hpp"
#include <stdexcept>
#include <fstream>
#include <ranges>
#include <sstream>

namespace r = std::ranges;
namespace rv = std::ranges::views;

namespace {


}

std::vector<std::string> file_to_string_vector(const std::string& filename) {
    std::vector<std::string> v;

    std::ifstream fs(filename);
    if (!fs) {
        throw std::runtime_error("bad file");
    }

    std::string line;
    while (std::getline(fs, line)) {
        v.push_back(line);
    }
    return v;
}

std::vector<std::string> args_to_strings(int argc, char* argv[]) {
    return rv::iota(1, argc) | rv::transform(
        [&](auto&& i)->std::string { return argv[i]; }
    ) | r::to<std::vector>();
}
