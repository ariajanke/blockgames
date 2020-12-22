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

#include "PlayControl.hpp"

#include <random>

#include <common/SubGrid.hpp>

class BoardBase;
class FallingPiece;

class ControllerState {
public:
    static constexpr const std::size_t k_id_count = static_cast<std::size_t>(PlayControlId::count);
    using IdList = std::initializer_list<PlayControlId>;
    using StatesArray = std::array<bool, k_id_count>;

    ControllerState()
        { std::fill(m_control_states.begin(), m_control_states.end(), false); }

    void update(IdList new_presses, IdList new_releases, BoardBase &);
    void update(PlayControlState, IdList new_releases, BoardBase &);
    void update_with_present_state(BoardBase &);
    bool is_pressed(PlayControlId) const;

    void update(const StatesArray &, BoardBase &);

    const StatesArray & states() const { return m_control_states; }

private:
    void update_with_present_state(StatesArray & sent, BoardBase &);
    StatesArray m_control_states;
};

class AiScript {
public:
    using StatesArray = ControllerState::StatesArray;
    virtual ~AiScript() {}

    void play_board(BoardBase & board) {
        StatesArray new_states = m_controller_state.states();
        play_board(board, new_states);
        m_controller_state.update(new_states, board);
    }

    static std::unique_ptr<AiScript> make_random_script();

    // meant for test cases
    const ControllerState & controller_state() const { return m_controller_state; }

protected:
    virtual void play_board(const BoardBase &, StatesArray &) = 0;

private:
    ControllerState m_controller_state;
};

class RandomPresses final : public AiScript {
public:
    RandomPresses() {}
private:
    using IntDistri = std::uniform_int_distribution<int>;

    void play_board(const BoardBase &, StatesArray & states) {
        static constexpr const int k_mutation_chance = 4;
        if (IntDistri(0, k_mutation_chance)(m_rng) != 0) {
            return;
        }

        using PId = PlayControlId;
        static const auto k_control_id_list = {
            PId::down,
            PId::left, PId::left, PId::left,
            PId::right, PId::right, PId::right,
            PId::rotate_left, PId::rotate_left,
            PId::rotate_right, PId::rotate_right
        };
        auto chosen_id = *(k_control_id_list.begin() + IntDistri(0, k_control_id_list.size() - 1)(m_rng));
        auto & state = states[ static_cast<std::size_t>(chosen_id) ];
        state = !state;
    }
    std::default_random_engine m_rng = std::default_random_engine { std::random_device() () };
};

class SimpleMatcher final : public AiScript {
public:
    static const VectorI k_no_location;

    void play_board(const BoardBase &, StatesArray &) override;

    static Grid<bool> compute_reachable_blocks
        (VectorI pivot, const ConstBlockSubGrid &);

    static Grid<bool> compute_reachable_blocks
        (VectorI pivot, const ConstBlockSubGrid &, Grid<bool> && reachables);

    VectorI pivot_target() const noexcept { return m_pivot_target; }
    VectorI adjacent_target() const noexcept { return m_adjacent_target; }
    int states_int() const noexcept { return m_states_int; }

private:
    void on_turn_update(const BoardBase &, StatesArray &) const;

    void rotate_adjacent_toward_destination
        (const BoardBase &, StatesArray &) const;

    void motion_toward(const BoardBase &, StatesArray &) const;

    void face_adjacent_down
        (const FallingPiece &, StatesArray &) const;

    void on_turn_change(const BoardBase &);

    // sorts closest location first (x-ways)
    static void find_reachable_ground_blocks
        (VectorI pivot, std::vector<VectorI> & accessibles,
         const ConstBlockSubGrid & blocks);

    std::vector<VectorI> m_accessibles;
    ControllerState m_controller_state;

    VectorI m_pivot_target    = k_no_location;
    VectorI m_adjacent_target = k_no_location;
    int m_states_int = 0;
};

inline std::unique_ptr<AiScript> AiScript::make_random_script() {
    return std::make_unique<SimpleMatcher>();
}
