#include "BlockAlgorithm.hpp"

#include <array>

#include <cassert>

namespace {

std::array<VectorI, 4> get_neighbor_positions_for(VectorI);

void pop_special_neighbors(Grid<int> &, VectorI location, PopEffects &);

[[nodiscard]] inline auto make_finisher(PopEffects & pop_effects) {
    struct FinisherRaii {
        FinisherRaii(PopEffects & effects_): effects(effects_) {}
        ~FinisherRaii() { effects.finish(); }
        PopEffects & effects;
    };
    return FinisherRaii(pop_effects);
}

[[nodiscard]] inline auto make_finisher(FallBlockEffects & pop_effects) {
    struct FinisherRaii {
        FinisherRaii(FallBlockEffects & effects_): effects(effects_) {}
        ~FinisherRaii() { effects.finish(); }
        FallBlockEffects & effects;
    };
    return FinisherRaii(pop_effects);
}

} // end of <anonymous> namespace

/* static */ FallBlockEffects & FallBlockEffects::default_instance() {
    class DefInst final : public FallBlockEffects {
        void start() override {}
        void post_stationary_block(VectorI, int) override {}
        void post_block_fall(VectorI, VectorI, int) override {}
        void finish() override {}
    };
    static DefInst inst;
    return inst;
}

FallBlockEffects::~FallBlockEffects() {}

void make_blocks_fall(SubGrid<int> grid, FallBlockEffects & effects) {
    effects.start();
    auto finisher = make_finisher(effects);

    for (int x = 0                ; x != grid.width(); ++x) {
    for (int y = grid.height() - 1; y != -1          ;    ) {
        if (grid(x, y) != k_empty_block) {
            effects.post_stationary_block(VectorI(x, y), grid(x, y));
            --y;
            continue;
        }
        bool swap_occured = false;
        for (int yi = y; yi != -1 && !swap_occured; --yi) {
            if (grid(x, yi) == k_empty_block) continue;
            std::swap(grid(x, y), grid(x, yi));
            effects.post_block_fall(VectorI(x, yi), VectorI(x, y), grid(x, y));
            // only one step
            --y;
            swap_occured = true;
        }
        if (!swap_occured) {
            // skip rest of column
            break;
        }
    }}
}

void make_tetris_rows_fall(SubGrid<int> blocks, FallBlockEffects & effects) {
    effects.start();
    auto finisher = make_finisher(effects);

    int cleared_rows = 0;
    for (int y = blocks.height() - 1; y != -1; --y) {
        bool row_is_clear = [&blocks, y]() {
            for (int x = 0; x != blocks.width(); ++x) {
                if (blocks(x, y) != k_empty_block) return false;
            }
            return true;
        }();
        if (row_is_clear) {
            ++cleared_rows;
        } else if (cleared_rows > 0) {
            for (int x = 0; x != blocks.width(); ++x) {
                if (!is_block_color(blocks(x, y))) continue;
                effects.post_block_fall(VectorI(x, y), VectorI(x, y + cleared_rows), blocks(x, y));
                std::swap(blocks(x, y), blocks(x, y + cleared_rows));
            }
        } else {
            for (int x = 0; x != blocks.width(); ++x) {
                if (!is_block_color(blocks(x, y))) continue;
                effects.post_stationary_block(VectorI(x, y), blocks(x, y));
            }
        }
    }
}

void make_all_blocks_fall_out(SubGrid<int> blocks, FallBlockEffects & effects) {
    effects.start();
    auto finisher = make_finisher(effects);
    for (VectorI r; r != blocks.end_position(); r = blocks.next(r)) {
        if (blocks(r) == k_empty_block) continue;
        effects.post_block_fall(r, VectorI(r.x, blocks.height()), blocks(r));
        blocks(r) = k_empty_block;
    }
}

// ----------------------------------------------------------------------------

/* static */ PopEffects & PopEffects::default_instance() {
    struct NullPopEffects : public PopEffects {
        void start() override {}
        void finish() override {}
        void post_pop_effect(VectorI, int) override {}
    };
    static NullPopEffects inst;
    return inst;
}

PopEffects::~PopEffects() {}

std::vector<VectorI> select_connected_blocks(const Grid<int> & grid, VectorI r) {
    std::vector<VectorI> rv;
    Grid<bool> explored;
    rv.push_back(r);
    explored.set_size(grid.width(), grid.height());
    select_connected_blocks(grid, rv, explored);
    return rv;
}

void select_connected_blocks
    (const ConstSubGrid<int> & grid,
     std::vector<VectorI> & selected, Grid<bool> & explored)
{
    assert(selected.size() == 1);
    if (explored(selected[0])) return;
    explored(selected[0]) = true;
    int color = grid(selected[0]);
    std::size_t last     = 0;
    std::size_t old_size = selected.size();
    while (true) {
        for (auto i = last; i != old_size; ++i) {
            auto v = selected[i];
            for (auto n : get_neighbor_positions_for(v)) {
                if (!grid.has_position(n)) continue;

                if (grid(n) != color || !is_block_color(grid(n))) continue;
                if (explored(n)) continue;
                explored(n) = true;
                selected.push_back(n);
            }
        }
        if (old_size == selected.size()) break;
        last     = old_size;
        old_size = selected.size();
    }
}

bool pop_connected_blocks
    (Grid<int> & grid, int amount_required, PopEffects & effects)
{
    effects.start();
    auto finisher = make_finisher(effects);

    bool any_popped = false;
    Grid<bool> explored;
    std::vector<VectorI> selections;
    explored.set_size(grid.width(), grid.height());

    for (VectorI r; r != grid.end_position(); r = grid.next(r)) {
        if (grid(r) == 0) continue;
        selections.clear();
        selections.push_back(r);
        select_connected_blocks(grid, selections, explored);
        if (int(selections.size()) >= amount_required) {
            any_popped = true;
            for (auto u : selections) {
                pop_special_neighbors(grid, u, effects);
                effects.post_pop_effect(u, grid(u));
                grid(u) = 0;
            }
        }
    }
    return any_popped;
}

bool pop_columns_blocks(Grid<int> & blocks, int pop_requirement, PopEffects & effects) {
    struct IterGroup {
        VectorI beg, step, sweep_step;
    };

    effects.start();
    auto finisher = make_finisher(effects);

    static const VectorI k_no_match(-1, -1);

    bool any_popped = false;
    static const auto k_scan_list = {
        IterGroup { VectorI(0, 0), VectorI(1, 0), VectorI( 0,  1) },
        IterGroup { VectorI(0, 0), VectorI(0, 1), VectorI( 1,  0) },
        IterGroup { VectorI(0, 0), VectorI(1, 0), VectorI( 1,  1) },
        IterGroup { VectorI(0, 0), VectorI(0, 1), VectorI( 1,  1) },
        IterGroup { VectorI(0, 0), VectorI(1, 0), VectorI(-1,  1) },
        IterGroup { VectorI(blocks.width() - 1, 0), VectorI(0, 1), VectorI(-1,  1) }
    };
    for (const auto & group : k_scan_list) {
    for (auto r = group.beg; blocks.has_position(r); r += group.step) {
        int count = 0;
        for (auto t = r; blocks.has_position(t); t += group.sweep_step) {
            if (blocks(t) == k_empty_block) {
                if (count >= pop_requirement) {}
                count = 0;
            } else if (count == 0 && blocks(t) != k_empty_block) {
                count = 1;
            } else if (count && blocks(t) != blocks(t - count*group.sweep_step)) {
                if (count >= pop_requirement) {}
                count = 1;
            } else if (count && blocks(t) == blocks(t - count*group.sweep_step)) {
                ++count;
            }
        }
        if (count >= pop_requirement) {

        }
    }}
    return any_popped;
}

int clear_tetris_rows(Grid<int> & blocks, PopEffects & effects) {
    effects.start();
    auto finisher = make_finisher(effects);

    int rv = 0;
    for (int y = 0; y != blocks.height(); ++y) {
        bool is_filled = true;
        for (int x = 0; x != blocks.width(); ++x) {
            if (blocks(x, y) == k_empty_block) is_filled = false;
        }
        if (is_filled) {
            ++rv;
            for (int x = 0; x != blocks.width(); ++x) {
                effects.post_pop_effect(VectorI(x, y), blocks(x, y));
                blocks(x, y) = k_empty_block;
            }
        }
    }
    return rv;
}

namespace {

std::array<VectorI, 4> get_neighbor_positions_for(VectorI v) {
    return { VectorI(1, 0) + v, VectorI(-1, 0) + v,
             VectorI(0, 1) + v, VectorI(0, -1) + v };
}

void pop_special_neighbors(Grid<int> & grid, VectorI location, PopEffects & effects) {
    for (auto n : get_neighbor_positions_for(location)) {
        if (!grid.has_position(n)) continue;
        switch (grid(n)) {
        case 6: case 7:
            effects.post_pop_effect(n, grid(n));
            grid(n) = decay_block(grid(n));
            break;
        default: break;
        }
    }
}


} // end of <anonymous> namespace
