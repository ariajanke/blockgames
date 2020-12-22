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

#include "AppState.hpp"
#include "Defs.hpp"
#include "Polyomino.hpp"
#include "EffectsFull.hpp"
#include "FallingPiece.hpp"
#include "Settings.hpp"
#include "PlayControl.hpp"

#include <random>

class BoardState : public AppState, public PlayControlEventReceiver {
public:
    using BoardOptions = Settings::Board;

    static void draw_fill_with_background
        (sf::RenderTarget &, int board_width, int board_height,
         VectorI offset = VectorI(), sf::Color mask = sf::Color::White);

    static void draw_fill_with_score_background
        (sf::RenderTarget &, int board_width, int board_height,
         VectorI offset = VectorI());

protected:
    using Rng       = std::default_random_engine;
    using IntDistri = std::uniform_int_distribution<int>;

    double width() const final { return double(width_in_blocks ()*k_block_size); }

    double height() const final { return double(height_in_blocks()*k_block_size); }

    void process_event(const sf::Event & event) override;

    void update(double et) override;

    virtual void setup_board(const Settings &) = 0;

    virtual int width_in_blocks () const = 0;

    virtual int height_in_blocks() const = 0;

    /** Sets board settings */
    void setup_(Settings &) final;

#   if 0
    int random_color(Rng & rng) const
        { return IntDistri(1, m_max_colors)(rng); }
#   endif
    BlockId random_color(Rng & rng) const
        { return ColorBlockDistri(m_max_colors)(rng); }

    void set_max_colors(int);

private:
    int m_max_colors = k_min_colors;
    PlayControlEventHandler m_pc_handler;
};

// ----------------------------------------------------------------------------

class PauseableWithFallingPieceState : public BoardState {
public:
    static constexpr const double k_fast_fall  = 5.;
    static constexpr const double k_slow_fall  = 1.;
    static constexpr const double k_move_delay = 1. / 8.;

    void update(double et) override;

    void handle_event(PlayControlEvent) override;

protected:
    virtual FallingPieceBase & piece_base() = 0;
    virtual const BlockGrid & blocks() const = 0;

    bool is_paused() const { return m_is_paused; }
    double fall_multiplier() const { return m_fall_multiplier; }

private:
    static constexpr const auto k_niether_dir = PlayControlId::count;
    double m_fall_multiplier = 1.;
    double m_move_time = 0.;
    PlayControlId m_move_dir = k_niether_dir;
    bool m_is_paused = false;
};

// ----------------------------------------------------------------------------

class TetrisState final : public PauseableWithFallingPieceState {
    static constexpr const double k_default_fall_delay = 1.;
    void setup_board(const Settings &) override;
    void update(double et) override;

    void draw(sf::RenderTarget &, sf::RenderStates) const override;

    int width_in_blocks () const override { return m_blocks.width(); }

    int height_in_blocks() const override { return m_blocks.height(); }

    int scale() const override { return 2; }

    FallingPieceBase & piece_base() override { return m_piece; }

    const BlockGrid & blocks() const override { return m_blocks; }

    void reset_piece();

    BlockGrid m_blocks;
    Polyomino m_piece;
    double m_fall_time = 0.;

    double m_fall_delay = k_default_fall_delay;
    FallEffectsFull m_fef;
    std::vector<Polyomino> m_available_polyominos;

    Rng m_rng { std::random_device()() };
};

// ----------------------------------------------------------------------------

class SameGame final : public BoardState {
    void setup_board(const Settings &) override;
    void update(double et) override;
    void process_event(const sf::Event &) override;
    void handle_event(PlayControlEvent) override;

    void draw(sf::RenderTarget &, sf::RenderStates) const override;

    int width_in_blocks () const override { return m_blocks.width(); }

    int height_in_blocks() const override { return m_blocks.height(); }

    int scale() const override { return 3; }

    void do_selection();

    void try_sweep();

    VectorI m_selection;
    BlockGrid m_blocks;
    BlockGrid m_sweep_temp;
    SameGamePopEffects m_pop_ef;
    FallEffectsFull m_fall_ef;
    bool m_pop_singles_enabled = false;

    Rng m_rng { std::random_device()() };
};
