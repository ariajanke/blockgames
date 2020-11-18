#include "../src/Defs.hpp"
#include "../src/BlockAlgorithm.hpp"
#include "../src/EffectsFull.hpp"

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

} // end of <anonymous> namespace

int MACRO_TEST_DRIVER_ENTRY_FUNCTION() {
    ts::TestSuite suite;
    auto test_fns = {
        test_GetEdgeValue, test_select_connected_blocks, test_make_blocks_fall,
        test_FallEffectsFull_do_fall_in, test_columns_algo
    };
    bool all_good = true;
    for (auto f : test_fns) {
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
            { 4, 1, 4, 4, 1, 3, 3 },
            { 1, 1, 4, 3, 2, 1, 3 },
            { 4, 3, 4, 3, 2, 3, 1 }
        });
        auto cor = make_grid({
            { 0, 0, 0, 2, 2, 3, 0 },
            { 2, 0, 3, 2, 3, 0, 2 },
            { 4, 0, 0, 4, 0, 3, 3 },
            { 1, 0, 0, 3, 2, 0, 3 },
            { 4, 3, 0, 3, 2, 3, 0 }
        });
        return ts::test(compare_test_to_cor(g, cor));
    });
    return suite.has_successes_only();
}

} // end of <anonymous> namespace
