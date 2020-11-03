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

class PuyoState final : public BoardState {
public:
    // needed by Scenario
    using Response = MultiType<std::pair<int, int>, Grid<int>>;
    PuyoState() {}
    explicit PuyoState(ScenarioPtr sptr):
        m_current_scenario(std::move(sptr))
    {}
private:
    using UpdateFunc = void(PuyoState::*)(double);

    void setup_board(const Settings &) override;
    void update(double et) override;
    void process_event(const sf::Event &) override;

    void draw(sf::RenderTarget & target, sf::RenderStates states) const override;

    int width_in_blocks () const override { return m_blocks.width () + 4; }

    int height_in_blocks() const override { return m_blocks.height(); }

    int scale() const override { return 3; }

    void update_piece(double);
    void update_pop_effects(double);
    void update_fall_effects(double);

    void handle_response(const Response &);

    Grid<int> m_blocks;
    Rng m_rng = Rng { std::random_device()() };
    FallEffectsFull m_fef;
    PuyoPopEffects m_pef;

    double m_fall_time = 0.;
    double m_fall_delay = 0.5;
    FallingPiece m_piece;
    std::pair<int, int> m_next_piece = std::make_pair(k_empty_block, k_empty_block);
    UpdateFunc m_update_func = &PuyoState::update_fall_effects;

    double m_fall_multi = 1.;
    int m_pop_requirement = 4;
    bool m_is_paused = false;

    ScenarioPtr m_current_scenario;
};
