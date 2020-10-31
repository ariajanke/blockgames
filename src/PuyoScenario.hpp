#pragma once

#include "PuyoState.hpp"

class Scenario {
public:
    virtual ~Scenario();
    static const std::pair<int, int> k_random_pair;
    static constexpr const int k_use_freeplay_settings = -1;
    using Response = PuyoState::Response;

    struct Params {
        int board_width  = k_use_freeplay_settings;
        int board_height = k_use_freeplay_settings;
        int max_colors   = k_use_freeplay_settings;
    };

    virtual void setup(Params &) = 0;
    virtual Response on_turn_change() = 0;

    double fall_speed() const;
    void assign_board(Grid<int> &);

    template<typename T>
    static ScenarioPtr make_scenario() {
        ScenarioPtr rv;
        rv.reset(new T());
        return rv;
    }

    static ScenarioPtr make_pop_forever();
    static ScenarioPtr make_glass_waves();

protected:
    Grid<int> & board();

    void set_fall_speed(double);

private:
    double m_fall_speed = 1.;
    Grid<int> * m_board = nullptr;
};