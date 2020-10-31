#include "Defs.hpp"

#include <stdexcept>

#include <cassert>

namespace {

bool has_consistent_width(std::initializer_list<std::initializer_list<int>>);

} // end of <anonymous> namespace
#if 0
sf::Color to_sfcolor(int color) {
    switch (color) {
    case k_empty_block:
        throw std::invalid_argument("to_sfcolor: Empty block has no color");
    case 1: return sf::Color(230,  70,  70);
    case 2: return sf::Color( 70, 230,  70);
    case 3: return sf::Color( 70,  70, 230);
    case 4: return sf::Color(230, 230,  70);
    case 5: return sf::Color(230,  70, 230);
    }
    throw std::invalid_argument("to_sfcolor: invalid color");
}

sf::IntRect texture_rect_for(TileEdges edges) {
    auto val = edges.to_ulong();
    VectorI u(val % 4, val / 4);
    return sf::IntRect(u.x*k_block_size, u.y*k_block_size,
                       k_block_size, k_block_size);
}
#endif
Grid<int> make_grid
    (std::initializer_list<std::initializer_list<int>> list)
{
    assert(has_consistent_width(list));
    int last_width = -1;
    for (auto sublist : list) {
        if (last_width == -1) {
            last_width = sublist.size();
        } else {
            assert(last_width == int(sublist.size()));
        }
    }
    Grid<int> rv;
    rv.set_size(last_width, list.size());
    VectorI r;
    for (auto sublist : list) {
        for (int i : sublist) {
            assert(r != rv.end_position());
            rv(r) = i;
            r = rv.next(r);
        }
    }
    return rv;
}

bool is_grid_the_same
    (const Grid<int> & grid, std::initializer_list<std::initializer_list<int>> list)
{
    assert(has_consistent_width(list));
    if (grid.height() != int(list.size())) return false;
    if (grid.height() == 0) return true;
    if (grid.width() != int(list.begin()->size())) return false;
    VectorI r;
    for (auto sublist : list) {
        for (int i : sublist) {
            assert(r != grid.end_position());
            if (grid(r) != i) return false;
            r = grid.next(r);
        }
    }
    return true;
}

UString to_ustring(const std::string & str) {
    UString rv;
    rv.reserve(str.length());
    for (auto c : str) rv.push_back(UString::value_type(c));
    return rv;
}

GameSelection to_game_selection(std::size_t idx) {
    if (idx >= static_cast<std::size_t>(GameSelection::count)) {
        throw std::invalid_argument("to_game_selection: parameter does not map to a game.");
    }
    return static_cast<GameSelection>(idx);
}

namespace {

bool has_consistent_width(std::initializer_list<std::initializer_list<int>> list) {
    int last_width = -1;
    for (auto sublist : list) {
        if (last_width == -1) {
            last_width = sublist.size();
        } else {
            if (last_width != int(sublist.size())) return false;
        }
    }
    return true;
}

} // end of <anonymous> namespace
