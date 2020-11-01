#pragma once

#include "Defs.hpp"

#include <common/SubGrid.hpp>

class FallBlockEffects {
public:
    static FallBlockEffects & default_instance();

    virtual void start() = 0;
    virtual void post_stationary_block(VectorI at, int block_id) = 0;
    virtual void post_block_fall(VectorI from, VectorI to, int block_id) = 0;
    virtual void finish() = 0;
protected:
    FallBlockEffects() {}
    virtual ~FallBlockEffects();
};

void make_blocks_fall
    (SubGrid<int>, FallBlockEffects & = FallBlockEffects::default_instance());

void make_tetris_rows_fall
    (SubGrid<int>, FallBlockEffects & = FallBlockEffects::default_instance());

class PopEffects {
public:
    static PopEffects & default_instance();

    virtual void start() = 0;
    virtual void finish() = 0;
    virtual void post_pop_effect(VectorI at, int block_id) = 0;
protected:
    PopEffects() {}
    virtual ~PopEffects();
};

// this is a pretty intense algorithm
// so solid and numerous test cases are necessary
void select_connected_blocks
    (const ConstSubGrid<int> &, std::vector<VectorI> & selected, Grid<bool> & explored);

std::vector<VectorI> select_connected_blocks(const Grid<int> &, VectorI);

bool pop_connected_blocks(Grid<int> &, int amount_required, PopEffects & = PopEffects::default_instance());

int clear_tetris_rows(Grid<int> &, PopEffects & = PopEffects::default_instance());
