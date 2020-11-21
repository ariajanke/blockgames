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
#include "PlayControl.hpp"

class Polyomino final : public FallingPieceBase {
public:
    struct Block {
        Block() {}
        Block(VectorI r, int c): offset(r), color(c) {}
        VectorI offset;
        int color = k_empty_block;
    };
    enum class Domino { l, count };
    enum class Tromino { i, l, count };
    enum class Tetromino { i, j, l, o, s, t, z, count };
    enum class Pentomino { f, i, l, n, p, t, u, v, w, x, y, z, count };

    enum class PolyominoSet { domino, tromino, tetromino, pentomino, count };

    static constexpr const int k_domino_count = static_cast<int>(Domino::count);
    static constexpr const int k_tromino_count = static_cast<int>(Tromino::count);
    static constexpr const int k_tetromino_count = static_cast<int>(Tetromino::count);
    static constexpr const int k_pentomino_count = static_cast<int>(Pentomino::count);
    static constexpr const int k_polyomino_set_count = static_cast<int>(PolyominoSet::count);
    static constexpr const int k_total_polyomino_count =
        k_domino_count + k_tromino_count + k_tetromino_count + k_pentomino_count;

    static const std::vector<Polyomino> & default_domino();
    static const std::vector<Polyomino> & default_trominos();
    static const std::vector<Polyomino> & default_tetrominos();
    static const std::vector<Polyomino> & default_pentominos();
    static const std::vector<Polyomino> & all_polyminos();

    Polyomino() {}
    explicit Polyomino(std::vector<Block> &&);

    void rotate_left(const BlockGrid &);
    void rotate_right(const BlockGrid &);
    void move_up(const BlockGrid &);
    bool move_down(const BlockGrid &);
    void move_right(const BlockGrid &);
    void move_left(const BlockGrid &);
    void set_location(int x, int y);
    void place(BlockGrid &) const;
    VectorI location() const { return m_location; }
    void set_colors(int);
    void enable_rotation() { m_rotation_enabled = true; }
    void disable_rotation() { m_rotation_enabled = false; }

    int block_count() const;
    int block_color(int) const;
    VectorI block_location(int) const;

    bool obstructed_by(const BlockGrid &) const;

private:
    using RotateFunc = VectorI(*)(VectorI);
    bool move(const BlockGrid &, VectorI & location, VectorI offset) const;
    bool rotate(const BlockGrid &, std::vector<Block> &, RotateFunc) const;

    static VectorI rotate_plus_halfpi (VectorI r) { return VectorI(-r.y, r.x); }
    static VectorI rotate_minus_halfpi(VectorI r) { return VectorI(r.y, -r.x); }

    std::vector<Block> m_blocks;
    VectorI m_location;
    bool m_rotation_enabled = true;
};

using PolyominoEnabledSet = std::bitset<Polyomino::k_total_polyomino_count>;
const char * to_string(Polyomino::PolyominoSet);
