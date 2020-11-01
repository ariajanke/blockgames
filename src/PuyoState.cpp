#include "PuyoState.hpp"
#include "PuyoScenario.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <cassert>

namespace {

void all_blocks_fall_out(Grid<int> &, FallBlockEffects &);
#if 0
void all_blocks_in_column_fall_out(Grid<int> &, FallBlockEffects &, int col);
#endif
} // end of <anonymous> namespace

void ScenarioPriv::DefDeleter::operator () (Scenario * ptr) const { delete ptr; }

void PuyoState::setup_board(const Settings & settings) {
    m_pef.assign_texture(load_builtin_block_texture());

    m_current_scenario = Scenario::make_glass_waves();
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
    }
    m_blocks.set_size(p.board_width, p.board_height);
    m_fef.setup(p.board_width, p.board_height, load_builtin_block_texture());
    set_max_colors(p.max_colors);
    handle_response(m_current_scenario->on_turn_change());
    handle_response(m_current_scenario->on_turn_change());
}

void PuyoState::update(double et) {
    if (m_is_paused) return;
    assert(!m_blocks.is_empty());
    std::invoke(m_update_func, *this, et);
}

void PuyoState::update_piece(double et) {
    assert(is_block_color(m_piece.color()) && is_block_color(m_piece.other_color()));
    if ((m_fall_time += et*m_fall_multi) <= k_fall_delay) return;

    m_fall_time = 0.;
    VectorI spawn_point(m_blocks.width() / 2 - 1, 0);
    if (!m_piece.descend(m_blocks)) {
        if (m_blocks(spawn_point + VectorI(0, 1)) != k_empty_block) {
            // on loss
            all_blocks_fall_out(m_blocks, m_fef);
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

        m_piece.set_location(VectorI(m_blocks.width() / 2 - 1, 0));
        m_update_func = &PuyoState::update_piece;
    } else if (auto * fallins = response.as_pointer<Grid<int>>()) {
        m_fef.do_fall_in(m_blocks, *fallins);
        m_update_func = &PuyoState::update_fall_effects;
    }
}

void PuyoState::process_event(const sf::Event & event) {
    BoardState::process_event(event);
    if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
        case sf::Keyboard::Down:
            m_fall_multi = 5.;
            break;
        default: break;
        }
    } else if (event.type == sf::Event::KeyReleased) {
        switch (event.key.code) {
        case sf::Keyboard::Up:
            break;
        case sf::Keyboard::Down:
            m_fall_multi = 1.;
            break;
        case sf::Keyboard::Right:
            m_piece.move_right(m_blocks);
            break;
        case sf::Keyboard::Left:
            m_piece.move_left(m_blocks);
            break;
        case sf::Keyboard::A:
            m_piece.rotate_left(m_blocks);
            break;
        case sf::Keyboard::S:
            m_piece.rotate_right(m_blocks);
            break;
        case sf::Keyboard::Return:
            m_is_paused = !m_is_paused;
            break;
        default: break;
        }
    }
}

/* private */ void PuyoState::draw
    (sf::RenderTarget & target, sf::RenderStates) const
{
    {
    sf::Sprite brush;
    brush.setTexture(load_builtin_block_texture());
    brush.setTextureRect(texture_rect_for_background());
    for (VectorI r; r != m_blocks.end_position(); r = m_blocks.next(r)) {
        brush.setPosition(float(k_block_size*r.x),
                          float(k_block_size*r.y));
        target.draw(brush);
    }
    }

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
        y_offset = int(std::round((m_fall_time / k_fall_delay)*double(k_block_size)));
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

    draw_block(m_next_piece.first , VectorI(m_blocks.width(), 1)*k_block_size);
    draw_block(m_next_piece.second, VectorI(m_blocks.width(), 0)*k_block_size);
}

namespace {

void all_blocks_fall_out(Grid<int> & blocks, FallBlockEffects & effects) {
    effects.start();
    for (VectorI r; r != blocks.end_position(); r = blocks.next(r)) {
        if (blocks(r) == k_empty_block) continue;
        effects.post_block_fall(r, VectorI(r.x, blocks.height()), blocks(r));
        blocks(r) = k_empty_block;
    }
    effects.finish();
}
#if 0
void all_blocks_in_column_fall_out
    (Grid<int> & blocks, FallBlockEffects & effects, int col)
{
    effects.start();
    for (VectorI r; r != blocks.end_position(); r = blocks.next(r)) {
        if (r.x != col) effects.post_stationary_block(r, blocks(r));
    }
    for (int y = 0; y != blocks.height(); ++y) {
        if (blocks(col, y) == k_empty_block) continue;
        effects.post_block_fall(
            VectorI(col, y), VectorI(col, blocks.height()), blocks(col, y));
        blocks(col, y) = k_empty_block;
    }
    effects.finish();
}
#endif
} // end of <anonymous> namespace
