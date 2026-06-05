#pragma once

#include <vector>
#include <string>
#include <array>
#include <ranges>
#include <stdexcept>

std::vector<std::string> file_to_string_vector(const std::string& filename);

template <class T, std::size_t N>
std::array<T, N> to_array(std::span<const T> s) {
    if (s.size() != N) {
        throw std::runtime_error("wrong size");
    }

    std::array<T, N> result;
    std::ranges::copy(s, result.begin());
    return result;
}

std::vector<std::string> args_to_strings(int argc, char* argv[]);

template <class T>
auto two_combinations(const std::vector<T>& items)-> std::vector<std::tuple<T, T>>
{
    std::vector<std::tuple<T, T>> result;

    const auto n = items.size();
    result.reserve(n * (n - 1) / 2);

    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = i + 1; j < n; ++j) {
            result.emplace_back(items[i], items[j]);
        }
    }

    return result;
}