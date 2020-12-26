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

#include "PuyoAiScript.hpp"

#include "PuyoState.hpp"

#include <common/TestSuite.hpp>

#include <iostream>

void ControllerState::update
    (IdList new_presses, IdList new_releases, BoardBase & board)
{
    using InvArg = std::invalid_argument;

    StatesArray sent;
    std::fill(sent.begin(), sent.end(), false);

    static constexpr const auto k_dupe_id_msg =
        "ControllerState::update: All ids in both lists must be unique.";
    for (const auto * list : { &new_presses, &new_releases }) {
        for (auto id : *list) {
            auto idx = static_cast<std::size_t>(id);
            if (sent[idx]) throw InvArg(k_dupe_id_msg);
            sent[idx] = true;
        }
    }

    using std::make_pair;
    std::fill(sent.begin(), sent.end(), false);
    for (auto & [list, state] : {
         make_pair(new_presses , PlayControlState::just_pressed ),
         make_pair(new_releases, PlayControlState::just_released) })
    {
        for (auto id : list) {
            board.handle_event(PlayControlEvent(id, state));
            auto idx = static_cast<std::size_t>(id);
            m_control_states[idx] = state == PlayControlState::just_pressed;
            sent[idx] = true;
        }
    }

    update_with_present_state(sent, board);
}

void ControllerState::update(PlayControlState state, IdList list, BoardBase & board) {
    switch (state) {
    case PlayControlState::still_pressed: case PlayControlState::still_released:
        throw std::invalid_argument("");
    default: break;
    }
    static constexpr const auto k_just_pressed = PlayControlState::just_pressed;
    static const IdList k_empty_list = {};
    update(state == k_just_pressed ? list : k_empty_list,
           state == k_just_pressed ? k_empty_list : list, board);
}

void ControllerState::update_with_present_state(BoardBase & board) {
    StatesArray sent;
    std::fill(sent.begin(), sent.end(), false);
    update_with_present_state(sent, board);
}

bool ControllerState::is_pressed(PlayControlId id) const {
    auto idx = static_cast<std::size_t>(id);
    if (idx >= m_control_states.size()) {
        throw std::invalid_argument("ControllerState::is_pressed: invalid id provided (corrupt parameter?).");
    }
    return m_control_states[idx];
}

void ControllerState::update(const StatesArray & new_states, BoardBase & board) {
    using Pcs = PlayControlState;
    for (std::size_t i = 0; i != k_id_count; ++i) {
        auto state = [&]() {
            if (new_states[i] == m_control_states[i]) {
                return new_states[i] ? Pcs::still_pressed : Pcs::still_released;
            } else {
                return new_states[i] ? Pcs::just_pressed : Pcs::just_released;
            }
        } ();
        if (state == Pcs::still_released) continue;
        board.handle_event(PlayControlEvent( static_cast<PlayControlId>(i), state ));
    }
    m_control_states = new_states;
}

/* private */ void ControllerState::update_with_present_state
    (StatesArray & sent, BoardBase & board)
{
    for (std::size_t i = 0; i != k_id_count; ++i) {
        if (sent[i] || !m_control_states[i]) continue;
        auto as_id = static_cast<PlayControlId>(i);
        board.handle_event(PlayControlEvent(as_id, PlayControlState::still_pressed));
    }
}

// ----------------------------------------------------------------------------

void SimpleMatcher::play_board(const BoardBase & board, StatesArray & states) {
    if (   !board.is_ready()
        || (m_pivot_target == k_no_location && m_adjacent_target == k_no_location))
    {
        on_turn_change(board);
    }
    // controller can only be used once, any more would mean more than
    // frame perfect movement.
    // therefore the following rule on AI opponents:
    // the AI player must not do anything that is not theoretically
    // impossible for a human to do (although no human has consistently
    // frame perfect timing)

    on_turn_update(board, states);
    static constexpr const auto k_down = static_cast<std::size_t>(PlayControlId::down);
    int count = std::count(states.begin(), states.end(), false);
    states[k_down] = (count >= int(ControllerState::k_id_count - 1));
#   if 1
    int mul = 1;
    m_states_int = 0;
    using Pid = PlayControlId;
    for (auto id : { Pid::left, Pid::right, Pid::rotate_left, Pid::rotate_right }) {
        if (states[static_cast<std::size_t>(id)])
            m_states_int += mul;
        mul *= 10;
    }
#   endif
}

/* static */ Grid<bool> SimpleMatcher::compute_reachable_blocks
    (VectorI pivot, const ConstBlockSubGrid & blocks)
{
    Grid<bool> reachables;
    return compute_reachable_blocks(pivot, blocks, std::move(reachables));
}

/* static */ Grid<bool> SimpleMatcher::compute_reachable_blocks
    (VectorI pivot, const ConstBlockSubGrid & blocks, Grid<bool> && reachables)
{
    reachables.clear();
    if (!blocks.has_position(pivot)) {
        return std::move(reachables);
    }

    // consider [-1 1] tiles accessible x-ways for this row unless...
    // row blocks below are occupied, then all spaces above occupied blocks
    // are accessible, so long as they are not obstructed
    reachables.set_size(blocks.width(), blocks.height(), true);
    for (VectorI r; r != blocks.end_position(); r = blocks.next(r)) {
        if (r.y >= pivot.y) break;
        reachables(r) = false;
    }

    using std::make_tuple;
    auto sweep_list = { make_tuple(pivot.x, blocks.width(), 1),
                        make_tuple(pivot.x, -1, -1) };
    for (auto [beg, lim, step] : sweep_list) {
        bool obstructed = false;
        for (int i = beg; i != lim; i += step) {
            VectorI r(i, pivot.y);
            bool floor_below = !blocks.has_position(r + VectorI(0, 1));
            if (!floor_below) floor_below = blocks(r + VectorI(0, 1)) != k_empty_block;

            if (blocks(r) != k_empty_block || obstructed) {
                obstructed = true;
                reachables(r) = false;
            } else if (magnitude(i - pivot.x) <= 1) {
                // (do nothing)
            } else if (floor_below) {
                // must be support by a "floor" and unobstructed
                // (do nothing)
            } else {
                obstructed = true;
                reachables(r) = false;
            }
        }
    }

    auto clear_col = [&blocks, &reachables](int i) {
        for (int y = 0; y != blocks.height(); ++y) {
            reachables(i, y) = false;
        }
    };
    // the rest of the board is processed as columns
    for (auto [beg, lim, step] : sweep_list) {
        bool total_obstruction = false;
        for (int i = beg; i != lim; i += step) {
            if (total_obstruction) {
                clear_col(i);
                continue;
            }
            VectorI r(i, pivot.y);
            auto below = r + VectorI(0, 1);
            // skip column
            if (!blocks.has_position(below)) break;
            if (!reachables(r) && blocks(below) != k_empty_block) {
                clear_col(i);
                total_obstruction = true;
                continue;
            }
            bool obstructed = false;
            for (int y = below.y; y != blocks.height(); ++y) {
                if (obstructed || blocks(i, y) != k_empty_block) {
                    reachables(i, y) = false;
                    obstructed = true;

                }
            }
        }
    }
    return std::move(reachables);
}

/* private */ void SimpleMatcher::on_turn_update(const BoardBase & board, StatesArray & con_states) const {
    if (m_adjacent_target != k_no_location && m_pivot_target == k_no_location) {
        // rotate adjacent until facing down, then motion toward destination
        face_adjacent_down(board.current_piece(), con_states);
        motion_toward(board, con_states);
        return;
    }
    if (m_pivot_target != k_no_location && m_adjacent_target != k_no_location) {
        // if the position is where we want it...
        // then press down
        rotate_adjacent_toward_destination(board, con_states);
    }
    if (m_pivot_target != k_no_location) {
        // motion toward destination
        motion_toward(board, con_states);
    }
}

/* private */ void SimpleMatcher::rotate_adjacent_toward_destination
    (const BoardBase & board, StatesArray & con_states) const
{
    static constexpr const auto k_rotate = PlayControlId::rotate_left;
    auto & rotate_state = con_states[static_cast<std::size_t>(k_rotate)];
    const auto & piece = board.current_piece();
    if (   (   m_pivot_target.x > m_adjacent_target.x
            && piece.location() - piece.other_location() != VectorI( 1, 0))
        || (   m_pivot_target.x < m_adjacent_target.x
            && piece.location() - piece.other_location() != VectorI(-1, 0))
    ) {
        // tap rotate (either is fine for this primitive bot
        rotate_state = !rotate_state;
    } else {
        rotate_state = false;
    }
}

/* private */ void SimpleMatcher::motion_toward(const BoardBase & board, StatesArray & con_states) const {
    const auto & piece = board.current_piece();
    auto loc  = m_pivot_target == k_no_location ? piece.other_location() : piece.location();
    auto dest = m_pivot_target == k_no_location ? m_adjacent_target : m_pivot_target;
    if (dest == k_no_location) {
        throw std::runtime_error("No suitable destination to move toward.");
    }

    static constexpr const auto k_left_idx  = static_cast<std::size_t>(PlayControlId::left );
    static constexpr const auto k_right_idx = static_cast<std::size_t>(PlayControlId::right);
    if (dest.x < loc.x) {
        con_states[k_left_idx ] = true ;
        con_states[k_right_idx] = false;
    } else if (dest.x > loc.x) {
        con_states[k_left_idx ] = false;
        con_states[k_right_idx] = true ;
    } else {
        con_states[k_left_idx] = con_states[k_right_idx] = false;
    }
}

/* private */ void SimpleMatcher::face_adjacent_down
    (const FallingPiece & piece, StatesArray & con_states) const
{
    static constexpr const auto k_rotate = PlayControlId::rotate_right;
    auto & rotate_state = con_states[static_cast<std::size_t>(k_rotate)];
    if (piece.location() - piece.other_location() == VectorI(0, -1)) {
        rotate_state = false;
    } else {
        rotate_state = !rotate_state;
    }
}

/* private */ void SimpleMatcher::on_turn_change(const BoardBase & board) {
    const auto & current_piece = board.current_piece();
    const auto & blocks        = board.blocks();
    find_reachable_ground_blocks(current_piece.location(), m_accessibles, blocks);
    auto acc_end = m_accessibles.end();

    auto do_match = [&blocks](BlockId color, VectorI r) {
        auto below = r + VectorI(0, 1);
        if (!blocks.has_position(below)) return false;
        auto get_n = [&blocks](VectorI r)
            { return blocks.has_position(r) ? blocks(r) : k_empty_block; };

        return    blocks(below) == color
               || get_n(r + VectorI( 1, 0)) == color
               || get_n(r + VectorI(-1, 0)) == color;
    };
    using namespace std::placeholders;

    auto pivot_match = std::find_if(m_accessibles.begin(), acc_end,
        std::bind(do_match, current_piece.color(), _1));

    auto adjacent_match = std::find_if(m_accessibles.begin(), acc_end,
        std::bind(do_match, current_piece.other_color(), _1));

    if (pivot_match == acc_end && adjacent_match == acc_end) {
        auto itr = std::max_element(m_accessibles.begin(), m_accessibles.end(),
                                    [](VectorI l, VectorI r) { return l.y < r.y; });
        m_pivot_target = itr == m_accessibles.end() ? k_no_location : *itr;
        m_adjacent_target = k_no_location;
    } else {
        m_pivot_target    = pivot_match    == acc_end ? k_no_location : *pivot_match   ;
        m_adjacent_target = adjacent_match == acc_end ? k_no_location : *adjacent_match;
    }
#   if 0
    auto print_vec = [](VectorI r) -> std::ostream & {
        if (r == k_no_location) {
            std::cout << "no location";
        } else {
            std::cout << r.x << ", " << r.y;
        }
        return std::cout;
    };
    std::cout << "Targeting: pivot (";
    print_vec(m_pivot_target) << ") adjacent (";
    print_vec(m_adjacent_target) << ")" << std::endl;
#   endif
}

// sorts closest location first (x-ways)
/* private static */ void SimpleMatcher::find_reachable_ground_blocks
    (VectorI pivot, std::vector<VectorI> & accessibles,
     const ConstBlockSubGrid & blocks)
{
    accessibles.clear();
    if (!blocks.has_position(pivot)) return;
    auto reachables = compute_reachable_blocks(pivot, blocks);
    for (int x = 0; x != reachables.width(); ++x) {
        for (int y = reachables.height() - 1; y != -1; --y) {
            if (reachables(x, y)) {
                accessibles.push_back(VectorI(x, y));
                break; // skip rest of column
            }
        }
    }
    std::sort(accessibles.begin(), accessibles.end(),
              [pivot](const VectorI & lhs, const VectorI & rhs)
    {
        return magnitude(lhs - pivot) < magnitude(rhs - pivot);
    });
    if (!accessibles.empty()) {
        auto itr = std::find_if(accessibles.begin(), accessibles.end(), [pivot](VectorI r) { return r.x == pivot.x; });
        if (itr == accessibles.end())
            throw "what";
    }
}
