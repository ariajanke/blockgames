/****************************************************************************

    Copyright 2020 Aria Janke

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*****************************************************************************/

#pragma once

#include <bitset>

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Color.hpp>

#include <common/Grid.hpp>
#include <common/SubGrid.hpp>

#include <ksg/Text.hpp>

using VectorI = sf::Vector2i;
using VectorD = sf::Vector2<double>;

enum class GameSelection {
    puyo_clone, tetris_clone, samegame_clone, columns_clone,
    count
};

enum TileEdge {
    k_left_edge, k_right_edge, k_top_edge, k_bottom_edge,
    k_edge_count
};

using TileEdges = std::bitset<k_edge_count>;

template <TileEdge ... Types>
struct GetEdgeValue {
    static constexpr const unsigned k_value = 0;
};

template <TileEdge edge, TileEdge ... Types>
struct GetEdgeValue<edge, Types...> : public GetEdgeValue<Types...> {
    static constexpr const unsigned k_value =  1 << edge | GetEdgeValue<Types...>::k_value;
};

// ----------------------------- global constants -----------------------------

[[maybe_unused]] const constexpr auto k_game_list = {
    GameSelection::puyo_clone, GameSelection::tetris_clone,
    GameSelection::samegame_clone, GameSelection::columns_clone
};

[[maybe_unused]] constexpr const int k_block_size         = 16;

// block types
// [0]    : empty
// [1   5]: colored normal blocks
// [6 ...]: special blocks
#if 0
[[maybe_unused]] constexpr const int k_empty_block        =  0;
#endif
[[maybe_unused]] constexpr const int k_min_colors         =  1;
[[maybe_unused]] constexpr const int k_max_colors         =  5;
#if 0
[[maybe_unused]] constexpr const int k_glass_block        =  6;
[[maybe_unused]] constexpr const int k_hard_glass_block   =  7;
#endif
[[maybe_unused]] constexpr const int k_min_board_size     =  2;
[[maybe_unused]] constexpr const int k_max_board_size     = 30;
[[maybe_unused]] constexpr const int k_free_play_scenario = -1;

enum class BlockId {
    empty,
    red, blue, green, magenta, yellow,
    glass, hard_glass,
};

[[maybe_unused]] constexpr const auto k_empty_block = BlockId::empty;
[[maybe_unused]] constexpr const auto k_special_block_begin = static_cast<int>(BlockId::glass);

namespace BlockIdShorthand {

// the issue with the "k" prefix, is that it makes the names too samey imho
// They need to simultaneously be distinct enough from each other and short
// enough to be usable.

const constexpr auto e_ = BlockId::empty;
const constexpr auto r_ = BlockId::red;
const constexpr auto b_ = BlockId::blue;
const constexpr auto g_ = BlockId::green;
const constexpr auto m_ = BlockId::magenta;
const constexpr auto y_ = BlockId::yellow;

} // end of BlockIdShorthand namespace

[[maybe_unused]] constexpr const auto k_edge_list =
    { k_left_edge, k_right_edge, k_top_edge, k_bottom_edge };

[[maybe_unused]] constexpr const auto k_full_edge_list = {
    TileEdges(GetEdgeValue<>::k_value), // 0
    TileEdges(GetEdgeValue<k_left_edge>::k_value), // 1
    TileEdges(GetEdgeValue<            k_right_edge>::k_value), // 2
    TileEdges(GetEdgeValue<            k_right_edge, k_left_edge>::k_value), // 3
    TileEdges(GetEdgeValue<      k_top_edge>::k_value), // 4
    TileEdges(GetEdgeValue<k_left_edge, k_top_edge>::k_value), // 5
    TileEdges(GetEdgeValue<k_right_edge, k_top_edge>::k_value), // 6
    TileEdges(GetEdgeValue<k_top_edge, k_right_edge, k_left_edge>::k_value), // 7
    TileEdges(GetEdgeValue<k_bottom_edge>::k_value), // 8
    TileEdges(GetEdgeValue<k_left_edge, k_bottom_edge>::k_value), // 9
    TileEdges(GetEdgeValue<k_right_edge, k_bottom_edge>::k_value), // 10
    TileEdges(GetEdgeValue<k_bottom_edge, k_right_edge, k_left_edge>::k_value), // 11
    TileEdges(GetEdgeValue<k_top_edge, k_bottom_edge>::k_value), // 12
    TileEdges(GetEdgeValue<k_top_edge, k_bottom_edge, k_left_edge>::k_value), // 13
    TileEdges(GetEdgeValue<k_top_edge, k_bottom_edge, k_right_edge>::k_value), // 14
    TileEdges(GetEdgeValue<k_top_edge, k_right_edge, k_left_edge, k_bottom_edge>::k_value), // 15
};

template <typename T>
struct AlwaysMidRng {
    using result_type = T;
    static constexpr T min() { return T(0); }
    static constexpr T max() { return T(1); }
    T operator () () { return T(0.5); }
};

using BlockGrid         =         Grid<BlockId>;
using BlockSubGrid      =      SubGrid<BlockId>;
using ConstBlockSubGrid = ConstSubGrid<BlockId>;

BlockGrid make_grid(std::initializer_list<std::initializer_list<BlockId>>);

bool is_grid_the_same(const BlockGrid &, std::initializer_list<std::initializer_list<BlockId>>);
#if 0
inline bool is_block_color(int x)
    { return x > static_cast<int>(k_empty_block) && x < k_glass_block; }
#endif
inline bool is_block_color(BlockId bid) {
    using namespace BlockIdShorthand;
    switch (bid) {
    case r_: case g_: case b_: case m_: case y_: return true;
    default: return false;
    }
}
#if 0
inline int decay_block(int x)
    { return x == k_hard_glass_block ? k_glass_block : static_cast<int>(k_empty_block); }
#endif
inline BlockId decay_block(BlockId bid) {
    switch (bid) {
    case BlockId::glass     : return BlockId::empty;
    case BlockId::hard_glass: return BlockId::glass;
    default: return bid;
    }
}

class ColorBlockDistri {
public:
    ColorBlockDistri(int max_colors): m_max_colors(max_colors) {}
    template <typename Rng>
    BlockId operator () (Rng & rng) const {
        auto rv = static_cast<BlockId>(std::uniform_int_distribution<int>(k_min_colors, m_max_colors)(rng));
        if (is_block_color(rv)) return rv;
        throw std::runtime_error("ColorBlockDistri::operator(): failed to generate valid block color id.");
    }

private:
    int m_max_colors = k_min_colors;
};

inline BlockId map_int_to_color(int n) {
    using namespace BlockIdShorthand;
    auto verify_is_color = [](BlockId bid) {
        if (is_block_color(bid)) return bid;
        throw std::runtime_error("map_int_to_color: returning block id that is not a color.");
    };
    switch (n) {
    case 1: case 2: case 3: case 4: case 5:
        return verify_is_color(static_cast<BlockId>(n));
    default: throw std::invalid_argument("map_int_to_color: Only integers in [1 5] can be mapped to block ids.");
    }
}

using UString = ksg::Text::UString;

UString to_ustring(const std::string &);

GameSelection to_game_selection(std::size_t idx);
