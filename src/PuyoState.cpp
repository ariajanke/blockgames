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

VectorI get_spawn_point(const ConstSubGrid<int> &);

std::string pad_to_right(std::string &&, int);

} // end of <anonymous> namespace

// ----------------------------------------------------------------------------

const std::pair<int, int> PuyoBoard::k_empty_pair = std::make_pair(k_empty_block, k_empty_block);

void PuyoBoard::set_size(int width, int height) {
    m_blocks.clear();
    m_blocks.set_size(width, height, k_empty_block);
    m_fef.setup(m_blocks.width(), m_blocks.height(), load_builtin_block_texture());
    m_pef.assign_texture(load_builtin_block_texture());
}

void PuyoBoard::assign_score_board
    (int this_board_number, PuyoScoreBoardBase & sb_ptr)
{
    m_score_board_number = this_board_number;
    m_score_board        = &sb_ptr;
}

void PuyoBoard::set_settings(double fall_speed, int pop_requirement) {
    if (fall_speed <= 0.) {
        throw std::invalid_argument("PuyoBoard::set_settings: fall speed must be a positive real number.");
    }
    m_fall_delay = 1. / fall_speed;
    m_pop_requirement = pop_requirement;
}

void PuyoBoard::update(double et) {
    assert(!is_gameover() && is_ready());
    assert(m_update_func);
    std::invoke(m_update_func, this, et);
}

void PuyoBoard::push_falling_piece(int first, int second) {
    if (m_piece.color() == k_empty_block) {
        assert(m_piece.other_color() == k_empty_block);
        m_piece = FallingPiece(first, second);
        m_update_func = nullptr;
    } else {
        assert(m_piece.other_color() != k_empty_block);
        m_next_piece = std::make_pair(first, second);
        m_score_board->set_next_pair(m_score_board_number, first, second);
        m_piece.set_location(get_spawn_point(m_blocks));
        m_update_func = &PuyoBoard::update_piece;
    }
}

void PuyoBoard::push_fall_in_blocks(const BlockGrid & blocks_) {
    m_fef.do_fall_in(m_blocks, blocks_);
    m_update_func = &PuyoBoard::update_fall_effects;
}

bool PuyoBoard::is_ready() const {
    return m_update_func;
}

bool PuyoBoard::is_gameover() const {
    return m_update_func == &PuyoBoard::update_on_gameover && !m_fef.has_effects();
}

/* private */ void PuyoBoard::draw(sf::RenderTarget & target, sf::RenderStates states) const {
    if (m_pef.has_effects()) {
        target.draw(m_pef, states);
    } else if (m_fef.has_effects()) {
        target.draw(m_fef, states);
    } else {
        sf::Sprite brush;
        brush.setTexture(load_builtin_block_texture());
        render_merged_blocks(m_blocks, brush, target, states);
    }

    bool bottom_is_open = [this]() {
        auto lower_loc = m_piece.location() + VectorI(0, 1);
        auto lower_o_loc = m_piece.other_location() + VectorI(0, 1);
        if (!m_blocks.has_position(lower_loc) ||
            !m_blocks.has_position(lower_o_loc))
        { return false; }
        return m_blocks(lower_loc  ) == k_empty_block &&
               m_blocks(lower_o_loc) == k_empty_block;
    } ();
    int y_offset = 0;
    if (bottom_is_open) {
        y_offset = int(std::round((m_fall_time / m_fall_delay)*double(k_block_size)));
    }

    sf::Sprite brush;
    brush.setTexture(load_builtin_block_texture());
    auto draw_block = [&target, &brush, &states](int color, VectorI r) {
        brush.setTextureRect(texture_rect_for(color));
        brush.setPosition(sf::Vector2f(r));
        brush.setColor(base_color_for_block(color));
        target.draw(brush, states);
    };

    // draw falling piece
    if (m_update_func == &PuyoBoard::update_piece) {
        DrawRectangle drect;
        drect.set_position(sf::Vector2f(m_piece.location()*k_block_size) +
                           sf::Vector2f(0.f, y_offset));
        drect.set_color(sf::Color::White);
        drect.set_size(float(k_block_size), float(k_block_size));
        target.draw(drect, states);

        draw_block(m_piece.color(), m_piece.location()*k_block_size + VectorI(0, y_offset));
        draw_block(m_piece.other_color(), m_piece.other_location()*k_block_size + VectorI(0, y_offset));
    }
}

/* private */ void PuyoBoard::update_piece(double et) {
    assert(is_block_color(m_piece.color()) && is_block_color(m_piece.other_color()));
    if ((m_fall_time += et*fall_multiplier()) <= m_fall_delay) return;

    m_fall_time = 0.;
    if (!m_piece.descend(m_blocks)) {
        if (m_blocks(get_spawn_point(m_blocks)) != k_empty_block) {
            // on loss
            make_all_blocks_fall_out(m_blocks, m_fef);
            m_update_func = &PuyoBoard::update_on_gameover;
            return;
        }
        // merge blocks
        if (m_blocks.has_position(m_piece.location())) {
            m_blocks(m_piece.location()) = m_piece.color();
        }
        if (m_blocks.has_position(m_piece.other_location())) {
            m_blocks(m_piece.other_location()) = m_piece.other_color();
        }
        m_piece = FallingPiece(m_next_piece.first, m_next_piece.second);
        m_piece.set_location(get_spawn_point(m_blocks));
        m_next_piece = k_empty_pair;
        m_fef.restart();
        make_blocks_fall(m_blocks, m_fef);
        m_update_func = &PuyoBoard::update_fall_effects;
    }
}

/* private */ void PuyoBoard::update_pop_effects(double et) {
    if (m_pef.has_effects()) {
        m_pef.update(et);
    } else {
        make_blocks_fall(m_blocks, m_fef);
        m_update_func = &PuyoBoard::update_fall_effects;
    }
}

/* private */ void PuyoBoard::update_fall_effects(double et) {
    if (m_fef.has_effects()) {
        m_fef.update(et);
    } else if (m_pef.do_pop(m_blocks,  m_pop_requirement)) {
        m_update_func = &PuyoBoard::update_pop_effects;
    } else {
        // after pop
#       if 0
        m_score += m_pef.get_score_delta_and_reset_wave_number();
        m_score = std::min(k_max_possible_score, m_score);
#       endif
        m_score_board->increment_score(m_score_board_number, m_pef.get_score_delta_and_reset_wave_number());
        m_update_func = nullptr;
#       if 0
        assert(m_blocks(get_spawn_point(m_blocks)) == k_empty_block);
        m_piece = FallingPiece(m_next_piece.first, m_next_piece.second);
        m_piece.set_location(get_spawn_point(blocks()));
        m_next_piece = k_empty_pair;
#       endif
        // I need to signal that a turn has changed...
#       if 0
        handle_response(m_current_scenario->on_turn_change());
#       endif
    }
}

/* private */ void PuyoBoard::update_on_gameover(double et) {
    if (m_fef.has_effects()) {
        m_fef.update(et);
    }
}

// ----------------------------------------------------------------------------

void PuyoScoreBoard::increment_score(int board, int delta) {
    if (board == 0) {
        m_first_player_score = std::min(k_max_possible_score, m_first_player_score + delta);
    }
}

void PuyoScoreBoard::reset_score(int board) {
    if (board == 0)
        m_first_player_score = 0;
}

void PuyoScoreBoard::set_next_pair(int board, int first, int second) {
    if (board != 0) return;
    m_next_piece = std::make_pair(first, second);
}

/* private */ void PuyoScoreBoard::draw
    (sf::RenderTarget & target, sf::RenderStates states) const
{
    sf::Sprite brush;
    brush.setTexture(load_builtin_block_texture());
    auto draw_block = [&target, &brush, &states](int color, VectorI r) {
        brush.setTextureRect(texture_rect_for(color));
        brush.setPosition(sf::Vector2f(r));
        brush.setColor(base_color_for_block(color));
        target.draw(brush, states);
    };
    if (m_next_piece.first != k_empty_block && m_next_piece.second != k_empty_block) {
        brush.setColor(sf::Color::White);
        brush.setPosition(sf::Vector2f(VectorI(0, 2)*k_block_size));
        brush.setTextureRect(texture_rect_for_next());
        target.draw(brush, states);

        draw_block(m_next_piece.first , VectorI(0, 3)*k_block_size);
        draw_block(m_next_piece.second, VectorI(0, 4)*k_block_size);
    }

    brush.setTextureRect(texture_rect_for_score());
    brush.setColor(sf::Color::White);
    brush.setPosition(sf::Vector2f(VectorI(0, 0)*k_block_size));
    target.draw(brush, states);

    brush.setPosition(sf::Vector2f(VectorI(0, 1)*k_block_size));
    {
    for (char c : pad_to_right(std::to_string(m_first_player_score), 6)) {
        if (c != ' ') {
            brush.setTextureRect(texture_rect_for_char(c));
            target.draw(brush, states);
        }
        brush.move(float(texture_rect_for_char('0').width), 0.f);
    }
    }
}

// ----------------------------------------------------------------------------

PuyoStateN::PuyoStateN(int scenario_number) {
    m_current_scenario = Scenario::get_all_scenarios().at(scenario_number)->clone();
}

/* private */ void PuyoStateN::setup_board(const Settings &) {
    auto params = m_current_scenario->setup(Settings::Puyo());
    m_board.set_settings(params.fall_speed, params.pop_requirement);
    m_board.assign_score_board(0, m_score_board);
    m_board.assign_pause_pointer(m_pause);
    m_board.set_size(params.width, params.height);
    m_current_scenario->assign_board(m_board.blocks());
    set_max_colors(params.colors);
    while (!m_board.is_ready()) {
        handle_response(m_current_scenario->on_turn_change());
    }
}

/* private */ void PuyoStateN::update(double et) {
    BoardState::update(et);

    if (m_pause) return;

    m_board.update(et);
    while (m_board.is_gameover() || !m_board.is_ready()) {
        handle_response(m_current_scenario->on_turn_change());
    }
}

/* private */ void PuyoStateN::draw(sf::RenderTarget & target, sf::RenderStates states) const {
    const auto & blocks = m_board.blocks();
    draw_fill_with_background(target, blocks.width(), blocks.height());
    draw_fill_with_score_background(target, 3, blocks.height(), VectorI(blocks.width(), 0)*k_block_size);

    target.draw(m_board, states);
    states.transform.translate( float( m_board.width()*k_block_size ), 0.f );
    target.draw(m_score_board, states);
}

/* private */ void PuyoStateN::handle_response(const Response & response) {
    if (!response.is_valid()) {
        throw std::runtime_error("idk what to do");
#       if 0
        make_blocks_fall(m_blocks, m_fef);
        m_update_func = &PuyoState::update_fall_effects;
        return;
#       endif

    }
    if (auto * cpair = response.as_pointer<std::pair<int, int>>()) {
        auto pair = *cpair;
        if (pair == Scenario::k_random_pair) {
            pair = std::make_pair(random_color(m_rng), random_color(m_rng));
        }
        m_board.push_falling_piece(pair.first, pair.second);
#       if 0
        m_piece = FallingPiece(m_next_piece.first, m_next_piece.second);
        m_next_piece = pair;

        m_piece.set_location(get_spawn_point(m_blocks));
        m_update_func = &PuyoState::update_piece;
#       endif
    } else if (auto * fallins = response.as_pointer<Grid<int>>()) {
        m_board.push_fall_in_blocks(*fallins);
#       if 0
        m_fef.do_fall_in(m_blocks, *fallins);
        m_update_func = &PuyoState::update_fall_effects;
#       endif
    }
}

// ----------------------------------------------------------------------------

void ScenarioPriv::DefDeleter::operator () (Scenario * ptr) const
    { ConstDefDeleter()(ptr); }

void ScenarioPriv::ConstDefDeleter::operator () (const Scenario * ptr) const
    { delete ptr; }

ConstScenarioPtr move(ScenarioPtr & ptr)
    { return ConstScenarioPtr(ptr.release()); }
#if 0
PuyoState::PuyoState(int scenario_number) {
    m_current_scenario = Scenario::get_all_scenarios().at(scenario_number)->clone();
}

void PuyoState::setup_board(const Settings &) {
    m_pef.assign_texture(load_builtin_block_texture());
    assert(m_current_scenario);

    auto params = m_current_scenario->setup(Settings::Puyo());
    m_pop_requirement = params.pop_requirement;
    m_fall_delay      = 1. / params.fall_speed;
    m_blocks.set_size(params.width, params.height);
    m_current_scenario->assign_board(m_blocks);
    m_fef.setup(m_blocks.width(), m_blocks.height(), load_builtin_block_texture());
    set_max_colors(params.colors);
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
        m_score = std::min(k_max_possible_score, m_score);
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

    if (m_next_piece.first != k_empty_block && m_next_piece.second != k_empty_block) {
        brush.setColor(sf::Color::White);
        brush.setPosition(sf::Vector2f(VectorI(m_blocks.width(), 2)*k_block_size));
        brush.setTextureRect(texture_rect_for_next());
        target.draw(brush);

        draw_block(m_next_piece.first , VectorI(m_blocks.width(), 3)*k_block_size);
        draw_block(m_next_piece.second, VectorI(m_blocks.width(), 4)*k_block_size);
    }

    brush.setTextureRect(texture_rect_for_score());
    brush.setColor(sf::Color::White);
    brush.setPosition(sf::Vector2f(VectorI(m_blocks.width(), 0)*k_block_size));
    target.draw(brush);

    brush.setPosition(sf::Vector2f(VectorI(m_blocks.width(), 1)*k_block_size));
    {
    for (char c : pad_to_right(std::to_string(std::min(m_score, k_max_possible_score)), 6)) {
        if (c != ' ') {
            brush.setTextureRect(texture_rect_for_char(c));
            target.draw(brush);
        }
        brush.move(float(texture_rect_for_char('0').width), 0.f);
    }
    }
}
#endif
namespace {

VectorI get_spawn_point(const ConstSubGrid<int> & board) {
    auto w = board.width();
    auto x = (w / 2) - (w % 2 ? 0 : 1);
    return VectorI(x, 0);
}

std::string pad_to_right(std::string && str, int pad) {
    if (int(str.length()) > pad) {
        throw std::invalid_argument("pad_to_right: string too large");
    }
    str.insert(str.begin(), pad - int(str.length()), ' ');
    return std::move(str);
}

} // end of <anonymous> namespace
