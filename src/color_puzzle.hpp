#pragma once

#include <vector>
#include <string>
#include <array>
#include <optional>
#include <boost/container/static_vector.hpp>

enum class color {
    green,
    lilac,
    purple,
    blue,
    white,
    cyan,
    magenta,
    orange,
    yellow,
    red
};

struct move {
    int from_tube;
    int to_tube;
};

struct solution_record {
    std::string opening;
    int num_moves;
};

constexpr int k_tube_size = 4;
constexpr int k_num_tubes = 12;

using tube = boost::container::static_vector<color, k_tube_size>;
using game_state = std::array<tube, k_num_tubes>;

game_state load_game_state(const std::string& path);
std::string move_to_string(const move& m);
std::vector<move> solve(const game_state& state);

std::vector<move> solve(const game_state& state, color opening_1, color opening_2);
std::vector< solution_record> catalog_solutions(const game_state& state);
std::optional<color> color_from_char(char c);

