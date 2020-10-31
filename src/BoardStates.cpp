#include "BoardStates.hpp"
#include "Graphics.hpp"
#include "PuyoScenario.hpp"
#include "DialogState.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <functional>

#include <cassert>

namespace {

using BoardOptions = BoardState::BoardOptions;
using InvArg       = std::invalid_argument;

} // end of <anonymous> namespace

/* protected */ void BoardState::process_event(const sf::Event & event) {
    if (event.type == sf::Event::KeyReleased) {
        if (event.key.code == sf::Keyboard::Escape) {
            set_next_state(std::make_unique<DialogState>());
        }
    }
}

/* private */ void BoardState::setup_(Settings & settings) {
    setup_board(settings);
}

/* private */ void BoardState::set_max_colors(int n) {
    if (n < k_min_colors || n > k_max_colors) {
        throw InvArg("BoardState::set_max_colors: max colors must be in [" +
                     std::to_string(k_min_colors) + " " +
                     std::to_string(k_max_colors) + "].");
    }
    m_max_colors = n;
}

// ----------------------------------------------------------------------------

/* private */ void TetrisState::setup_board(const Settings & settings) {
    const auto & conf = settings.tetris;
    m_blocks.set_size(conf.width, conf.height);

    reset_piece();
    m_fef.setup(conf.width, conf.height, load_builtin_block_texture());
    set_max_colors(conf.colors);
}

/* private */ void TetrisState::update(double et) {
    if (m_fef.has_effects()) {
        m_fef.update(et);
    } else if ((m_fall_time += (et*m_fall_multiplier)) > m_fall_delay) {
        m_fall_time = 0.;
        if (!m_piece.move_down(m_blocks)) {
            m_piece.place(m_blocks);
            reset_piece();
            clear_tetris_rows(m_blocks);
            make_tetris_rows_fall(m_blocks, m_fef);
        }
    }
}

/* private */ void TetrisState::process_event(const sf::Event & event) {
    BoardState::process_event(event);
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Down) {
            m_fall_multiplier = 10.;
        }
    } else if (event.type == sf::Event::KeyReleased) {
        switch (event.key.code) {
        case sf::Keyboard::A:
            m_piece.rotate_left(m_blocks);
            break;
        case sf::Keyboard::S:
            m_piece.rotate_right(m_blocks);
            break;
        case sf::Keyboard::Down:
            m_fall_multiplier = 1.;
            break;
        case sf::Keyboard::Left:
            m_piece.move_left(m_blocks);
            break;
        case sf::Keyboard::Right:
            m_piece.move_right(m_blocks);
            break;
        default: break;
        }
    }
}

/* private */ void TetrisState::draw(sf::RenderTarget & target, sf::RenderStates) const {
    if (m_fef.has_effects()) {
        target.draw(m_fef);
    } else {
        sf::Sprite brush;
        brush.setTexture(load_builtin_block_texture());
        for (int i = 0; i != m_piece.block_count(); ++i) {
            auto loc = m_piece.block_location(i);
            brush.setPosition(float(loc.x*k_block_size), float(loc.y*k_block_size));
            brush.setTextureRect(texture_rect_for(m_piece.block_color(i)));
            brush.setColor(base_color_for_block(m_piece.block_color(i)));
            target.draw(brush);
        }
        render_blocks(m_blocks, brush, target);
    }
}

/* private */ void TetrisState::reset_piece() {
    const auto & tetras = Polyomino::default_tetrominos();
    const auto & piece  = tetras[IntDistri(0, tetras.size() - 1)(m_rng)];
    m_piece = piece;
    m_piece.set_colors( 1 + ( (&piece - &tetras.front()) % k_max_colors ) );
    m_piece.set_location(m_blocks.width() / 2, 0);
}

// ----------------------------------------------------------------------------

namespace {

void flip_along_trace(const Grid<int> &, Grid<int> &);

} // end of <anonymous> namespace

/* private */ void SameGame::setup_board(const Settings & settings) {
    const auto & conf = settings.samegame;
    m_pef.assign_texture(load_builtin_block_texture());
    m_blocks.set_size(conf.width, conf.height);
    m_fef.setup(conf.width, conf.height, load_builtin_block_texture());
    set_max_colors(conf.colors);
    for (auto & block : m_blocks) {
        block = random_color(m_rng);
    }
}

/* private */ void SameGame::update(double et) {
    if (m_pef.has_effects()) {
        m_pef.update(et);
        if (!m_pef.has_effects()) {
            make_blocks_fall(m_blocks, m_fef);
        }
    } else if (m_fef.has_effects()) {
        m_fef.update(et);
        if (!m_fef.has_effects() && !m_blocks.is_empty()) {
            try_sweep();
        } else if (!m_fef.has_effects() && !m_sweep_temp.is_empty()) {
            flip_along_trace(m_sweep_temp, m_blocks);
            m_sweep_temp.clear();
        }
    }
    assert(m_blocks.is_empty() ^ m_sweep_temp.is_empty());
}

/* private */ void SameGame::process_event(const sf::Event & event) {    
    BoardState::process_event(event);
    if (m_blocks.is_empty()) return;
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
    case sf::Event::KeyPressed:
        switch (event.key.code) {
        case sf::Keyboard::Up:
            --m_selection.y;
            if (!m_blocks.has_position(m_selection)) {
                m_selection.y = m_blocks.height() - 1;
            }
            break;
        case sf::Keyboard::Down:
            ++m_selection.y;
            if (!m_blocks.has_position(m_selection)) {
                m_selection.y = 0;
            }
            break;
        case sf::Keyboard::Right:
            ++m_selection.x;
            if (!m_blocks.has_position(m_selection)) {
                m_selection.x = 0;
            }
            break;
        case sf::Keyboard::Left:
            --m_selection.x;
            if (!m_blocks.has_position(m_selection)) {
                m_selection.x = m_blocks.height() - 1;
            }
            break;
        default: break;
        }
    case sf::Event::KeyReleased:
        switch (event.key.code) {
        case sf::Keyboard::Return:
            do_selection();
            break;
        default: break;
        }
    default: break;
    }
}

/* private */ void SameGame::draw(sf::RenderTarget & target, sf::RenderStates states) const {
    DrawRectangle drect;
    drect.set_size(k_block_size, k_block_size);
    drect.set_position(float(m_selection.x*k_block_size), float(m_selection.y*k_block_size));
    target.draw(drect);

    target.draw(m_pef, states);
    target.draw(m_fef, states);
    if (!m_fef.has_effects()) {
        sf::Sprite brush;
        brush.setTexture(load_builtin_block_texture());
        render_merged_blocks(m_blocks, brush, target);
    }
}

/* private */ void SameGame::do_selection() {
    if (m_pef.has_effects() || m_fef.has_effects()) return;
    m_pef.do_pop(m_blocks, m_selection);
    if (!m_pef.has_effects()) {
        try_sweep();
    }
}

/* private */ void SameGame::try_sweep() {
    flip_along_trace(m_blocks, m_sweep_temp);
    make_tetris_rows_fall(m_sweep_temp, m_fef);
    if (m_fef.has_effects()) {
        m_blocks.clear();
    } else {
        m_sweep_temp.clear();
    }
}

namespace {

void flip_along_trace(const Grid<int> & source, Grid<int> & dest) {
    dest.clear();
    dest.set_size(source.height(), source.width());
    for (VectorI r; r != source.end_position(); r = source.next(r)) {
        dest(r.y, r.x) = source(r);
    }
}

} // end of <anonymous> namespace