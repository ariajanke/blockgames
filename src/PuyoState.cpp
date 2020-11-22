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

#include "PuyoState.hpp"
#include "PuyoScenario.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <cassert>

namespace {

VectorI get_spawn_point(const Grid<int> &);

} // end of <anonymous> namespace

void ScenarioPriv::DefDeleter::operator () (Scenario * ptr) const
    { ConstDefDeleter()(ptr); }

void ScenarioPriv::ConstDefDeleter::operator () (const Scenario * ptr) const
    { delete ptr; }

ConstScenarioPtr move(ScenarioPtr & ptr)
    { return ConstScenarioPtr(ptr.release()); }

void PuyoState::setup_board(const Settings & settings) {
    m_pef.assign_texture(load_builtin_block_texture());

    if (!m_current_scenario) {
        m_current_scenario = Scenario::make_glass_waves();
    }
    m_current_scenario->assign_board(m_blocks);
    Scenario::Params p;
    m_current_scenario->setup(p);

    static constexpr const auto k_use_freeplay = Scenario::k_use_freeplay_settings;
    {
    const auto & conf = settings.puyo;
    if (p.board_height == k_use_freeplay) {
        p.board_height = conf.height;
    }
    if (p.board_width == k_use_freeplay) {
        p.board_width = conf.width;
    }
    if (p.max_colors == k_use_freeplay) {
        p.max_colors = conf.colors;
    }
    m_pop_requirement = settings.puyo.pop_requirement;
    m_fall_delay = 1. / settings.puyo.fall_speed;
    }
    m_blocks.set_size(p.board_width, p.board_height);
    m_fef.setup(p.board_width, p.board_height, load_builtin_block_texture());
    set_max_colors(p.max_colors);
    handle_response(m_current_scenario->on_turn_change());
    handle_response(m_current_scenario->on_turn_change());
}

void PuyoState::update(double et) {
    PauseableWithFallingPieceState::update(et);
    if (is_paused()) return;
    assert(!m_blocks.is_empty());
    std::invoke(m_update_func, *this, et);
}

void PuyoState::update_piece(double et) {
    assert(is_block_color(m_piece.color()) && is_block_color(m_piece.other_color()));
    if ((m_fall_time += et*fall_multiplier()) <= m_fall_delay) return;

    m_fall_time = 0.;
    if (!m_piece.descend(m_blocks)) {
        if (m_blocks(get_spawn_point(m_blocks)) != k_empty_block) {
            // on loss
            make_all_blocks_fall_out(m_blocks, m_fef);
            m_update_func = &PuyoState::update_fall_effects;
            return;
        }
        // merge blocks
        if (m_blocks.has_position(m_piece.location())) {
            m_blocks(m_piece.location()) = m_piece.color();
        }
        if (m_blocks.has_position(m_piece.other_location())) {
            m_blocks(m_piece.other_location()) = m_piece.other_color();
        }
        m_fef.restart();
        make_blocks_fall(m_blocks, m_fef);
        m_update_func = &PuyoState::update_fall_effects;
    }
}

void PuyoState::update_pop_effects(double et) {
    if (m_pef.has_effects()) {
        m_pef.update(et);
    } else {
        make_blocks_fall(m_blocks, m_fef);
        m_update_func = &PuyoState::update_fall_effects;
    }
}

void PuyoState::update_fall_effects(double et) {
    if (m_fef.has_effects()) {
        m_fef.update(et);
    } else if (m_pef.do_pop(m_blocks,  m_pop_requirement)) {
        m_update_func = &PuyoState::update_pop_effects;
    } else {
        // after pop
        m_score += m_pef.get_score_delta_and_reset_wave_number();
        handle_response(m_current_scenario->on_turn_change());
    }
}

void PuyoState::handle_response(const Response & response) {
    if (!response.is_valid()) {
        make_blocks_fall(m_blocks, m_fef);
        m_update_func = &PuyoState::update_fall_effects;
        return;
    }
    if (auto * cpair = response.as_pointer<std::pair<int, int>>()) {
        auto pair = *cpair;
        if (pair == Scenario::k_random_pair) {
            auto & [f, s] = pair;
            f = random_color(m_rng);
            s = random_color(m_rng);
        }

        m_piece = FallingPiece(m_next_piece.first, m_next_piece.second);
        m_next_piece = pair;

        m_piece.set_location(get_spawn_point(m_blocks));
        m_update_func = &PuyoState::update_piece;
    } else if (auto * fallins = response.as_pointer<Grid<int>>()) {
        m_fef.do_fall_in(m_blocks, *fallins);
        m_update_func = &PuyoState::update_fall_effects;
    }
}

/* private */ void PuyoState::draw
    (sf::RenderTarget & target, sf::RenderStates) const
{
    draw_fill_with_background(target, m_blocks.width(), m_blocks.height());
    draw_fill_with_score_background(target, 3, m_blocks.height(), VectorI(m_blocks.width(), 0)*k_block_size);

    if (m_pef.has_effects()) {
        target.draw(m_pef);
    } else if (m_fef.has_effects()) {
        target.draw(m_fef);
    } else {
        sf::Sprite brush;
        brush.setTexture(load_builtin_block_texture());
        render_merged_blocks(m_blocks, brush, target);
    }

    bool bottom_is_open = [this]() {
        auto lower_loc = m_piece.location() + VectorI(0, 1);
        auto lower_o_loc = m_piece.other_location() + VectorI(0, 1);
        if (!m_blocks.has_position(lower_loc) ||
            !m_blocks.has_position(lower_o_loc))
        { return false; }
        return m_blocks(lower_loc  ) == k_empty_block &&
               m_blocks(lower_o_loc) == k_empty_block;
    }();
    int y_offset = 0;
    if (bottom_is_open) {
        y_offset = int(std::round((m_fall_time / m_fall_delay)*double(k_block_size)));
    }

    sf::Sprite brush;
    brush.setTexture(load_builtin_block_texture());
    auto draw_block = [&target, &brush](int color, VectorI r) {
        brush.setTextureRect(texture_rect_for(color));
        brush.setPosition(sf::Vector2f(r));
        brush.setColor(base_color_for_block(color));
        target.draw(brush);
    };

    // draw falling piece
    if (m_update_func == &PuyoState::update_piece) {
        DrawRectangle drect;
        drect.set_position(sf::Vector2f(m_piece.location()*k_block_size) +
                           sf::Vector2f(0.f, y_offset));
        drect.set_color(sf::Color::White);
        drect.set_size(float(k_block_size), float(k_block_size));
        target.draw(drect);

        draw_block(m_piece.color(), m_piece.location()*k_block_size + VectorI(0, y_offset));
        draw_block(m_piece.other_color(), m_piece.other_location()*k_block_size + VectorI(0, y_offset));
    }

    brush.setColor(sf::Color::White);
    brush.setPosition(sf::Vector2f(VectorI(m_blocks.width(), 2)*k_block_size));
    brush.setTextureRect(texture_rect_for_next());
    target.draw(brush);

    draw_block(m_next_piece.first , VectorI(m_blocks.width(), 3)*k_block_size);
    draw_block(m_next_piece.second, VectorI(m_blocks.width(), 4)*k_block_size);

    brush.setTextureRect(texture_rect_for_score());
    brush.setColor(sf::Color::White);
    brush.setPosition(sf::Vector2f(VectorI(m_blocks.width(), 0)*k_block_size));
    target.draw(brush);

    brush.setPosition(sf::Vector2f(VectorI(m_blocks.width(), 1)*k_block_size));
    {
    for (char c : std::to_string(m_score)) {
        brush.setTextureRect(texture_rect_for_char(c));
        target.draw(brush);
        brush.move(float(texture_rect_for_char(c).width), 0.f);
    }
    }
}

namespace {

VectorI get_spawn_point(const Grid<int> & board) {
    auto w = board.width();
    auto x = (w / 2) - (w % 2 ? 0 : 1);
    return VectorI(x, 0);
}

} // end of <anonymous> namespace
