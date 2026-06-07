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
#include <print>

namespace r = std::ranges;
namespace rv = std::ranges::views;

namespace {

    const static std::unordered_map<char, color> g_char_to_color = {
        {'G', color::green} ,
        {'L', color::lavender} ,
        {'P', color::purple} ,
        {'B', color::blue} ,
        {'W', color::white} ,
        {'C', color::cyan} ,
        {'M', color::magenta} ,
        {'O', color::orange} ,
        {'Y', color::yellow} ,
        {'R', color::red}
    };

    struct opening {
        std::string opening_str;
        game_state post_opening_state;
        std::vector<move> moves;
    };

    struct search_node {
        std::vector<move> moves;
        game_state current_state;
    };

    struct color_run {
        color col;
        int num;
    };

    auto all_colors() {
        return g_char_to_color | rv::values;
    }

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

        if (!g_char_to_color.contains(c)) {
            throw std::runtime_error(std::format("invalid color: {}", c));
        }

        return g_char_to_color.at(c);
    }

    char color_to_char(color c) {
        static std::unordered_map<color, char> color_to_char;
        if (color_to_char.empty()) {
            color_to_char = g_char_to_color | rv::transform(
                [](auto&& kvp)->std::unordered_map<color, char>::value_type {
                    const auto& [k, v] = kvp;
                    return { v,k };
                }
            ) | r::to<std::unordered_map<color, char>>();
        }

        if (!color_to_char.contains(c)) {
            throw std::runtime_error("invalid color");
        }

        return color_to_char.at(c);
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

std::optional<opening> perform_opening(const game_state& state, color opening_1, color opening_2) {

    auto [state_after_1, opening_moves_1] = consolidate_color(state, opening_1, 10);
    if (opening_moves_1.empty()) {
        return {};
    }

    auto [state_after_2, opening_moves_2] = consolidate_color(state_after_1, opening_2, 11);
    if (opening_moves_2.empty()) {
        return {};
    }

    auto moves = opening_moves_1;
    r::copy(opening_moves_2, std::back_inserter(moves));

    return opening{
        std::format("{}-{}", color_to_char(opening_1), color_to_char(opening_2)),
        state_after_2,
        moves
    };

}

std::vector<move> solve(const opening& opening) {

    auto solution_moves = solve(opening.post_opening_state);
    if (solution_moves.empty()) {
        return {};
    }

    auto all_moves = opening.moves;
    r::copy(solution_moves, std::back_inserter(all_moves));

    return all_moves;

}

std::vector<move> solve(const game_state& state, color opening_1, color opening_2) {

    auto opening = perform_opening(state, opening_1, opening_2);
    if (!opening) {
        return {};
    }

    return solve(*opening);

}

std::vector<solution_record> catalog_solutions(const game_state& state) {
    
    game_state_set visited_openings;
    std::vector<solution_record> catalog;

    int i = 0;
    std::print("\ncataloging solutions...\n  ");

    for (const auto& [c1, c2] : rv::cartesian_product(all_colors(), all_colors())) {

        std::print(".");
        if (++i % k_num_colors == 0) {
            std::print("\n  ");
        }

        if (c1 == c2) {
            continue;
        }

        auto opening_result = perform_opening(state, c1, c2);
        if (!opening_result) {
            continue;
        }
        auto tok = game_state_token(opening_result->post_opening_state);
        if (visited_openings.contains(tok)) {
            continue;
        }
        visited_openings.insert(tok);

        auto solution = solve(*opening_result);
        if (solution.empty()) {
            continue;
        }

        catalog.emplace_back(
            opening_result->opening_str,
            static_cast<int>(solution.size())
        );

    }
    std::println("");

    r::sort(catalog,
        [](const auto& lhs, const auto& rhs) {
            return lhs.num_moves < rhs.num_moves;
        }
    );

    return catalog;
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
