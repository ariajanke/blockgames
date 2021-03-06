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

#include "Polyomino.hpp"

#include <cassert>

namespace {
#if 0
using BlockId = Polyomino::Block;
#endif
Polyomino to_polyomino(std::initializer_list<VectorI> &&);

} // end of <anonymous> namespace

/* static */ const std::vector<Polyomino> & Polyomino::default_domino() {
    static std::vector<Polyomino> rv;
    if (!rv.empty()) return rv;

    using Vec = VectorI;
    using Do  = Domino ;
    rv.resize(Polyomino::k_domino_count);
    rv[static_cast<int>(Do::l)] = to_polyomino({ Vec(0, 0), Vec(0, -1) });
    return rv;
}

/* static */ const std::vector<Polyomino> & Polyomino::default_trominos() {
    static std::vector<Polyomino> rv;
    if (!rv.empty()) return rv;

    using Vec = VectorI;
    using Tr  = Tromino;
    rv.resize(Polyomino::k_tromino_count);
    rv[static_cast<int>(Tr::i)] = to_polyomino({
        Vec(0, -1),
        Vec(0,  0),
        Vec(0,  1)
    });
    rv[static_cast<int>(Tr::l)] = to_polyomino({
        Vec(0, -1),
        Vec(0,  0), Vec(1, 0)
    });
    return rv;
}

/* static */ const std::vector<Polyomino> & Polyomino::default_tetrominos() {
    using Vec = VectorI;
    using Tetromino = Polyomino::Tetromino;
    static std::vector<Polyomino> rv;
    if (!rv.empty()) return rv;
    rv.resize(Polyomino::k_tetromino_count);
    // I-Block
    rv[static_cast<int>(Tetromino::i)] = to_polyomino({
        Vec(0, -1),
        Vec(0,  0),
        Vec(0,  1),
        Vec(0,  2)
    });
    // J-Block
    rv[static_cast<int>(Tetromino::j)] = to_polyomino({
                     Vec(0, -1),
                     Vec(0,  0),
        Vec(-1,  1), Vec(0,  1)
    });
    // L-Block
    rv[static_cast<int>(Tetromino::l)] = to_polyomino({
        Vec(0, -1),
        Vec(0,  0),
        Vec(0,  1), Vec(1, 1)
    });
    // O-Block
    rv[static_cast<int>(Tetromino::o)] = to_polyomino({
        Vec(0,  0), Vec(1,  0),
        Vec(0,  1), Vec(1,  1)
    });
    rv[static_cast<int>(Tetromino::o)].disable_rotation();
    // S-Block
    rv[static_cast<int>(Tetromino::s)] = to_polyomino({
                     Vec(0, -1), Vec(1, -1),
        Vec(-1,  0), Vec(0,  0),
    });
    // T-Block
    rv[static_cast<int>(Tetromino::t)] = to_polyomino({
                     Vec(0, -1),
        Vec(-1,  0), Vec(0,  0), Vec(1,  0),
    });
    // Z-Block
    rv[static_cast<int>(Tetromino::z)] = to_polyomino({
        Vec(-1, -1), Vec(0, -1),
                     Vec(0,  0), Vec(1,  0)
    });
    return rv;
}

/* static */ const std::vector<Polyomino> & Polyomino::default_pentominos() {
    static std::vector<Polyomino> rv;
    if (!rv.empty()) return rv;
    rv.resize(k_pentomino_count);
    auto get_pm = [](Pentomino p) -> Polyomino &
        { return rv.at(static_cast<std::size_t>(p)); };
    using Vec = VectorI;
    using Pe = Pentomino;
    get_pm(Pe::f) = to_polyomino({
                    Vec(0, -1), Vec(1, -1),
        Vec(-1, 0), Vec(0,  0),
                    Vec(0,  1)
    });
    get_pm(Pe::i) = to_polyomino({
        Vec(0, -2),
        Vec(0, -1),
        Vec(0,  0),
        Vec(0,  1),
        Vec(0,  2)
    });
    get_pm(Pe::l) = to_polyomino({
        Vec(0, -3),
        Vec(0, -2),
        Vec(0, -1),
        Vec(0,  0), Vec(1,  0),
    });
#   if 0
    Grid<int> g;
    g.set_size(10, 20);
    assert(!get_pm(Pe::l).obstructed_by(g));
#   endif
    get_pm(Pe::n) = to_polyomino({
                    Vec(1, -2),
                    Vec(1, -1),
        Vec(0,  0), Vec(1,  0),
        Vec(0,  1)
    });
    get_pm(Pe::p) = to_polyomino({
        Vec(0, -1), Vec(1, -1),
        Vec(0,  0), Vec(1,  0),
        Vec(0,  1),
    });
    get_pm(Pe::t) = to_polyomino({
        Vec(-1, 0), Vec(0, 0), Vec(1, 0),
                    Vec(0, 1),
                    Vec(0, 2)
    });
    get_pm(Pe::u) = to_polyomino({
        Vec(-1, -1),            Vec(1, -1),
        Vec(-1,  0), Vec(0, 0), Vec(1,  0)
    });
    get_pm(Pe::v) = to_polyomino({
        Vec(0, -2),
        Vec(0, -1),
        Vec(0,  0), Vec(1, 0), Vec(2, 0)
    });
    get_pm(Pe::w) = to_polyomino({
        Vec(-1, -1),
        Vec(-1,  0), Vec(0, 0),
                     Vec(0, 1), Vec(1, 1)
    });
    get_pm(Pe::x) = to_polyomino({
                    Vec(0, -1),
        Vec(-1, 0), Vec(0,  0), Vec(1, 0),
                    Vec(0,  1)
    });
    get_pm(Pe::x).disable_rotation();
    get_pm(Pe::y) = to_polyomino({
                    Vec(0, -1),
        Vec(-1, 0), Vec(0,  0),
                    Vec(0,  1),
                    Vec(0,  2)
    });
    get_pm(Pe::z) = to_polyomino({
        Vec(-1, -1), Vec(0, -1),
                     Vec(0,  0),
                     Vec(0,  1), Vec(1, 1)
    });
    return rv;
}

/* static */ const std::vector<Polyomino> & Polyomino::all_polyminos() {
    static std::vector<Polyomino> rv;
    if (!rv.empty()) return rv;
    rv.reserve(k_total_polyomino_count);
    auto insert_cont = [](const std::vector<Polyomino> & cont)
        { rv.insert(rv.end(), cont.begin(), cont.end()); };
    insert_cont(default_domino    ());
    insert_cont(default_trominos  ());
    insert_cont(default_tetrominos());
    insert_cont(default_pentominos());
    return rv;
}

Polyomino::Polyomino(std::vector<Block> && blocks):
    m_blocks(std::move(blocks))
{}

void Polyomino::rotate_left(const BlockGrid & grid)
    { (void)rotate(grid, m_blocks, rotate_plus_halfpi); }

void Polyomino::rotate_right(const BlockGrid & grid)
    { (void)rotate(grid, m_blocks, rotate_minus_halfpi); }

void Polyomino::move_up(const BlockGrid & grid)
    { (void)move(grid, m_location, VectorI(0, -1)); }

bool Polyomino::move_down(const BlockGrid & grid)
    { return move(grid, m_location, VectorI(0, 1)); }

void Polyomino::move_right(const BlockGrid & grid)
    { (void)move(grid, m_location, VectorI(1, 0)); }

void Polyomino::move_left(const BlockGrid & grid)
    { (void)move(grid, m_location, VectorI(-1, 0)); }

void Polyomino::set_location(int x, int y)
    { m_location = VectorI(x, y); }

void Polyomino::place(BlockGrid & grid) const {
    for (const auto & block : m_blocks) {
        auto loc = block.offset + m_location;
        if (!grid.has_position(loc)) continue;
        grid(loc) = block.color;
    }
}

void Polyomino::set_colors(BlockId i)
    { for (auto & block : m_blocks) { block.color = i; } }

int Polyomino::block_count() const
    { return int(m_blocks.size()); }

BlockId Polyomino::block_color(int i) const
    { return m_blocks[std::size_t(i)].color; }

VectorI Polyomino::block_location(int i) const
    { return m_blocks[std::size_t(i)].offset + m_location; }

bool Polyomino::obstructed_by(const BlockGrid & grid) const {
    for (const auto & block : m_blocks) {
        auto r = block.offset + m_location;
        if (!grid.has_position(r)) continue;
        if (grid(r) != k_empty_block) return true;
    }
    return false;
}

/* private */ bool Polyomino::move
    (const BlockGrid & grid, VectorI & location, VectorI offset) const
{
    // all block new locations must be cleared

    // exception to movement behavior added:
    // a polyomino may move if fewer blocks fall outside of the board

    int old_inside_count = 0;
    int new_inside_count = 0;
    for (const auto & block : m_blocks) {
        if (grid.has_position(location + block.offset         )) ++old_inside_count;
        if (grid.has_position(location + block.offset + offset)) ++new_inside_count;
    }

    if (new_inside_count < old_inside_count) {
        return false;
    }

    for (const auto & block : m_blocks) {
        auto loc = location + offset + block.offset;
        if (!grid.has_position(loc)) continue;
        if (grid(loc) != k_empty_block) return false;
    }
    location += offset;
    return true;
}

/* private */ bool Polyomino::rotate
    (const BlockGrid & grid, std::vector<Block> & blocks, RotateFunc rotf) const
{
    if (!m_rotation_enabled) return false;
    for (const auto & block : m_blocks) {
        auto loc = m_location + rotf(block.offset);
        // would like to "kick" here also
        if (!grid.has_position(loc)) return false;
        if (grid(loc) != k_empty_block) return false;
    }
    for (auto & block : blocks) {
        // this may need to be "fixed"
        block.offset = rotf(block.offset);
    }
    return true;
}

const char * to_string(Polyomino::PolyominoSet set) {
    using Ps = Polyomino::PolyominoSet;
    switch (set) {
    case Ps::domino   : return "Domino"   ;
    case Ps::tromino  : return "Tromino"  ;
    case Ps::tetromino: return "Tetromino";
    case Ps::pentomino: return "Pentomino";
    default: throw std::invalid_argument("to_string: not a valid polyomino set.");
    }
}

namespace {

using PolyBlock = Polyomino::Block;

std::vector<PolyBlock> to_blocks(std::initializer_list<VectorI> &&);

Polyomino to_polyomino(std::initializer_list<VectorI> && offsets)
    { return Polyomino(to_blocks(std::move(offsets))); }

std::vector<PolyBlock> to_blocks(std::initializer_list<VectorI> && offsets) {
    std::vector<PolyBlock> rv;
    rv.reserve(offsets.size());
    for (auto r : offsets) {
        rv.emplace_back(r, k_empty_block);
    }
    return rv;
}

// ----------------------------------------------------------------------------

const auto & def_init = Polyomino::all_polyminos();

} // end of <anonymous> namespace
