#include "FallingPiece.hpp"

#include <common/Util.hpp>

#include <cassert>

void FallingPiece::rotate_left(const Grid<int> & grid) {
    using VecI = VectorI;
    /**/ if (m_offset == VecI( 1,  0)) { set_rotation(grid, VecI( 0, -1)); }
    else if (m_offset == VecI( 0, -1)) { set_rotation(grid, VecI(-1,  0)); }
    else if (m_offset == VecI(-1,  0)) { set_rotation(grid, VecI( 0,  1)); }
    else if (m_offset == VecI( 0,  1)) { set_rotation(grid, VecI( 1,  0)); }
    else { throw std::runtime_error("\"impossible\" branch"); }

    check_invarients_with_positions(grid);
}

void FallingPiece::rotate_right(const Grid<int> & grid) {
    using VecI = VectorI;
    /**/ if (m_offset == VecI( 1,  0)) { set_rotation(grid, VecI( 0,  1)); }
    else if (m_offset == VecI( 0, -1)) { set_rotation(grid, VecI( 1,  0)); }
    else if (m_offset == VecI(-1,  0)) { set_rotation(grid, VecI( 0, -1)); }
    else if (m_offset == VecI( 0,  1)) { set_rotation(grid, VecI(-1,  0)); }
    else { throw std::runtime_error("\"impossible\" branch"); }

    check_invarients_with_positions(grid);
}

void FallingPiece::ascend(const Grid<int> & grid) {
    (void)move(grid, VectorI(0, -1));
    check_invarients_with_positions(grid);
}

bool FallingPiece::descend(const Grid<int> & grid) {
    bool rv = move(grid, VectorI(0, 1));
    check_invarients_with_positions(grid);
    return rv;
}

void FallingPiece::move_left(const Grid<int> & grid) {
    (void)move(grid, VectorI(-1, 0));
    check_invarients_with_positions(grid);
}

void FallingPiece::move_right(const Grid<int> & grid) {
    (void)move(grid, VectorI(1, 0));
    check_invarients_with_positions(grid);
}

void FallingPiece::set_location(VectorI r) {
    m_location = r;
    check_invarients();
}

/* private */ void FallingPiece::set_rotation(const Grid<int> & grid, VectorI offset) {
    assert(offset_ok(offset));
    auto other_location_ = m_location + offset;
    if (has_space_open(grid, other_location_)) {
        m_offset = offset;
    } else if (has_space_open(grid, m_location - offset)) {
        // wall kick
        auto old_offset = m_offset;
        m_offset = offset;
        if (!move(grid, -offset)) {
            m_offset = old_offset;
        }
    }
}

/* private */ bool FallingPiece::move(const Grid<int> & grid, VectorI offset) {
    assert(offset_ok(offset));
    if (has_space_open(grid, location      () + offset) &&
        has_space_open(grid, other_location() + offset))
    {
        m_location += offset;
        return true;
    }
    return false;
}

/* private */ void FallingPiece::check_invarients() const {
    assert(offset_ok(m_offset));
}

/* private */ void FallingPiece::check_positions_ok(const Grid<int> &) const {
#   if 0
    assert(has_space_open(grid, location()) && has_space_open(grid, other_location()));
#   endif
}

/* private */ void FallingPiece::check_invarients_with_positions(const Grid<int> & grid) const {
    check_positions_ok(grid);
    check_invarients();
}

/* private static */ bool FallingPiece::has_space_open
    (const Grid<int> & grid, VectorI r)
{
    if (!grid.has_position(r)) return false;
    return grid(r) == k_empty_block;
}

/* private static */ bool FallingPiece::offset_ok(VectorI r) {
    return (r.x == 0 && magnitude(r.y) == 1) ||
           (magnitude(r.x) == 1 && r.y == 0);
}