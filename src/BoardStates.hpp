#pragma once

#include "AppState.hpp"
#include "Defs.hpp"
#include "Polyomino.hpp"
#include "EffectsFull.hpp"
#include "FallingPiece.hpp"
#include "Settings.hpp"

#include <random>

class BoardState : public AppState {
public:
    using BoardOptions = Settings::Board;

protected:
    using Rng       = std::default_random_engine;
    using IntDistri = std::uniform_int_distribution<int>;

    double width () const final { return double(width_in_blocks ()*k_block_size); }

    double height() const final { return double(height_in_blocks()*k_block_size); }

    void process_event(const sf::Event & event) override;

    virtual void setup_board(const Settings &) = 0;

    virtual int width_in_blocks () const = 0;

    virtual int height_in_blocks() const = 0;

    /** Sets board settings */
    void setup_(Settings &) final;

    int random_color(Rng & rng) const
        { return IntDistri(1, m_max_colors)(rng); }

    void set_max_colors(int);
private:
    int m_max_colors = k_min_colors;
};

// ----------------------------------------------------------------------------

class TetrisState final : public BoardState {
    static constexpr const double k_default_fall_delay = 1.;
    void setup_board(const Settings &) override;
    void update(double et) override;
    void process_event(const sf::Event & event) override;

    void draw(sf::RenderTarget &, sf::RenderStates) const override;

    int width_in_blocks () const override { return m_blocks.width(); }

    int height_in_blocks() const override { return m_blocks.height(); }

    int scale() const override { return 2; }

    void reset_piece();

    Grid<int> m_blocks;
    Polyomino m_piece;
    double m_fall_time = 0.;
    double m_fall_multiplier = 1.;
    double m_fall_delay = k_default_fall_delay;
    FallEffectsFull m_fef;

    Rng m_rng { std::random_device()() };
};

// ----------------------------------------------------------------------------

class SameGame final : public BoardState {
    void setup_board(const Settings &) override;
    void update(double et) override;
    void process_event(const sf::Event &) override;
    void draw(sf::RenderTarget &, sf::RenderStates) const override;

    int width_in_blocks () const override { return m_blocks.width(); }

    int height_in_blocks() const override { return m_blocks.height(); }

    int scale() const override { return 3; }

    void do_selection();

    void try_sweep();

    VectorI m_selection;
    Grid<int> m_blocks;
    Grid<int> m_sweep_temp;
    SameGamePopEffects m_pef;
    FallEffectsFull m_fef;

    Rng m_rng { std::random_device()() };
};