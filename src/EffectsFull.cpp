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

#include "EffectsFull.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <common/SfmlVectorTraits.hpp>

#include <random>

#include <cassert>

namespace {

using Rng = std::default_random_engine;
using cul::convert_to;

template <typename T, bool (*del_f)(const T &)>
void remove_from_container(std::vector<T> &);

} // end of <anonymous> namespace

void FallEffectsFull::restart() {
    m_fall_effects.clear();
    start();
}

void FallEffectsFull::setup
    (int board_width, int board_height, const sf::Texture & texture)
{
    auto emp = k_empty_block;
    m_blocks_copy.set_size(board_width, board_height, std::move(emp));
    m_texture = &texture;

    Rng rng { std::random_device()() };
    m_rates_for_col.resize(board_width, 1.);
    for (auto & rate : m_rates_for_col) {
        rate = std::uniform_real_distribution<double>(0.75, 1.2)(rng)*double(board_height);
    }
}

void FallEffectsFull::update(double et) {
    for (auto & effect : m_fall_effects) {
        effect.fall_position += et*effect.rate;
        if (effect_is_finished(effect) && m_blocks_copy.has_position(effect.to)) {
            m_blocks_copy(effect.to) = effect.color;
        }
    }
    m_fall_effects.erase(std::remove_if(m_fall_effects.begin(), m_fall_effects.end(),
        effect_is_finished), m_fall_effects.end());
}

bool FallEffectsFull::has_effects() const {
    return !m_fall_effects.empty();
}

void FallEffectsFull::do_fall_in
    (BlockGrid & original_board, const BlockGrid & board_of_fallins)
{
    if (original_board.width () != board_of_fallins.width () ||
        original_board.height() != board_of_fallins.height())
    {
        throw std::invalid_argument("FallEffectsFull::do_fall_in: board size mismatch");
    }
    if (original_board.height() == 0 || original_board.width() == 0)
        { return; }

    const auto end_position = board_of_fallins.end_position();
    m_blocks_copy.set_size(original_board.width(), original_board.height());
    start();
    for (VectorI r; r != end_position; r = board_of_fallins.next(r)) {
        if (original_board(r) != k_empty_block) {
            post_stationary_block(r, original_board(r));
        }
    }
    for (int x = 0; x != original_board.width(); ++x) {
        // find the lowest empty space
        // (assumes all blocks have fallen)
        int lowest_empty = original_board.height() - 1;
        for (int y = 0; y != original_board.height(); ++y) {
            if (original_board(x, y) == k_empty_block) continue;
            lowest_empty = y - 1;
            break;
        }
        // no space available for fallins
        if (lowest_empty == -1) continue;
        // start from the bottom of the fallins grid
        int fallins_y = board_of_fallins.height() - 1;
        // iterate empty blocks
        for (int y = lowest_empty; y != -1; --y) {
            // find first fallin block
            for ( ;fallins_y != -1; --fallins_y) {
                if (board_of_fallins(x, fallins_y) != k_empty_block) { break; }
            }
            // breaks once we're out of fallins
            if (fallins_y == -1) { break; }
            assert(board_of_fallins(x, fallins_y) != k_empty_block);
            assert(original_board(x, y) == k_empty_block);

            auto fallins_block = board_of_fallins(x, fallins_y);
            original_board(x, y) = fallins_block;
            post_block_fall(VectorI(x, y - lowest_empty - 1),
                            VectorI(x, y), fallins_block);
            --fallins_y;
        }
    }
    finish();
}

/* private */ void FallEffectsFull::start() {
    for (auto & c : m_blocks_copy) c = k_empty_block;
}

/* private */ void FallEffectsFull::post_stationary_block
    (VectorI at, BlockId color)
{
    m_blocks_copy(m_transf_v(at)) = color;
}

/* private */ void FallEffectsFull::post_block_fall
    (VectorI from, VectorI to, BlockId color)
{
    if (from == to) {
        throw std::invalid_argument("FallEffectsFull::post_block_fall: from and to cannot be the same location");
    }
    FallEffect effect;
    effect.to    = m_transf_v(to  );
    effect.from  = m_transf_v(from);
    effect.color = color;
    effect.rate  = m_rates_for_col[from.x];
    m_fall_effects.push_back(effect);
}

/* private */ void FallEffectsFull::finish() {}

/* private */ void FallEffectsFull::draw
    (sf::RenderTarget & target, sf::RenderStates states) const
{
    if (!has_effects()) return;
    using VectorF = sf::Vector2<float>;
    sf::Sprite brush;
    brush.setTexture(*m_texture);

    for (const auto & effect : m_fall_effects) {
        auto rd = cul::normalize(convert_to<VectorF>(effect.to - effect.from));
        rd *= float(effect.fall_position*k_block_size);
        brush.setTextureRect(texture_rect_for(effect.color));
        brush.setColor(base_color_for_block(effect.color));
        brush.setPosition(rd + convert_to<VectorF>(effect.from*k_block_size));
        target.draw(brush, states);
    }
    brush.setPosition(0.f, 0.f);
    using Fp = void(*)(const ConstBlockSubGrid &, const sf::Sprite &, sf::RenderTarget &, sf::RenderStates);
    (m_render_merged ? Fp(render_merged_blocks) : Fp(render_blocks))(m_blocks_copy, brush, target, states);
}

// ----------------------------------------------------------------------------

/* static */ const VectorD PopEffectsPartial::CharEffect::k_velocity(0, -33);

void PopEffectsPartial::update(double et) {
    for (auto & effect : m_flash_effects) {
        effect.remaining -= et;
        if (ready_to_delete(effect)) {
            spawn_piece_effects(effect);
        }
    }

    static const VectorD k_gravity(0, PieceEffect::k_gravity);
    for (auto & effect : m_piece_effects) {
        effect.velocity += k_gravity*et;
        effect.location += effect.velocity*et;
        effect.remaining -= et;
    }

    for (auto & effect : m_char_effects) {
        effect.remaining -= et;
        effect.location += et*CharEffect::k_velocity;
    }

    remove_from_container<FlashEffect, ready_to_delete>(m_flash_effects);
    remove_from_container<PieceEffect, ready_to_delete>(m_piece_effects);
    remove_from_container<CharEffect , ready_to_delete>(m_char_effects );
}

bool PopEffectsPartial::has_effects() const {
    return !m_flash_effects.empty() || !m_piece_effects.empty() || !m_char_effects.empty();
}

/* protected */ void PopEffectsPartial::start() {

}

/* protected */ void PopEffectsPartial::finish() {

}

/* protected */ void PopEffectsPartial::post_pop_effect(VectorI at, BlockId block_id) {
    // we can insert further pop behaviors here
    FlashEffect effect;
    effect.at = at;
    effect.block_id = block_id;
    m_flash_effects.push_back(effect);
    if (!m_blocks_copy.has_position(at)) {
        throw std::runtime_error("PopEffectsFull::post_pop_effect: pop effects "
                                 "does not have its own copy of the board, this "
                                 "can be fixed by calling the do_pop member function");
    }
    m_blocks_copy(at) = decay_block(m_blocks_copy(at));
}

/* protected */ void PopEffectsPartial::post_number(VectorI at, int delta) {
    auto delta_string = (delta < 0 ? "-" : "+") + std::to_string(delta);
    auto char_width = texture_rect_for_char('+').width;
    auto width = int(delta_string.size())*char_width;
    auto height = texture_rect_for_char('+').height;
    auto start = at*k_block_size + VectorI(1, 1)*char_width - VectorI(width, height)/2;
    for (auto c : delta_string) {
        CharEffect effect;
        effect.identity = c;
        effect.location = VectorD(start);
        start.x += char_width;
        m_char_effects.push_back(effect);
    }
}

/* private */ void PopEffectsPartial::draw
    (sf::RenderTarget & target, sf::RenderStates states) const
{
    if (!has_effects()) return;
    sf::Sprite brush;
    brush.setTexture(*m_texture);
    render_merged_blocks(m_blocks_copy, brush, target, states);
    for (const auto & effect : m_flash_effects) {
        draw_flash_effect(target, states, effect);
    }
    for (const auto & effect : m_piece_effects) {
        draw_piece_effect(target, states, effect);
    }
    for (const auto & effect : m_char_effects) {
        draw_char_effect(target, states, effect);
    }
}

/* private */ void PopEffectsPartial::draw_flash_effect
    (sf::RenderTarget & target, sf::RenderStates states,
     const FlashEffect & effect) const
{
    sf::Sprite brush;
    brush.setTexture(*m_texture);
    brush.setTextureRect(texture_rect_for(effect.block_id));
    brush.setColor(brighten_by(base_color_for_block(effect.block_id), effect));
    brush.setPosition(float(effect.at.x*k_block_size), float(effect.at.y*k_block_size));
    target.draw(brush, states);
}

/* private */ void PopEffectsPartial::draw_piece_effect
    (sf::RenderTarget & target, sf::RenderStates states,
     const PieceEffect & effect) const
{
    static const auto get_decayed_color = [](const PieceEffect & effect) {
        //static constexpr const auto k_init_rem = PieceEffect::k_init_remaining;
        int darken_amount = int(std::round(
            ((k_init_remaining - effect.remaining) / k_init_remaining)*200
        ));
        assert(darken_amount + 55 <= 255);
        sf::Color rv = effect.color;
        rv.r = std::max(0, rv.r - darken_amount);
        rv.g = std::max(0, rv.g - darken_amount);
        rv.b = std::max(0, rv.b - darken_amount);
        rv.a = int(std::round((effect.remaining / k_init_remaining)*200)) + 55;
        return rv;
    };
    assert(effect.block_id != k_empty_block);
    auto trect = texture_rect_for(effect.block_id);
    trect.width  /= 2;
    trect.height /= 2;
    trect.top  += effect.texture_offset.y*(k_block_size / 2);
    trect.left += effect.texture_offset.x*(k_block_size / 2);
    auto color = get_decayed_color(effect);

    sf::Sprite spt;
    spt.setTexture(*m_texture);
    spt.setTextureRect(trect);
    spt.setPosition(convert_to<sf::Vector2f>(effect.location));
    spt.setColor(color);
    target.draw(spt, states);
}

/* private */ void PopEffectsPartial::draw_char_effect
    (sf::RenderTarget & target, sf::RenderStates states,
     const CharEffect & effect) const
{
    sf::Sprite brush;
    brush.setTexture(*m_texture);
    brush.setTextureRect(texture_rect_for_char(effect.identity));
    brush.setPosition(convert_to<sf::Vector2f>(effect.location));
    target.draw(brush, states);
}

/* private */ void PopEffectsPartial::spawn_piece_effects
    (const FlashEffect & flash_effect)
{
    using std::make_pair;
    static constexpr const auto k_pi = cul::k_pi_for_type<double>;

    using RealDistri = std::uniform_real_distribution<double>;
    RealDistri top_interval   (0, k_pi / 4.);
    RealDistri bottom_interval(0, k_pi / 5.);
    auto & rng = m_rng;

    auto piece_list = {
        make_pair(VectorI(0, 0), -(1./8. + 0.5)*k_pi - top_interval   (rng)),
        make_pair(VectorI(0, 1), -(1./6. + 0.5)*k_pi - bottom_interval(rng)),
        make_pair(VectorI(1, 0),  (1./8. - 0.5)*k_pi + top_interval   (rng)),
        make_pair(VectorI(1, 1),  (1./6. - 0.5)*k_pi + bottom_interval(rng))
    };

    static constexpr const auto k_init_speed = PieceEffect::k_init_speed;
    static const auto ucircle = [](double t)
        { return VectorD(std::cos(t), std::sin(t)); };
    sf::Color init_color = sf::Color::White;
    if (is_block_color(flash_effect.block_id)) {
        init_color = brighten_color(base_color_for_block(flash_effect.block_id), 1.);
    }

    for (auto [offset, angle] : piece_list) {
        PieceEffect effect;
        effect.color = init_color;
        effect.texture_offset = offset;
        effect.velocity = ucircle(angle)*k_init_speed;
        effect.location = VectorD(flash_effect.at)*double(k_block_size);
        effect.location += VectorD(offset)*double(k_block_size / 2);
        effect.block_id = flash_effect.block_id;
        m_piece_effects.push_back(effect);
    }
}

namespace {

template <typename T, bool (*del_f)(const T &)>
void remove_from_container(std::vector<T> & cont) {
    cont.erase(std::remove_if(cont.begin(), cont.end(), del_f), cont.end());
}

} // end of <anonymous> namespace
