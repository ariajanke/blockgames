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

#include "Defs.hpp"

class FallingPiece {
public:
    FallingPiece() {}

    FallingPiece(int color_, int other_color_):
        m_color(color_), m_other_color(other_color_) {}

    VectorI location() const { return m_location; }
    int color() const { return m_color; }

    VectorI other_location() const { return m_location + m_offset; }
    int other_color() const { return m_other_color; }

    void rotate_left(const Grid<int> &);
    void rotate_right(const Grid<int> &);

    void ascend(const Grid<int> &);
    /// @returns true is decention was successful
    bool descend(const Grid<int> &);
    void move_left(const Grid<int> &);
    void move_right(const Grid<int> &);

    // hard move, without regard to the state of the board
    void set_location(VectorI);

private:
    void set_rotation(const Grid<int> &, VectorI offset);
    bool move(const Grid<int> &, VectorI offset);
    void check_invarients() const;
    void check_positions_ok(const Grid<int> &) const;

    void check_invarients_with_positions(const Grid<int> &) const;

    static bool has_space_open(const Grid<int> &, VectorI);
    // both rotation and translation
    static bool offset_ok(VectorI);

    VectorI m_location;
    VectorI m_offset = VectorI(0, -1);
    int m_color = k_empty_block;
    int m_other_color = k_empty_block;
};
