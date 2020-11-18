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

    void do_fall_in(Grid<int> & original_board, const Grid<int> & board_of_fallins);

    void set_vector_transform(TransformVectorFunc f) { m_transf_v = f; }
    void set_render_blocks_merged_enabled(bool b) { m_render_merged = b; }

    static VectorI identity_func(VectorI r) { return r; }
    static VectorI flip_xy(VectorI r) { return VectorI(r.y, r.x); }
private:
    struct FallEffect {
        int color = k_empty_block;
        VectorI from, to;
        double fall_position = 0.;
        double rate = 1.;
    };

    static bool effect_is_finished(const FallEffect & effect)
        { return (magnitude(effect.from - effect.to) < effect.fall_position); }

    void start() override;
    void post_stationary_block(VectorI, int) override;
    void post_block_fall(VectorI, VectorI, int) override;
    void finish() override;
    void draw(sf::RenderTarget & target, sf::RenderStates states) const override;

    std::vector<FallEffect> m_fall_effects;
    std::vector<double> m_rates_for_col;
    Grid<int> m_blocks_copy;
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

    void set_internal_grid_copy(const Grid<int> & grid) {
        m_blocks_copy = grid;
    }
    void start() override;
    void finish() override;
    void post_pop_effect(VectorI at, int color) override;
private:
    struct FlashEffect {
        static constexpr const double k_init_remaining = .33;
        double remaining = k_init_remaining;
        int block_id = k_empty_block;
        VectorI at;
    };

    struct PieceEffect {
        static constexpr const double k_init_remaining = 0.33;
        static constexpr const double k_init_speed     = 75.;
        static constexpr const double k_gravity        = 533.;
        int block_id = k_empty_block;
        VectorD velocity;
        VectorD location;
        double remaining = k_init_remaining;
        sf::Color color;
        VectorI texture_offset;
    };

    static sf::Color brighten_by(sf::Color c, const FlashEffect & effect) {
        static constexpr const auto k_init_rem = FlashEffect::k_init_remaining;
        return brighten_color(c, (k_init_rem - effect.remaining) / k_init_rem);
    }

    void draw(sf::RenderTarget &, sf::RenderStates) const override;

    void draw_flash_effect(sf::RenderTarget &, sf::RenderStates, const FlashEffect &) const;
    void draw_piece_effect(sf::RenderTarget &, sf::RenderStates, const PieceEffect &) const;
    void spawn_piece_effects(const FlashEffect &);

    std::vector<FlashEffect> m_flash_effects;
    std::vector<PieceEffect> m_piece_effects;

    Grid<int> m_blocks_copy;
    std::default_random_engine m_rng = std::default_random_engine { std::random_device()() };
    const sf::Texture * m_texture = nullptr;
};

// ----------------------------------------------------------------------------

class PuyoPopEffects final : public PopEffectsPartial {
public:
    bool do_pop(Grid<int> & grid, int pop_requirement) {
        set_internal_grid_copy(grid);
        ++m_wave_number;
        return pop_connected_blocks(grid, pop_requirement, *this);
    }

    int get_score_delta_and_reset_wave_number() {
        int rv = m_score_delta;
        m_score_delta = m_wave_number = 0;
        return rv;
    }

private:
    void post_pop_effect(VectorI at, int color) override {
        PopEffectsPartial::post_pop_effect(at, color);
        if (!is_block_color(color)) return;
        m_score_delta += m_wave_number;
    }

    int m_wave_number = 0;
    int m_score_delta = 0;
};

// ----------------------------------------------------------------------------

class SameGamePopEffects final : public PopEffectsPartial {
public:
    void do_pop(Grid<int> & grid, VectorI selection, bool pop_single_blocks) {
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
};

// ----------------------------------------------------------------------------

#if 0
void bind_block_to_sprite(sf::Sprite &, int block_id, TileEdges);
#endif
