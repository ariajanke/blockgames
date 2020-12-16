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

#include "PuyoScenario.hpp"

#include <SFML/Window/VideoMode.hpp>

namespace {

using Rng          = std::default_random_engine;
using IntDistri    = std::uniform_int_distribution<int>;
using Response     = Scenario::Response;
using PuyoSettings = Scenario::PuyoSettings;

template <typename T, typename ... Args>
ScenarioPtr make_scenario(Args ... args) {
    static_assert(std::is_base_of_v<Scenario, T>, "");
    return ScenarioPtr(new T(std::forward(args)...));
}

template <typename T, typename U>
std::vector<T> move_and_convert(std::vector<U> && cont) {
    using std::move;
    std::vector<T> rv;
    rv.reserve(cont.size());
    for (auto & scen : cont) rv.emplace_back(move(scen));
    return rv;
}

class SequentialScenario : public Scenario {
    bool is_sequential() const final { return true; }
};

class NonSequentialScenario : public Scenario {
    bool is_sequential() const final { return false; }
};

class ForeverPop final : public NonSequentialScenario {
    PuyoSettings setup(PuyoSettings params) override {
        // need to disable score board somehow
        auto screen_width  = sf::VideoMode::getDesktopMode().width;
        auto screen_height = sf::VideoMode::getDesktopMode().height;
        params.width  = (screen_width  / (k_block_size*3)) - 3;
        params.height = (screen_height / (k_block_size*3));
        m_rng = Rng { std::random_device()() };
        //m_max_colors = params.colors;
        return params;
    }

    Response on_turn_change() override;

    const char * name() const override { return "Pop Forever"; }
    const char * description() const override {
        return "A \"non-playable\" scenario. Where the board will pop blocks "
               "forever for your viewing pleasure.";
    }
    ScenarioPtr clone() const override { return make_scenario<ForeverPop>(); }

    int m_max_colors = 3;
    Rng m_rng;
};

class PracticeMode final : public NonSequentialScenario {
    PuyoSettings setup(PuyoSettings params) override
        { return params; }

    Response on_turn_change() override
        { return Response(k_random_pair); }

    const char * name() const override { return "Regular Practice"; }

    const char * description() const override {
        return "The regualar free play practice mode. No glass blocks, no bells "
               "and wistles, just the simple game.";
    }

    ScenarioPtr clone() const override { return make_scenario<PracticeMode>(); }
};

class GlassWaves final : public NonSequentialScenario {
public:
    PuyoSettings setup(PuyoSettings params) override;
    Response on_turn_change() override;

    const char * name() const override { return "Glass Waves"; }
    const char * description() const override {
        return "An unending scenario. Try not to get buried in glass blocks!";
    }
    ScenarioPtr clone() const override { return make_scenario<GlassWaves>(); }

private:
    int m_turn_num = 0;
    Rng m_rng;
};

std::vector<ScenarioPtr> verify_guarantees(std::vector<ScenarioPtr> &&);

} // end of <anonymous> namespace

/* static */ const std::pair<int, int> Scenario::k_random_pair = std::make_pair(-1, -1);

/* virtual */ Scenario::~Scenario() {}

double Scenario::fall_speed() const
    { return m_fall_speed; }

void Scenario::assign_board(SubGrid<int> subgrid)
    { m_board_ref = subgrid; }
#if 0
void Scenario::assign_board(Grid<int> & grid)
    { m_board = &grid; }

/* protected */ Grid<int> & Scenario::board()
    { return *m_board; }
#endif
/* protected */ SubGrid<int> & Scenario::board() {
    if (m_board_ref.width() == 0 || m_board_ref.height() == 0)
        throw std::runtime_error("unassigned");
    return m_board_ref;
}

/* protected */ void Scenario::set_fall_speed(double x)
    { m_fall_speed = x; }

/* static */ ScenarioPtr Scenario::make_pop_forever()
    { return make_scenario<ForeverPop>(); }

/* static */ ScenarioPtr Scenario::make_glass_waves()
    { return make_scenario<GlassWaves>(); }

/* static */ std::vector<ScenarioPtr> Scenario::make_all_scenarios() {
    std::vector<ScenarioPtr> rv;
    rv.emplace_back(make_pop_forever());
    rv.emplace_back(make_scenario<PracticeMode>());
    rv.emplace_back(make_glass_waves());
    return verify_guarantees(std::move(rv));
}

/* static */ const std::vector<ConstScenarioPtr> & Scenario::get_all_scenarios() {
    static std::vector<ConstScenarioPtr> s_scenarios =
        move_and_convert<ConstScenarioPtr>(make_all_scenarios());
    return s_scenarios;
}

/* static */ const int Scenario::k_freeplay_scenario_count = []() {
    int count = 0;
    for (auto & uptr : Scenario::make_all_scenarios()) {
        if (uptr->is_sequential()) break;
        ++count;
    }
    return count;
}();

namespace {

/* private */ Response ForeverPop::on_turn_change() {
    // I need to implement bidirectionals for subgrid
    int count = int(board().size()) - std::count(board().begin(), board().end(), k_empty_block);
#   if 0
    [this]() {
        int rv = 0;
        for (VectorI r; r != board().end_position(); r = board().next(r)) {
            if (board()(r) != k_empty_block) ++rv;
        }
        return rv;
    } ();
    std::count_if(board().begin(), board().end(), [](int x) { return x != k_empty_block; });
#   endif
    int area  = board().width()*board().height();
    if (area == count) {
        int y = IntDistri(0, board().height() - 1)(m_rng);
        int t = board()(0, y);
        for (int x = 0; x != board().width() - 1; ++x) {
            board()(x, y) = board()(x + 1, y);
        }
        board()(board().width() - 1, y) = t;
    } else if (count > (area * 4) / 10) {
        for (int x = 0; x != board().width(); ++x) {
            int & top = board()(x, 0);
            if (top != k_empty_block) continue;
            if (IntDistri(0, 4)(m_rng) == 0) top = IntDistri(1, k_max_colors)(m_rng);
            std::swap(board()(x, board().height() - 1), top);
        }
    } else {
        Grid<int> fallins;
        fallins.set_size(board().width(), board().height());
        for (int & x : fallins) {
            if (IntDistri(1, 10)(m_rng) == 10) {
                x = k_hard_glass_block;
            } else {
                x = IntDistri(1, m_max_colors)(m_rng);
            }
        }
        return Response(fallins);
    }
    return Response();
}

// ------------------------------ class divider -------------------------------

/* private */ PuyoSettings GlassWaves::setup(PuyoSettings params) {
#   if 0
    params.max_colors = 3;
#   endif
    m_rng = Rng { std::random_device()() };
    return params;
}

/* private */ GlassWaves::Response GlassWaves::on_turn_change() {
    if (m_turn_num++ % 2 == 0) {
        return Response { k_random_pair };
    }
    Response rv;
    auto & fallins = rv.reset<Grid<int>>();
    fallins.set_size(board().width(), board().height());
    for (int x = 0; x != board().width(); ++x) {
        if (IntDistri(0, 3)(m_rng)) continue;
        if (IntDistri(0, 3)(m_rng)) {
            fallins(x, 0) = k_glass_block;
        } else {
            fallins(x, 0) = k_hard_glass_block;
        }
    }
    return rv;
}

std::vector<ScenarioPtr> verify_guarantees(std::vector<ScenarioPtr> && rv) {
    bool reached_end_of_ns = false;
    for (auto & uptr : rv) {
        if (uptr->is_sequential()) {
            reached_end_of_ns = true;
        } else {
            if (reached_end_of_ns) {
                throw std::invalid_argument("verify_guarantees: non-sequentials must all come before sequentials");
            }
        }
    }
    return std::move(rv);
}

} // end of <anonymous> namespace
