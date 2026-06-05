#include "game_state_set.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <stdexcept>

namespace {

    constexpr int k_tube_capacity = 4;
    constexpr std::uint8_t k_empty_slot = 0xF;

    std::uint8_t color_to_nibble(color c) {
        auto n = static_cast<std::uint8_t>(c);

        if (n > 0x9) {
            throw std::runtime_error("invalid color value");
        }

        return n;
    }

    std::uint16_t tube_to_token(const tube& t) {
        if (t.size() > k_tube_capacity) {
            throw std::runtime_error("tube contains more than four colors");
        }

        std::uint16_t result = 0;

        for (int i = 0; i < k_tube_capacity; ++i) {
            auto nibble =
                i < static_cast<int>(t.size())
                ? color_to_nibble(t[i])
                : k_empty_slot;

            result = static_cast<std::uint16_t>((result << 4) | nibble);
        }

        return result;
    }

} // namespace

game_state_token::game_state_token(const game_state& state) {
    impl_.fill(0);

    std::array<std::uint16_t, k_num_tubes> tube_tokens{};

    for (int i = 0; i < k_num_tubes; ++i) {
        tube_tokens[i] = tube_to_token(state[i]);
    }

    // Tube order does not matter, so canonicalize the state.
    std::sort(tube_tokens.begin(), tube_tokens.end());

    // 12 tubes * 2 bytes = 24 bytes.
    // Store each uint16_t in big-endian byte order.
    for (int i = 0; i < k_num_tubes; ++i) {
        impl_[2 * i] = static_cast<std::uint8_t>((tube_tokens[i] >> 8) & 0xFF);
        impl_[2 * i + 1] = static_cast<std::uint8_t>(tube_tokens[i] & 0xFF);
    }
}

bool game_state_token::operator==(const game_state_token& tok) const {
    return impl_ == tok.impl_;
}

std::size_t game_state_token_hasher::operator()(const game_state_token& tok) const {
    // 64-bit FNV-1a hash over the canonical byte representation.
    std::uint64_t h = 14695981039346656037ull;

    for (auto b : tok.impl_) {
        h ^= b;
        h *= 1099511628211ull;
    }

    if constexpr (sizeof(std::size_t) >= sizeof(std::uint64_t)) {
        return static_cast<std::size_t>(h);
    } else {
        return static_cast<std::size_t>(h ^ (h >> 32));
    }
}