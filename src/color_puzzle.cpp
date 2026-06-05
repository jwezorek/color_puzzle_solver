#include "color_puzzle.hpp"
#include "util.hpp"
#include "game_state_set.hpp"
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include <format>
#include <ranges>
#include <queue>
#include <optional>
#include <algorithm>

namespace r = std::ranges;
namespace rv = std::ranges::views;

namespace {

    struct search_node {
        std::vector<move> moves;
        game_state current_state;
    };

    struct color_run {
        color col;
        int num;
    };

    std::optional<color_run> active_color_run(const tube& t) {
        if (t.empty()) {
            return {};
        }
        auto active_color = t.back();
        int count = 0;
        for (auto i = t.rbegin(); i != t.rend(); ++i) {
            if (*i != active_color) {
                break;
            }
            ++count;
        }

        return color_run{
            active_color,
            count
        };
    }

    std::vector<std::tuple<color, color>> all_openings(const game_state& state) {
        auto colors = state | rv::take(k_num_tubes - 2) | rv::transform(
                [](const auto& tube)->std::optional<color> {
                    auto active = active_color_run(tube);
                    if (!active) {
                        return {};
                    }
                    return active->col;
                }
            ) | rv::filter(
                [](auto&& c) {
                    return c.has_value();
                }
            ) | rv::transform(
                [](auto&& c) {
                    return c.value();
                }
            ) | r::to<std::unordered_set<color>>() | r::to<std::vector>();

        return two_combinations(colors);
    }

    bool does_color_run_fit(const tube& tube, const color_run& run) {
        if (tube.empty()) {
            return true;
        }
        auto active_color = tube.back();
        if (run.col != active_color) {
            return false;
        }

        return k_tube_size - tube.size() >= run.num;
    }

    bool is_legal_move(const tube& from_tube, const tube& to_tube) {
        auto active = active_color_run(from_tube);
        if (!active) {
            return false;
        }
        return does_color_run_fit(to_tube, *active);
    }

    bool is_monochromatic(const tube& t) {
        if (t.empty()) {
            return true;
        }
        auto color = t.front();
        for (const auto& col : t) {
            if (col != color) {
                return false;
            }
        }
        return true;
    }

    bool is_solution(const game_state& state) {

        for (const auto& tube : state) {
            if (!is_monochromatic(tube)) {
                return false;
            }
            if (tube.size() != 0 && tube.size() != k_tube_size) {
                return false;
            }
        }

        return true;
    }

    std::vector<move> all_legal_moves(const game_state& state) {
        return rv::cartesian_product(
            rv::iota(0, k_num_tubes),
            rv::iota(0, k_num_tubes)
        ) | rv::filter(
            [&](const auto& pair) {
                auto [i, j] = pair;
                if (i == j) {
                    return false;
                }
                return is_legal_move(state[i], state[j]);
            }
        ) | rv::transform(
            [](const auto& pair)->move {
                auto [i, j] = pair;
                return { i, j };
            }
        ) | r::to<std::vector>();
    }


    std::vector<move> append_move(const std::vector<move>& moves, move m) {
        std::vector<move> out = moves;
        out.push_back(m);
        return out;
    }

    void make_move(tube& from, tube& to) {
        auto run = active_color_run(from);
        for (int i = 0; i < run->num; ++i) {
            from.pop_back();
            to.push_back(run->col);
        }
    }
    
    game_state make_move(const game_state& state, move m) {
        auto new_state = state;
        make_move(new_state[m.from_tube], new_state[m.to_tube]);
        return new_state;
    }

    color char_to_color(char c) {
        const static std::unordered_map<char, color> map = {
            {'G', color::green} ,
            {'L', color::lilac} ,
            {'P', color::purple} ,
            {'B', color::blue} ,
            {'W', color::white} ,
            {'C', color::cyan} ,
            {'M', color::magenta} ,
            {'O', color::orange} ,
            {'Y', color::yellow} ,
            {'R', color::red}
        };

        if (!map.contains(c)) {
            throw std::runtime_error(std::format("invalid color: {}", c));
        }

        return map.at(c);
    }

    char color_to_char(color c) {
        const static std::unordered_map<color,char> map = {
            {color::green  , 'G' } ,
            {color::lilac  , 'L' } ,
            {color::purple , 'P' } ,
            {color::blue   , 'B' } ,
            {color::white  , 'W' } ,
            {color::cyan   , 'C' } ,
            {color::magenta, 'M' } ,
            {color::orange , 'O' } ,
            {color::yellow , 'Y' } ,
            {color::red    , 'R' }
        };

        if (!map.contains(c)) {
            throw std::runtime_error("invalid color");
        }

        return map.at(c);
    }

    tube to_tube(const std::string& str) {

        return str | rv::filter(
            [](char ch) {
                return std::isalpha(ch);
            }
        ) | rv::transform(char_to_color) | r::to<tube>();
    }

    std::tuple<game_state, std::vector<move>> consolidate_color(
            const game_state& start, color col, int dest) {
        auto state = start;
        std::vector<move> moves;
        for (int i = 0; i < k_num_tubes; ++i) {
            if (i == dest) {
                continue;
            }
            auto& tube = state[i];
            if (tube.empty() || tube.back() != col) {
                continue;
            }
            make_move(tube, state[dest]);
            moves.emplace_back(i, dest);
        }

        return { state, moves };
    }
}

game_state load_game_state(const std::string& path){
    return to_array<tube, k_num_tubes>(
        file_to_string_vector(path) | rv::transform(to_tube) | r::to<std::vector>()
    );
}

std::string move_to_string(const move& m) {
    return std::format("{} -> {}", m.from_tube + 1, m.to_tube + 1);
}

std::vector<move> solve(const game_state& state) {
    std::queue<search_node> queue;
    game_state_set visited;

    queue.push(search_node{ {}, state });

    while (!queue.empty()) {
        auto current = queue.front();
        queue.pop();

        auto tok = game_state_token(current.current_state);
        if (visited.contains(tok)) {
            continue;
        }
        visited.insert(tok);

        if (is_solution(current.current_state)) {
            return current.moves;
        }

        auto moves = all_legal_moves(current.current_state);
        for (const auto move : moves) {
            queue.push(
                search_node{
                    append_move(current.moves, move),
                    make_move(current.current_state, move)
                }
            );
        }
    }

    return {};

}

std::vector<move> solve(const game_state& state, color opening_1, color opening_2) {
    auto [new_state, opening_moves_1] = consolidate_color(state, opening_1, 10);
    auto [new_state_2, opening_moves_2] = consolidate_color(new_state, opening_2, 11);
    auto moves = solve(new_state_2);

    if (moves.empty()) {
        return {};
    }
    return std::vector{ 
            opening_moves_1, opening_moves_2, moves
        } | rv::join | r::to<std::vector>();
}

std::vector<solution_record> catalog_solutions(const game_state& state) {
    auto solutions = all_openings(state) |
        rv::transform(
            [&](auto&& opening)->solution_record {
                const auto& [color_1, color_2] = opening;
                auto solution = solve(state, color_1, color_2);
                auto opening_str = std::format("{}-{}",
                    color_to_char(color_1),
                    color_to_char(color_2)
                );
                return { opening_str, static_cast<int>(solution.size()) };
            }
        ) | rv::filter(
            [](const auto& sol_rec) {
                return sol_rec.num_moves > 0;
            }
        ) | r::to<std::vector>();

    r::sort(solutions,
        [](const auto& lhs, const auto& rhs) {
            return lhs.num_moves < rhs.num_moves;
        }
    );

    return solutions;
}

std::optional<color> color_from_char(char c) {
    color col;
    try {
        col = char_to_color(c);
    } catch (...) {
        return {};
    }
    return col;
}
