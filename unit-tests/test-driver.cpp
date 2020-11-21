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

#include "../src/Defs.hpp"
#include "../src/BlockAlgorithm.hpp"
#include "../src/EffectsFull.hpp"
#include "../src/ColumnsClone.hpp"
#include "../src/PlayControl.hpp"

#include <common/TestSuite.hpp>

#include <iostream>

#include <cassert>

#ifndef MACRO_TEST_DRIVER_ENTRY_FUNCTION
#   define MACRO_TEST_DRIVER_ENTRY_FUNCTION main
#endif

namespace {

void print_grid(const Grid<int> &);

bool test_GetEdgeValue(ts::TestSuite &);
bool test_select_connected_blocks(ts::TestSuite &);
bool test_make_blocks_fall(ts::TestSuite &);
bool test_FallEffectsFull_do_fall_in(ts::TestSuite &);
bool test_columns_algo(ts::TestSuite &);
bool test_columns_rotate(ts::TestSuite &);
bool test_play_control(ts::TestSuite &);

} // end of <anonymous> namespace

int MACRO_TEST_DRIVER_ENTRY_FUNCTION() {
    ts::TestSuite suite;
    static const auto k_test_fns = {
        test_GetEdgeValue, test_select_connected_blocks, test_make_blocks_fall,
        test_FallEffectsFull_do_fall_in, test_columns_algo, test_columns_rotate,
        test_play_control
    };
    bool all_good = true;
    for (auto f : k_test_fns) {
        all_good = f(suite) && all_good;
    }
    return all_good ? 0 : ~0;
    // move to cul
#   if 0
    {
    auto g = make_grid({
       { 0, 1, 2 },
       { 3, 4, 5 },
       { 6, 7, 8 },
    });
    assert(g(0, 0) == 0);
    assert(g(1, 0) == 1);
    assert(g(2, 0) == 2);

    assert(g(0, 1) == 3);
    assert(g(1, 1) == 4);
    assert(g(2, 1) == 5);

    assert(g(0, 2) == 6);
    assert(g(1, 2) == 7);
    assert(g(2, 2) == 8);
    }
    {
    auto g = make_grid({
        { 0, 1 },
        { 2, 3 },
        { 4, 5 }
    });

    assert(g(0, 0) == 0);
    assert(g(1, 0) == 1);

    assert(g(0, 1) == 2);
    assert(g(1, 1) == 3);

    assert(g(0, 2) == 4);
    assert(g(1, 2) == 5);
    }
#   endif
}

namespace {

void print_grid(const Grid<int> & g) {
    for (int y = 0; y != g.height(); ++y) {
        std::cout << "[";
        for (int x = 0; x != g.width(); ++x) {
            std::cout << g(x, y);
            if (x + 1 != g.width()) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
    }
}

bool test_GetEdgeValue(ts::TestSuite & suite) {
    suite.start_series("GetEdgeValue");
    suite.test([]() { return ts::test(GetEdgeValue<>::k_value == TileEdges().to_ulong()); });
    suite.test([]() { return ts::test(GetEdgeValue<k_left_edge>::k_value == TileEdges().set(k_left_edge).to_ulong()); });
    suite.test([]() { return ts::test(GetEdgeValue<k_right_edge>::k_value == TileEdges().set(k_right_edge).to_ulong()); });
    suite.test([]() { return ts::test(GetEdgeValue<k_bottom_edge>::k_value == TileEdges().set(k_bottom_edge).to_ulong()); });
    suite.test([]() { return ts::test(GetEdgeValue<k_top_edge>::k_value == TileEdges().set(k_top_edge).to_ulong()); });
    suite.test([]() { return ts::test(GetEdgeValue<k_right_edge, k_left_edge>::k_value == TileEdges().set(k_right_edge).set(k_left_edge).to_ulong()); });

    suite.test([]() { return ts::test(GetEdgeValue<k_right_edge, k_bottom_edge>::k_value == TileEdges().set(k_right_edge).set(k_bottom_edge).to_ulong()); });
    suite.test([]() { return ts::test(GetEdgeValue<k_right_edge, k_top_edge>::k_value == TileEdges().set(k_right_edge).set(k_top_edge).to_ulong()); });
    suite.test([]() { return ts::test(GetEdgeValue<k_left_edge, k_top_edge>::k_value == TileEdges().set(k_left_edge).set(k_top_edge).to_ulong()); });
    suite.test([]() { return ts::test(GetEdgeValue<k_left_edge, k_bottom_edge>::k_value == TileEdges().set(k_left_edge).set(k_bottom_edge).to_ulong()); });
    suite.test([]() { return ts::test(GetEdgeValue<k_top_edge, k_bottom_edge>::k_value == TileEdges().set(k_top_edge).set(k_bottom_edge).to_ulong()); });


    suite.test([]() { return ts::test(GetEdgeValue<k_top_edge, k_bottom_edge, k_left_edge>::k_value == TileEdges().set(k_top_edge).set(k_bottom_edge).set(k_left_edge).to_ulong()); });
    suite.test([]() { return ts::test(GetEdgeValue<k_top_edge, k_bottom_edge, k_right_edge>::k_value == TileEdges().set(k_top_edge).set(k_bottom_edge).set(k_right_edge).to_ulong()); });
    suite.test([]() { return ts::test(GetEdgeValue<k_bottom_edge, k_right_edge, k_left_edge>::k_value == TileEdges().set(k_bottom_edge).set(k_right_edge).set(k_left_edge).to_ulong()); });
    suite.test([]() { return ts::test(GetEdgeValue<k_top_edge, k_right_edge, k_left_edge>::k_value == TileEdges().set(k_top_edge).set(k_right_edge).set(k_left_edge).to_ulong()); });

    suite.test([]() { return ts::test(GetEdgeValue<k_left_edge>::k_value == TileEdges().set(k_left_edge).to_ulong()); });
    return suite.has_successes_only();
}

bool test_select_connected_blocks(ts::TestSuite & suite) {
    suite.start_series("select_connected_blocks");
    suite.test([]() { // test 0
        auto g = make_grid({
           { 0, 1, 0 },
           { 1, 1, 1 },
           { 0, 1, 0 }
        });

        Grid<bool> explored;
        explored.set_size(g.width(), g.height());
        std::vector<VectorI> selections = { VectorI(1, 1) };
        select_connected_blocks(g, selections, explored);
        return ts::test(selections.size() == 5);
    });

    suite.test([]() { // test 1
        auto g = make_grid({
           { 1, 0, 1 },
           { 1, 1, 1 },
           { 1, 0, 1 }
        });
        Grid<bool> explored;
        explored.set_size(g.width(), g.height());
        std::vector<VectorI> selections = { VectorI(0, 0) };
        select_connected_blocks(g, selections, explored);
        return ts::test(selections.size() == 7);
    });
    suite.test([]() { // test 2
        auto g = make_grid({
           { 0, 0, 2, 0 },
           { 0, 1, 1, 0 },
           { 0, 1, 1, 2 },
           { 0, 0, 0, 0 }
        });
        Grid<bool> explored;
        explored.set_size(g.width(), g.height());
        std::vector<VectorI> selections = { VectorI(1, 1) };
        select_connected_blocks(g, selections, explored);
        return ts::test(selections.size() == 4);
    });
    return suite.has_successes_only();
}

bool test_make_blocks_fall(ts::TestSuite & suite) {
    suite.start_series("make_blocks_fall");
    suite.test([]() { // test 0
        auto g = make_grid({
            { 0 },
            { 1 },
            { 0 },
            { 1 },
            { 1 },
        });
        make_blocks_fall(g);
        return ts::test(
               g(0, 0) == 0
            && g(0, 1) == 0
            && g(0, 2) == 1
            && g(0, 3) == 1
            && g(0, 4) == 1);
    });
    assert(is_grid_the_same(make_grid({
        { 1, 0, 0 },
        { 1, 0, 0 },
        { 1, 1, 1 },
    }), {
        { 1, 0, 0 },
        { 1, 0, 0 },
        { 1, 1, 1 },
    }));
    return suite.has_successes_only();
}

bool test_FallEffectsFull_do_fall_in(ts::TestSuite & suite) {
    suite.start_series("FallEffectsFull::do_fall_in");
    static const sf::Texture test_texture;
    suite.test([]() {
        auto g = make_grid({
            { 0, 0, 0 },
            { 1, 0, 0 },
            { 1, 1, 1 },
        });
        auto fallins = make_grid({
            { 0, 0, 1 },
            { 0, 1, 0 },
            { 1, 0, 0 },
        });
        FallEffectsFull fef;
        fef.setup(g.width(), g.height(), test_texture);
        fef.do_fall_in(g, fallins);
        return ts::test(is_grid_the_same(g, {
            { 1, 0, 0, },
            { 1, 1, 1, },
            { 1, 1, 1, },
        }));
    });
    suite.test([]() {
        auto g = make_grid({
            { 0, 0, 0 },
            { 1, 0, 0 },
            { 1, 0, 1 },
        });
        auto fallins = make_grid({
            { 0, 0, 1 },
            { 0, 1, 0 },
            { 0, 0, 0 },
        });
        FallEffectsFull fef;
        fef.setup(g.width(), g.height(), test_texture);
        fef.do_fall_in(g, fallins);
        return ts::test(is_grid_the_same(g, {
            { 0, 0, 0, },
            { 1, 0, 1, },
            { 1, 1, 1, },
        }));
    });
    suite.test([]() {
        auto g = make_grid({
            { 0, 0, 0 },
            { 0, 0, 0 },
            { 0, 0, 0 },
            { 1, 0, 0 },
        });
        auto fallins = make_grid({
            { 0, 0, 1 },
            { 1, 1, 0 },
            { 1, 1, 1 },
            { 1, 1, 0 },
        });
        FallEffectsFull fef;
        fef.setup(g.width(), g.height(), test_texture);
        fef.do_fall_in(g, fallins);
        return ts::test(is_grid_the_same(g, {
            { 1, 0, 0 },
            { 1, 1, 0 },
            { 1, 1, 1 },
            { 1, 1, 1 },
        }));
    });
    return suite.has_successes_only();
}

bool test_columns_algo(ts::TestSuite & suite) {
    suite.start_series("pop_columns_blocks");
    suite.test([]() {
        auto g = make_grid({
            { 1 }, { 1 }, { 1 }
        });
        pop_columns_blocks(g, 3);
        return ts::test(g(0, 0) == 0 && g(0, 1) == 0 && g(0, 2) == 0);
    });
    suite.test([]() {
        auto g = make_grid({
            { 1, 1, 1 }
        });
        pop_columns_blocks(g, 3);
        return ts::test(g(0, 0) == 0 && g(1, 0) == 0 && g(2, 0) == 0);
    });
    suite.test([]() {
        auto g = make_grid({
            { 1, 1, 2 },
            { 1, 3, 2 },
            { 2, 3, 3 }
        });
        return ts::test(!pop_columns_blocks(g, 3));
    });
    static auto compare_test_to_cor = [](const Grid<int> & og, const Grid<int> & cor) {
        auto g = og;
        pop_columns_blocks(g, 3);
        bool res = std::equal(g.begin(), g.end(), cor.begin(), cor.end());
        if (!res) {
            std::cout << "Original grid:" << std::endl;
            print_grid(og);
            std::cout << "Result grid:" << std::endl;
            print_grid(g);
            std::cout << "Correct grid: " << std::endl;
            print_grid(cor);
        }
        return res;
    };
    suite.test([]() {
        return ts::test(compare_test_to_cor(
            make_grid({
                { 0, 0, 0, 2 },
                { 3, 1, 1, 1 },
            }), make_grid({
                { 0, 0, 0, 2 },
                { 3, 0, 0, 0 },
            })
        ));
    });
    suite.test([]() {
        return ts::test(compare_test_to_cor(
            make_grid({
                { 1, 1, 1, 2 }
            }), make_grid({
                { 0, 0, 0, 2 }
            })
        ));
    });
    suite.test([]() {
        return ts::test(compare_test_to_cor(
            make_grid({
                { 1, 2, 2 },
                { 2, 1, 2 },
                { 2, 2, 1 }
            }), make_grid({
                { 0, 2, 2 },
                { 2, 0, 2 },
                { 2, 2, 0 }
            })
        ));
    });
    suite.test([]() {
        return ts::test(compare_test_to_cor(
            make_grid({
                { 1, 1, 1 },
                { 2, 1, 2 },
                { 2, 2, 1 }
            }), make_grid({
                { 0, 0, 0 },
                { 2, 0, 2 },
                { 2, 2, 0 }
            })
        ));
    });
    suite.test([]() {
        auto g = make_grid({
            { 1, 1, 1, 2, 2, 3, 1 },
            { 2, 1, 3, 2, 3, 1, 2 },
            { 4, 1, 4, 4, 1, 4, 3 },
            { 1, 1, 4, 3, 2, 1, 3 },
            { 4, 3, 4, 3, 2, 3, 1 }
        });
        auto cor = make_grid({
            { 0, 0, 0, 2, 2, 3, 0 },
            { 2, 0, 3, 2, 3, 0, 2 },
            { 4, 0, 0, 4, 0, 4, 3 },
            { 1, 0, 0, 3, 2, 0, 3 },
            { 4, 3, 0, 3, 2, 3, 0 }
        });
        return ts::test(compare_test_to_cor(g, cor));
    });
    return suite.has_successes_only();
}

bool test_columns_rotate(ts::TestSuite & suite) {
    suite.start_series("ColumnsPiece::rotate");
    suite.test([]() {
        ColumnsPiece p(1, 2, 3);
        // bottom comes first assumed
        assert(p.as_blocks()[0].second == 1 && p.as_blocks()[2].second == 3);
        p.rotate_down();
        auto blocks = p.as_blocks();
        return ts::test(   blocks[2].second == 1
                        && blocks[1].second == 3
                        && blocks[0].second == 2);
    });
    suite.test([]() {
        ColumnsPiece p(1, 2, 3);
        p.rotate_up();
        auto blocks = p.as_blocks();
        return ts::test(   blocks[2].second == 2
                        && blocks[1].second == 1
                        && blocks[0].second == 3);
    });

    return suite.has_successes_only();
}

class PceTester final : public PlayControlEventReceiver {
public:
    PceTester() {}
    explicit PceTester(std::vector<PlayControlEvent> && vec)
        { set_expected(std::move(vec)); }

    void handle_event(PlayControlEvent pce) override {
        if (m_expected_events.empty()) return;
        if (are_same(pce, m_expected_events.back())) {
            m_expected_events.pop_back();
            return;
        }
        throw std::runtime_error("Unexpected play control event.");
    }

    void set_expected(std::vector<PlayControlEvent> && vec) {
        m_expected_events = vec;
        std::reverse(m_expected_events.begin(), m_expected_events.end());
    }
private:
    std::vector<PlayControlEvent> m_expected_events;
};

sf::Event make_key_press(sf::Keyboard::Key k) {
    sf::Event e;
    e.type = sf::Event::KeyPressed;
    e.key.code = k;
    e.key.alt = e.key.control = e.key.shift = e.key.system = false;
    return e;
}
sf::Event make_key_release(sf::Keyboard::Key k) {
    sf::Event e;
    e.type = sf::Event::KeyReleased;
    e.key.code = k;
    e.key.alt = e.key.control = e.key.shift = e.key.system = false;
    return e;
}

bool test_play_control(ts::TestSuite & suite) {
    suite.start_series("PlayControl");
    suite.test([]() {
        PceTester tester({
            PlayControlEvent(PlayControlId::up, PlayControlState::just_pressed)
        });
        PlayControlEventHandler pceh;
        pceh.update(make_key_press(sf::Keyboard::Up));
        pceh.send_events(tester);
        return ts::test(true);
    });
    return suite.has_successes_only();
}

} // end of <anonymous> namespace
