#include "PuyoScenario.hpp"

namespace {

using Rng = std::default_random_engine;
using IntDistri = std::uniform_int_distribution<int>;
using Response = Scenario::Response;

class ForeverPop final : public Scenario {
    void setup(Params &) override
        { m_rng = Rng { std::random_device()() }; }

    Response on_turn_change() override;
    Rng m_rng;
};

class GlassWaves final : public Scenario {
public:
    void setup(Params &) override;
    Response on_turn_change() override;
private:
    int m_turn_num = 0;
    Rng m_rng;
};

} // end of <anonymous> namespace

/* static */ const std::pair<int, int> Scenario::k_random_pair = std::make_pair(-1, -1);

/* virtual */ Scenario::~Scenario() {}

double Scenario::fall_speed() const
    { return m_fall_speed; }

void Scenario::assign_board(Grid<int> & grid)
    { m_board = &grid; }

/* protected */ Grid<int> & Scenario::board()
    { return *m_board; }

/* protected */ void Scenario::set_fall_speed(double x)
    { m_fall_speed = x; }

/* static */ ScenarioPtr Scenario::make_pop_forever()
    { return make_scenario<ForeverPop>(); }

/* static */ ScenarioPtr Scenario::make_glass_waves()
    { return make_scenario<GlassWaves>(); }

namespace {

/* private */ Response ForeverPop::on_turn_change() {
    int count = std::count_if(board().begin(), board().end(), [](int x) { return x != k_empty_block; });
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
                x = IntDistri(1, k_max_colors)(m_rng);
            }
        }
        return Response(fallins);
    }
    return Response();
}

// ------------------------------ class divider -------------------------------

/* private */ void GlassWaves::setup(Params & params) {
    params.max_colors = 3;
    m_rng = Rng { std::random_device()() };
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

} // end of <anonymous> namespace
