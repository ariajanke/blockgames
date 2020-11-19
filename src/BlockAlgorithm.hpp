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

void make_all_blocks_fall_out(SubGrid<int>, FallBlockEffects &);

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

// wip
bool pop_columns_blocks(Grid<int> &, int pop_requirement, PopEffects & = PopEffects::default_instance());

int clear_tetris_rows(Grid<int> &, PopEffects & = PopEffects::default_instance());
