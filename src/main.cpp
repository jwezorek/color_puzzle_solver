#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
#include <string>
#include "color_puzzle.hpp"
#include "util.hpp"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc != 2 && argc != 3) {
        std::println(std::cerr, "usage: {} <puzzle.txt> [command]", argv[0]);
        return 1;
    }
    try {
        auto cmd_line = args_to_strings(argc, argv);

        std::string inp_path = cmd_line.front();
        if (!fs::exists(inp_path)) {
            std::println(std::cerr, "error: file does not exist: {}", inp_path);
            return 1;
        }

        auto color_puzzle = load_game_state(inp_path);
        if (cmd_line.size() == 1) {

            auto solution = solve(color_puzzle);
            std::println("best solution is {} moves:", solution.size());
            for (const auto& move : solution) {
                std::println("{}", move_to_string(move));
            }
            return 0;
        }

        auto arg = cmd_line.back();
        if (arg == "catalog") {
            auto catalog = catalog_solutions(color_puzzle);
            for (const auto& solution : catalog) {
                std::println("{} moves with {}", solution.num_moves, solution.opening);
            }
        } else {
            auto color_1 = color_from_char(arg.front());
            auto color_2 = color_from_char(arg.back());
            if (!color_1 || !color_2) {
                std::println(std::cerr, "error: invalid opening colors");
                return -1;
            }
            auto solution = solve(color_puzzle, *color_1, *color_2);
            if (!solution.empty()) {
                std::println("Solution is {} moves:", solution.size());
                for (const auto& move : solution) {
                    std::println("{}", move_to_string(move));
                }
            }
            else {
                std::println("no solution with {}", arg);
            }
        }
    } catch (std::runtime_error e) {
        std::println("{}", e.what());
        return -1;
    } catch (...) {
        std::println("unknown error");
        return -1;
    }

    return 0;
}