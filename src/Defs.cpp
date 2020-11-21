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

#include "Defs.hpp"

#include <stdexcept>

#include <cassert>

namespace {

bool has_consistent_width(std::initializer_list<std::initializer_list<int>>);

} // end of <anonymous> namespace

BlockGrid make_grid
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
    (const BlockGrid & grid, std::initializer_list<std::initializer_list<int>> list)
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
