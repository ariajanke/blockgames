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
#include "BlockAlgorithm.hpp"
#include "Graphics.hpp"

#include <random>
#include <common/Util.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Color.hpp>

namespace sf { class Sprite; }

class FallEffectsFull final : public FallBlockEffects, public sf::Drawable {
public:
    using TransformVectorFunc = VectorI(*)(VectorI);
    void restart();
    void setup(int board_width, int board_height, const sf::Texture &);
    void update(double et);
    bool has_effects() const;

    void do_fall_in(BlockGrid & original_board, const BlockGrid & board_of_fallins);

    void set_vector_transform(TransformVectorFunc f) { m_transf_v = f; }
    void set_render_blocks_merged_enabled(bool b) { m_render_merged = b; }

    static VectorI identity_func(VectorI r) { return r; }
    static VectorI flip_xy(VectorI r) { return VectorI(r.y, r.x); }

private:
    struct FallEffect {
        BlockId color = k_empty_block;
        VectorI from, to;
        double fall_position = 0.;
        double rate = 1.;
    };

    static bool effect_is_finished(const FallEffect & effect)
        { return (magnitude(effect.from - effect.to) < effect.fall_position); }

    void start() override;
    void post_stationary_block(VectorI, BlockId) override;
    void post_block_fall(VectorI, VectorI, BlockId) override;
    void finish() override;
    void draw(sf::RenderTarget & target, sf::RenderStates states) const override;

    std::vector<FallEffect> m_fall_effects;
    std::vector<double> m_rates_for_col;
    BlockGrid m_blocks_copy;
    const sf::Texture * m_texture = nullptr;
    TransformVectorFunc m_transf_v = identity_func;
    bool m_render_merged = true;
};

// ----------------------------------------------------------------------------

class PopEffectsPartial : public PopEffects, public sf::Drawable {
public:
    void assign_texture(const sf::Texture & texture)
        { m_texture = &texture; }

    void update(double et);

    bool has_effects() const;

protected:
    PopEffectsPartial() {}
    ~PopEffectsPartial() override {}

    void set_internal_grid_copy(const BlockGrid & grid) {
        m_blocks_copy = grid;
    }
    void start() override;
    void finish() override;
    void post_pop_effect(VectorI at, BlockId color) override;
    void post_number(VectorI at, int);

private:
    static constexpr const double k_init_remaining = .33;

    struct FlashEffect {
        double remaining = k_init_remaining;
        BlockId block_id = k_empty_block;
        VectorI at;
    };

    struct PieceEffect {
        static constexpr const double k_init_speed = 75.;
        static constexpr const double k_gravity    = 533.;
        BlockId block_id = k_empty_block;
        VectorD velocity;
        VectorD location;
        double remaining = k_init_remaining;
        sf::Color color;
        VectorI texture_offset;
    };

    struct CharEffect {
        static const VectorD k_velocity;
        VectorD location;
        char identity;
        double remaining = k_init_remaining;
    };

    static bool ready_to_delete(const FlashEffect & ef) { return ef.remaining <= 0.; }
    static bool ready_to_delete(const PieceEffect & ef) { return ef.remaining <= 0.; }
    static bool ready_to_delete(const CharEffect  & ef) { return ef.remaining <= 0.; }

    static sf::Color brighten_by(sf::Color c, const FlashEffect & effect) {
        return brighten_color(c, (k_init_remaining - effect.remaining) / k_init_remaining);
    }

    void draw(sf::RenderTarget &, sf::RenderStates) const override;

    void draw_flash_effect(sf::RenderTarget &, sf::RenderStates, const FlashEffect &) const;
    void draw_piece_effect(sf::RenderTarget &, sf::RenderStates, const PieceEffect &) const;
    void draw_char_effect (sf::RenderTarget &, sf::RenderStates, const CharEffect  &) const;

    void spawn_piece_effects(const FlashEffect &);

    std::vector<FlashEffect> m_flash_effects;
    std::vector<PieceEffect> m_piece_effects;
    std::vector<CharEffect > m_char_effects ;

    BlockGrid m_blocks_copy;
    std::default_random_engine m_rng = std::default_random_engine { std::random_device()() };
    const sf::Texture * m_texture = nullptr;
};

// ----------------------------------------------------------------------------

class PuyoPopEffects final : public PopEffectsPartial {
public:
    bool do_pop(BlockGrid & grid, int pop_requirement) {
        ++m_wave_number;
        m_group_number = 0;
        m_pop_requirement = pop_requirement;
        auto rv = pop_connected_blocks(grid, pop_requirement, *this);
        set_internal_grid_copy(grid);
        return rv;
    }

    int get_score_delta_and_reset_wave_number() {
        int rv = m_score_delta;
        m_score_delta = m_wave_number = 0;
        return rv;
    }

private:
    void post_pop_effect(VectorI at, BlockId color) override {
        PopEffectsPartial::post_pop_effect(at, color);
        if (!is_block_color(color)) return;
    }

    void post_group(const std::vector<VectorI> & group_locations) override {
        VectorI avg_tile;
        int group_size = int(group_locations.size());
        for (auto v : group_locations) avg_tile += v;
        avg_tile.x /= group_size;
        avg_tile.y /= group_size;
        int delta = group_size*m_wave_number;
        if (group_size > m_pop_requirement) {
            delta += group_size / m_pop_requirement;
        }
        delta += m_group_number;
        ++m_group_number;
        post_number(avg_tile, delta);
        m_score_delta += delta;
    }

    int m_wave_number = 0;
    int m_score_delta = 0;
    int m_pop_requirement = 0;
    int m_group_number = 0;
};

// ----------------------------------------------------------------------------

class SameGamePopEffects final : public PopEffectsPartial {
public:
    void do_pop(BlockGrid & grid, VectorI selection, bool pop_single_blocks) {
        if (!grid.has_position(selection)) {
            throw std::invalid_argument("SameGamePopEffects::do_pop: selection is outside of the grid.");
        }
        if (grid(selection) == k_empty_block) return;
        set_internal_grid_copy(grid);
        auto selections = select_connected_blocks(grid, selection);
        if (selections.size() == 1 && !pop_single_blocks) {
            return;
        }

        start();
        for (auto r : selections) {
            post_pop_effect(r, grid(r));
            grid(r) = k_empty_block;
        }
        finish();

    }

private:
    void post_group(const std::vector<VectorI> &) override {}
};
