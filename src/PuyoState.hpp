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

#include "BoardStates.hpp"

class Scenario;

class ScenarioPriv {
    struct DefDeleter { void operator () (Scenario *) const; };
    struct ConstDefDeleter { void operator () (const Scenario *) const; };
public:
    using UPtr      = std::unique_ptr<      Scenario, DefDeleter>;
    using ConstUPtr = std::unique_ptr<const Scenario, ConstDefDeleter>;
};

using ScenarioPtr      = ScenarioPriv::UPtr;
using ConstScenarioPtr = ScenarioPriv::ConstUPtr;

ConstScenarioPtr move(ScenarioPtr &);

class PuyoState final : public PauseableWithFallingPieceState {
public:
    // needed by Scenario
    using Response = MultiType<std::pair<int, int>, Grid<int>>;
    PuyoState() {}
    explicit PuyoState(ScenarioPtr sptr):
        m_current_scenario(std::move(sptr))
    {}
private:
    static constexpr const int k_max_possible_score = 999999;
    using UpdateFunc = void(PuyoState::*)(double);

    void setup_board(const Settings &) override;
    void update(double et) override;

    void draw(sf::RenderTarget & target, sf::RenderStates states) const override;

    int width_in_blocks () const override { return m_blocks.width () + 3; }

    int height_in_blocks() const override { return m_blocks.height(); }

    int scale() const override { return 3; }

    FallingPieceBase & piece_base() override { return m_piece; }

    const BlockGrid & blocks() const override { return m_blocks; }

    void update_piece(double);
    void update_pop_effects(double);
    void update_fall_effects(double);

    void handle_response(const Response &);

    BlockGrid m_blocks;
    Rng m_rng = Rng { std::random_device()() };
    FallEffectsFull m_fef;
    PuyoPopEffects m_pef;

    double m_fall_time = 0.;
    double m_fall_delay = 0.5;
    FallingPiece m_piece;
    std::pair<int, int> m_next_piece = std::make_pair(k_empty_block, k_empty_block);
    UpdateFunc m_update_func = &PuyoState::update_fall_effects;

    int m_pop_requirement = 4;

    ScenarioPtr m_current_scenario;

    int m_score = 0;
};
