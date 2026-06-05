#pragma once

#include <array>
#include <unordered_set>
#include <unordered_map>
#include "color_puzzle.hpp"

class game_state_token {

    friend struct game_state_token_hasher;

    std::array<uint8_t, 24> impl_;

public:

    game_state_token(const game_state& state);
    bool operator==(const game_state_token& tok) const;

};

struct game_state_token_hasher {
    size_t operator()(const game_state_token& tok) const;
};

using game_state_set = std::unordered_set<game_state_token, game_state_token_hasher>;

template<typename T>
using game_state_map = std::unordered_map<game_state_token, T, game_state_token_hasher>;