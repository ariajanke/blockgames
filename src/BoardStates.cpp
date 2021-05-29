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

#include "BoardStates.hpp"
#include "Graphics.hpp"
#include "PuyoScenario.hpp"
#include "DialogState.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <common/SfmlVectorTraits.hpp>
#include <common/DrawRectangle.hpp>

#include <functional>
#include <variant>
#include <unordered_set>

#include <cassert>

namespace {

using BoardOptions = BoardState::BoardOptions;
using InvArg       = std::invalid_argument;
using cul::convert_to;

} // end of <anonymous> namespace

/* protected */ void BoardState::process_ui_event(const asgl::Event & event) {
#   if 0
    if (event.type == sf::Event::KeyReleased) {
        if (event.key.code == sf::Keyboard::Escape) {
            set_next_state(std::make_unique<DialogState>());
        }
    }
#   endif
    if (auto * keyrel = event.as_pointer<asgl::KeyRelease>()) {
        if (keyrel->key == asgl::keys::k_escape) {
            set_next_state(std::make_unique<DialogState>());
        }
    }
    // uh oh...
    // this is designed to handle event that maybe out of scope for asgl?
#   if 0
    m_pc_handler.update(event);
#   endif
}

/* protected */ void BoardState::update(double) {
#   if 0
    m_pc_handler.send_events(*this);
#   endif
}

/* private */ void BoardState::setup_(Settings & settings, const asgl::StyleMap &) {
    setup_board(settings);
}

/* protected */ void BoardState::set_max_colors(int n) {
    if (n < k_min_colors || n > k_max_colors) {
        throw InvArg("BoardState::set_max_colors: max colors must be in [" +
                     std::to_string(k_min_colors) + " " +
                     std::to_string(k_max_colors) + "].");
    }
    m_max_colors = n;
}

/* static */ void BoardState::draw_fill_with_background
    (sf::RenderTarget & target,
     int board_width, int board_height, VectorI offset, sf::Color mask)
{
    sf::Sprite brush;
    brush.setColor(mask);

    brush.setTexture(BuiltinBlockGraphics::as_texture());
    brush.setTextureRect(texture_rect_for_background());
    for (int y = 0; y != board_height; ++y) {
    for (int x = 0; x != board_width ; ++x) {
        brush.setPosition(convert_to<sf::Vector2f>(VectorI(x, y)*k_block_size + offset));
        target.draw(brush);
    }}
}

/* static */ void BoardState::draw_fill_with_score_background
    (sf::RenderTarget & target, int board_width, int board_height,
     VectorI offset)
{
    static const constexpr unsigned k_seed = 0x71EF2Bu;
    Rng rng { k_seed };
    sf::Sprite brush;
    brush.setTexture(BuiltinBlockGraphics::as_texture());
    for (int y = 0; y != board_height; ++y) {
    for (int x = 0; x != board_width ; ++x) {
        brush.setTextureRect(texture_rect_for_wood_board(IntDistri(0, k_wood_board_count - 1)(rng)));
        brush.setPosition(convert_to<sf::Vector2f>(VectorI(x, y)*k_block_size + offset));
        target.draw(brush);
    }}
}

// ----------------------------------------------------------------------------

void PauseableWithFallingPieceState::update(double et) {
    BoardState::update(et);
    if (is_paused()) return;
    if (m_move_dir == k_niether_dir) {
        m_move_time = 0.;
        return;
    }
    if ((m_move_time += et) >= k_move_delay) {
        switch (m_move_dir) {
        case PlayControlId::left:
            piece_base().move_left(blocks());
            break;
        case PlayControlId::right:
            piece_base().move_right(blocks());
            break;
        default:
            throw std::runtime_error("PauseableWithFallingPieceState::update: "
                                     "m_move_dir must be either right, left, or count.");
        }
        m_move_time = 0.;
    }
    m_move_dir = k_niether_dir;
}

void PauseableWithFallingPieceState::process_play_event(PlayControlEvent event) {
    if (event.id != PlayControlId::pause && m_is_paused) return;
    if (event.state == PlayControlState::just_pressed) {
        switch (event.id) {
        case PlayControlId::left:
            if (m_move_time == 0.)
                piece_base().move_left(blocks());
            break;
        case PlayControlId::right:
            if (m_move_time == 0.)
                piece_base().move_right(blocks());
            break;
        case PlayControlId::rotate_left : piece_base().rotate_left (blocks()); break;
        case PlayControlId::rotate_right: piece_base().rotate_right(blocks()); break;
        default: break;
        }
    }

    if (is_pressed(event) && (   event.id == PlayControlId::left
                              || event.id == PlayControlId::right))
    {
        m_move_dir = event.id;
    }

    if (   event.state == PlayControlState::just_pressed
        && event.id    == PlayControlId   ::pause       )
    { m_is_paused = !m_is_paused; }

    if (event.id == PlayControlId::down) {
        m_fall_multiplier = is_pressed(event) ? 5.*(blocks().height() / 10) : 1.;
    }
}

// ----------------------------------------------------------------------------

/* private */ void TetrisState::setup_board(const Settings & settings) {
    const auto & conf = settings.tetris;
    m_blocks.set_size(conf.width, conf.height);

    const auto & all_p = Polyomino::all_polyminos();
    assert(m_available_polyominos.empty());
    m_available_polyominos.reserve(all_p.size());
    assert(all_p.size() == PolyominoEnabledSet().size());
    for (std::size_t i = 0; i != all_p.size(); ++i) {
        if (conf.enabled_polyominos.test(i)) {
            m_available_polyominos.push_back(all_p[i]);
        }
    }
    // force dominos if the set is empty
    if (m_available_polyominos.empty()) {
        m_available_polyominos.push_back(all_p[0]);
    }

    reset_piece();
    m_fef.setup(conf.width, conf.height, BuiltinBlockGraphics::as_texture());
    m_fef.set_render_blocks_merged_enabled(false);
    set_max_colors(conf.colors);
}

/* private */ void TetrisState::update(double et) {
    PauseableWithFallingPieceState::update(et);
    if (is_paused()) return;
    if (m_fef.has_effects()) {
        m_fef.update(et);
    } else if ((m_fall_time += (et*fall_multiplier())) > m_fall_delay) {
        m_fall_time = 0.;
        auto temp = m_piece;
        if (!m_piece.move_down(m_blocks)) {
            temp.move_down(m_blocks);
            m_piece.place(m_blocks);
            reset_piece();
            clear_tetris_rows(m_blocks);
            make_tetris_rows_fall(m_blocks, m_fef);
        }
    }
}

/* private */ void TetrisState::draw(sf::RenderTarget & target, sf::RenderStates) const {
    draw_fill_with_background(target, m_blocks.width(), m_blocks.height());
    if (m_fef.has_effects()) {
        target.draw(m_fef);
    } else {
        sf::Sprite brush;
        brush.setTexture(BuiltinBlockGraphics::as_texture());
        for (int i = 0; i != m_piece.block_count(); ++i) {
            auto loc = m_piece.block_location(i);
            brush.setPosition(float(loc.x*k_block_size), float(loc.y*k_block_size));
            brush.setTextureRect(texture_rect_for(m_piece.block_color(i)));
            brush.setColor(base_color_for_block(m_piece.block_color(i)));
            target.draw(brush);
        }
        brush.setPosition(0.f, 0.f);
        render_blocks(m_blocks, brush, target);
    }
}

/* private */ void TetrisState::reset_piece() {
    assert(!m_available_polyominos.empty());
    const auto & polys = m_available_polyominos;
    const auto & piece = polys[IntDistri(0, polys.size() - 1)(m_rng)];
    m_piece = piece;
    m_piece.set_colors(map_int_to_color(k_min_colors + ( (&piece - &polys.front()) % k_max_colors )));
    m_piece.set_location(m_blocks.width() / 2, 0);

    if (m_piece.obstructed_by(m_blocks)) {
        make_all_blocks_fall_out(m_blocks, m_fef);
    }
}

// ----------------------------------------------------------------------------

namespace {

template <typename T>
void flip_along_trace(const Grid<T> &, Grid<T> &);

} // end of <anonymous> namespace

/* private */ void SameGame::setup_board(const Settings & settings) {
    const auto & conf = settings.samegame;
    m_pop_ef.assign_texture(BuiltinBlockGraphics::as_texture());
    m_blocks.set_size(conf.width, conf.height);
    m_fall_ef.setup(conf.width, conf.height, BuiltinBlockGraphics::as_texture());
    set_max_colors(conf.colors);
    for (auto & block : m_blocks) {
        block = random_color(m_rng);
    }
    m_pop_singles_enabled = !conf.gameover_on_singles;
}

/* private */ void SameGame::update(double et) {
    BoardState::update(et);
    if (m_pop_ef.has_effects()) {
        m_pop_ef.update(et);
        if (!m_pop_ef.has_effects()) {
            make_blocks_fall(m_blocks, m_fall_ef);
            if (!m_fall_ef.has_effects()) {
                try_sweep();
            }
        }        
    } else if (m_fall_ef.has_effects()) {
        m_fall_ef.update(et);
        if (!m_fall_ef.has_effects() && !m_blocks.is_empty()) {
            try_sweep();
        } else if (!m_fall_ef.has_effects() && !m_sweep_temp.is_empty()) {
            flip_along_trace(m_sweep_temp, m_blocks);
            m_sweep_temp.clear();
        }
    }
    assert(m_blocks.is_empty() ^ m_sweep_temp.is_empty());
}

/* private */ void SameGame::process_ui_event(const asgl::Event & event) {
    BoardState::process_ui_event(event);
    if (m_blocks.is_empty()) return;
    switch (event.type_id()) {
    case asgl::k_event_id_of<asgl::MouseMove>: {
        // no view-dependant transformations
        VectorI r = event.as<asgl::MouseMove>();
        r.x /= (k_block_size*scale());
        r.y /= (k_block_size*scale());
        r.x = std::max(std::min(r.x, m_blocks.width () - 1), 0);
        r.y = std::max(std::min(r.y, m_blocks.height() - 1), 0);
        m_selection = r;
        }
        break;
    case asgl::k_event_id_of<asgl::MouseRelease>:
        do_selection();
        break;
    default: break;
    }
#   if 0
    switch (event.type) {
    case sf::Event::MouseMoved: {
        // no view-dependant transformations
        VectorI r(event.mouseMove.x, event.mouseMove.y);
        r.x /= (k_block_size*scale());
        r.y /= (k_block_size*scale());
        r.x = std::max(std::min(r.x, m_blocks.width () - 1), 0);
        r.y = std::max(std::min(r.y, m_blocks.height() - 1), 0);
        m_selection = r;
        }
        break;
    case sf::Event::MouseButtonReleased:
        do_selection();
        break;
    default: break;
    }
#   endif
}

void SameGame::process_play_event(PlayControlEvent event) {
    if (event.state == PlayControlState::just_released) {
        switch (event.id) {
        case PlayControlId::down:
            ++m_selection.y;
            if (!m_blocks.has_position(m_selection)) {
                m_selection.y = 0;
            }
            break;
        case PlayControlId::up:
            --m_selection.y;
            if (!m_blocks.has_position(m_selection)) {
                m_selection.y = m_blocks.height() - 1;
            }
            break;
        case PlayControlId::left:
            --m_selection.x;
            if (!m_blocks.has_position(m_selection)) {
                m_selection.x = m_blocks.height() - 1;
            }
            break;
        case PlayControlId::right:
            ++m_selection.x;
            if (!m_blocks.has_position(m_selection)) {
                m_selection.x = 0;
            }
            break;
        case PlayControlId::rotate_left: case PlayControlId::rotate_right:
            do_selection();
            break;
        default: break;
        }
    }
}

/* private */ void SameGame::draw(sf::RenderTarget & target, sf::RenderStates states) const {
    draw_fill_with_background(target,
        m_blocks.is_empty() ? m_sweep_temp.height() : m_blocks.width (),
        m_blocks.is_empty() ? m_sweep_temp.width () : m_blocks.height());

    cul::DrawRectangle drect;
    drect.set_size(k_block_size, k_block_size);
    drect.set_position(float(m_selection.x*k_block_size), float(m_selection.y*k_block_size));
    target.draw(drect);

    target.draw(m_pop_ef , states);
    target.draw(m_fall_ef, states);
    if (!m_fall_ef.has_effects() && !m_pop_ef.has_effects()) {
        sf::Sprite brush;
        brush.setTexture(BuiltinBlockGraphics::as_texture());
        render_merged_blocks(m_blocks, brush, target);
    }
}

/* private */ void SameGame::do_selection() {
    if (m_pop_ef.has_effects() || m_fall_ef.has_effects()) return;
    m_pop_ef.do_pop(m_blocks, m_selection, m_pop_singles_enabled);
    if (!m_pop_ef.has_effects()) {
        try_sweep();
    }
}

/* private */ void SameGame::try_sweep() {
    flip_along_trace(m_blocks, m_sweep_temp);
    m_fall_ef.set_vector_transform(FallEffectsFull::flip_xy);
    make_tetris_rows_fall(m_sweep_temp, m_fall_ef);
    m_fall_ef.set_vector_transform(FallEffectsFull::identity_func);
    if (m_fall_ef.has_effects()) {
        m_blocks.clear();
    } else {
        m_sweep_temp.clear();
    }
}

namespace {

template <typename T>
void flip_along_trace(const Grid<T> & source, Grid<T> & dest) {
    dest.clear();
    dest.set_size(source.height(), source.width());
    for (VectorI r; r != source.end_position(); r = source.next(r)) {
        dest(r.y, r.x) = source(r);
    }
}

} // end of <anonymous> namespace
