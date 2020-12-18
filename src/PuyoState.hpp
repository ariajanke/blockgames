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

#include "PuyoAiScript.hpp"
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

// ScoreBoardBase                               <--*
// Base <- Pause (features) <- Basic (features) <--+-- With Score Board
//                                              <--- Without Score Board

class BoardBase : public sf::Drawable {
public:
    using ColorPair = std::pair<int, int>;
    static const ColorPair k_empty_pair;
    int width() const { return blocks().width(); }
    int height() const { return blocks().height(); }

    virtual const BlockGrid & blocks() const = 0;
    // false if between turns
    virtual bool is_ready() const = 0;
    virtual bool is_gameover() const = 0;

    virtual const FallingPiece & current_piece() const = 0;
    virtual ColorPair next_piece() const = 0;

    virtual void handle_event(PlayControlEvent) = 0;

protected:
    BoardBase() {}
    virtual ~BoardBase() {}
};

// copy of: PauseableWithFallingPieceState
// I'm going to adopt it here for puyo state *before* implementing it anywhere
// else
class PauseableBoard : public BoardBase {
public:
    static constexpr const double k_fast_fall  = 5.;
    static constexpr const double k_slow_fall  = 1.;
    static constexpr const double k_move_delay = 1. / 8.;

    virtual void update(double et);

    void handle_event(PlayControlEvent) override;

    void assign_pause_pointer(bool & bptr) { m_pause_ptr = &bptr; }

protected:
    PauseableBoard() {}

    virtual FallingPieceBase & piece_base() = 0;

    double fall_multiplier() const { return m_fall_multiplier; }

private:
    static constexpr const auto k_niether_dir = PlayControlId::count;
    double m_fall_multiplier = 1.;
    double m_move_time = 0.;
    PlayControlId m_move_dir = k_niether_dir;
    bool * m_pause_ptr = nullptr;
};

class PuyoScoreBoardBase {
public:
    static constexpr const int k_not_any_board = -1;
    virtual void increment_score(int board, int delta) = 0;
    virtual void reset_score(int board) = 0;
    virtual void set_next_pair(int board, int first, int second) = 0;
    virtual ~PuyoScoreBoardBase() {}

    static PuyoScoreBoardBase & null_instance() {
        class NullScoreBoard final : public PuyoScoreBoardBase {
            void increment_score(int, int) override {}
            void reset_score(int) override {}
            void set_next_pair(int, int, int) override {}
        };
        static NullScoreBoard inst;
        return inst;
    }
};

// doesn't know what a scenario is *at all*
// however methods exist so that it can be driven by one
// also doesn't know how to render/treat the score board
// but, it can talk to one
// no concept of location, but render states transformations are available
class PuyoBoard final : public PauseableBoard {
public:
    static constexpr const int k_init_pop_requirement = 4;

    void assign_score_board(int this_board_number, PuyoScoreBoardBase &);
    void set_settings(double fall_speed, int pop_requirement);
    void set_size(int width, int height);

    void update(double) override;
    void push_falling_piece(int first, int second);
    void push_fall_in_blocks(const BlockGrid &);

    bool is_ready() const override;
    bool is_gameover() const override;

    const FallingPiece & current_piece() const override { return m_piece; }

    ColorPair next_piece() const override { return m_next_piece; }

    const BlockGrid & blocks() const override { return m_blocks; }
    auto blocks() { return make_sub_grid(m_blocks); }

private:
    using UpdateFunc = void(PuyoBoard::*)(double);

    void draw(sf::RenderTarget &, sf::RenderStates) const override;

    FallingPieceBase & piece_base() override { return m_piece; }

    void update_piece(double);
    void update_pop_effects(double);
    void update_fall_effects(double);
    void update_on_gameover(double);

    PuyoScoreBoardBase * m_score_board = &PuyoScoreBoardBase::null_instance();
    int m_score_board_number = PuyoScoreBoardBase::k_not_any_board;

    FallingPiece m_piece;
    ColorPair m_next_piece = k_empty_pair;
    UpdateFunc m_update_func = &PuyoBoard::update_fall_effects;
    double m_fall_delay = 0.5;
    double m_fall_time  = 0.;

    BlockGrid m_blocks;
    FallEffectsFull m_fef;
    PuyoPopEffects m_pef;

    int m_pop_requirement = k_init_pop_requirement;
};

class PuyoScoreBoard final : public PuyoScoreBoardBase, public sf::Drawable {
public:
    static constexpr const int k_max_possible_score = 999999;
    void increment_score(int board, int delta) override;
    void reset_score(int board) override;
    void set_next_pair(int board, int first, int second) override;
    int width() const { return 3; }
    int take_last_delta(int board);

private:
    void draw(sf::RenderTarget &, sf::RenderStates) const override;

    std::pair<int, int> m_next_piece    = PuyoBoard::k_empty_pair;
    std::pair<int, int> m_next_p2_piece = PuyoBoard::k_empty_pair;

    int m_p1_delta = 0, m_p2_delta = 0;

    int m_first_player_score = 0;
};

// ----------------------------------------------------------------------------

class PuyoStateN final : public BoardState {
public:
    struct ContinueFall {};
    // needed by Scenario
    using Response = MultiType<std::pair<int, int>, BlockGrid, ContinueFall>;
    explicit PuyoStateN(int scenario_number);
    explicit PuyoStateN(ScenarioPtr sptr):
        m_current_scenario(std::move(sptr))
    {}

private:
    int width_in_blocks () const override
        { return m_board.width() + m_score_board.width(); }

    int height_in_blocks() const override { return m_board.height(); }

    int scale() const override { return 3; }

    void handle_event(PlayControlEvent pce) override {
        m_board.handle_event(pce);
    }

    void update(double) override;

    void setup_board(const Settings &) override;

    void draw(sf::RenderTarget &, sf::RenderStates) const override;

    void handle_response(const Response &);

    Rng m_rng = Rng { std::random_device()() };
    PuyoScoreBoard m_score_board;
    PuyoBoard m_board;
    ScenarioPtr m_current_scenario;
    // actually state wide
    bool m_pause = false;
};

class PuyoStateVS final : public BoardState {
public:
    // needed by Scenario
    using Response = MultiType<std::pair<int, int>, BlockGrid>;
    PuyoStateVS();

private:
    int width_in_blocks () const override;

    int height_in_blocks() const override;

    int scale() const override { return 3; }

    void handle_event(PlayControlEvent pce) override;

    void update(double) override;

    void setup_board(const Settings &) override;

    void draw(sf::RenderTarget &, sf::RenderStates) const override;

    void update_board(PuyoBoard &, Rng &, double et);

    Rng m_p1_rng, m_p2_rng, m_refuge_rng;

    PuyoScoreBoard m_score_board;
    PuyoBoard m_p1_board;
    PuyoBoard m_p2_board;

    std::unique_ptr<AiScript> m_ai_player;
    SimpleMatcher * m_matcher_ptr = nullptr;

    // actually state wide
    bool m_pause = false;

    Grid<bool> m_p2_accessables;
};

#if 0
// ----------------------------------------------------------------------------

class PuyoState final : public PauseableWithFallingPieceState {
public:
    // needed by Scenario
    using Response = MultiType<std::pair<int, int>, BlockGrid>;
#   if 0
    PuyoState() {}
#   endif
    explicit PuyoState(int scenario_number);
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
#endif
