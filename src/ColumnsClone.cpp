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

#include "ColumnsClone.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

#include <cassert>

namespace {

template <typename Cont>
int wrap_index(const Cont &, int);

} // end of <anonymous> namespace

ColumnsPiece::ColumnsPiece(BlockId bottom, BlockId mid, BlockId top) {
    for (auto b : { bottom, mid, top }) {
        if (is_block_color(b)) continue;
        throw std::invalid_argument("ColumnsPiece::ColumnsPiece: all blocks must be colors.");
    }
    m_blocks[k_top   ] = top;
    m_blocks[k_middle] = mid;
    m_blocks[k_bottom] = bottom;
    check_invarients();
}

void ColumnsPiece::rotate_down()
    { rotate(k_top - k_bottom); }

void ColumnsPiece::rotate_up()
    { rotate(k_bottom - k_top); }

bool ColumnsPiece::descend(const BlockGrid & grid)
    { return move(grid, VectorI(0, 1)); }

void ColumnsPiece::move_left(const BlockGrid & grid)
    { (void)move(grid, VectorI(-1, 0)); }

void ColumnsPiece::move_right(const BlockGrid & grid)
    { (void)move(grid, VectorI(1, 0)); }

void ColumnsPiece::place(BlockGrid & grid) const {
    auto my_blocks = as_blocks();
    for (auto [pos, id] : my_blocks) {
        // this would prevent piece placement at the top if it won't fit
        if (!grid.has_position(pos)) continue;
        grid(pos) = id;
    }
}

void ColumnsPiece::set_column_position(int col) {
    if (col < 0) {
        throw std::invalid_argument(
            "ColumnsPiece::set_column_position: negative columns are rejected, "
            "because no grid may have negative columns.");
    }
    m_bottom = VectorI(col, -1);
    check_invarients();
}

ColumnsPiece::BlocksArray ColumnsPiece::as_blocks() const {
    using std::make_pair;
    return {
        make_pair(m_bottom                       , m_blocks[k_bottom]),
        make_pair(m_bottom + offset_for(k_middle), m_blocks[k_middle]),
        make_pair(m_bottom + offset_for(k_top   ), m_blocks[k_top   ])
    };
}

VectorI ColumnsPiece::bottom() const
    { return m_bottom; }

/* private static */ constexpr const decltype(ColumnsPiece::k_positions) ColumnsPiece::k_positions;

/* private static */ VectorI ColumnsPiece::offset_for
    (decltype(k_top) piece_pos)
{
    switch (piece_pos) {
    case k_top   : return VectorI(0, -2);
    case k_middle: return VectorI(0, -1);
    case k_bottom: return VectorI(0,  0);
    }
    throw std::runtime_error("ColumnsPiece::offset_for: impossible branch");
}

/* private */ void ColumnsPiece::rotate(int direction) {
    if (direction == 0) return;
    direction /= magnitude(direction);
    decltype(m_blocks) t;
    for (int i = 0; i != k_piece_size; ++i) {
        t[wrap_index(t, i - direction)] = m_blocks[i];
    }
    m_blocks = t;

    check_invarients();
}

/* private */ bool ColumnsPiece::move(const BlockGrid & grid, VectorI offset) {
    int old_inside_of_grid = 0;
    int new_inside_of_grid = 0;
    for (auto pos : k_positions) {
        auto block_position = m_bottom + offset_for(pos);
        auto block_new_position = offset + block_position;
        if (grid.has_position(block_position    )) ++old_inside_of_grid;
        if (grid.has_position(block_new_position)) ++new_inside_of_grid;
    }
    if (new_inside_of_grid < old_inside_of_grid) return false;

    // reject if new position is an occupied block
    for (auto pos : k_positions) {
        auto block_new_position = offset + m_bottom + offset_for(pos);
        if (!grid.has_position(block_new_position)) continue;
        if (grid(block_new_position) != k_empty_block) return false;
    }

    m_bottom += offset;
    check_invarients();
    return true;
}

/* private */ void ColumnsPiece::check_invarients() const {
    for (auto i : m_blocks) {
        assert(is_block_color(i) || i == k_empty_block);
    }
    assert(m_bottom.x >=  0);
    assert(m_bottom.y >= -1);
}

// ----------------------------------------------------------------------------

/* private */ void ColumnsState::setup_board(const Settings & settings) {
    (void)settings;
    const Settings::Puyo conf;
    m_blocks.set_size(conf.width, conf.height);
    m_fall_ef.setup(conf.width, conf.height, load_builtin_block_texture());
    m_fall_ef.set_render_blocks_merged_enabled(false);
    set_max_colors(conf.colors);
    m_falling_piece = ColumnsPiece(random_color(m_rng), random_color(m_rng), random_color(m_rng));
    m_falling_piece.set_column_position(m_blocks.width() / 2);
}

/* private */ int ColumnsState::width_in_blocks() const
    { return m_blocks.width(); }

/* private */ int ColumnsState::height_in_blocks() const
    { return m_blocks.height(); }

/* private */ void ColumnsState::update(double et) {
    PauseableWithFallingPieceState::update(et);
    if (is_paused()) return;
    if (m_fall_ef.has_effects()) {
        m_fall_ef.update(et);
        if (!m_fall_ef.has_effects()) {
            if (pop_columns_blocks(m_blocks, 3)) {
                make_blocks_fall(m_blocks, m_fall_ef);
            }
        }
    } else if ((m_fall_offset += et*m_fall_rate*fall_multiplier()) > 1.) {
        if (!m_falling_piece.descend(m_blocks)) {
            m_falling_piece.place(m_blocks);
            m_falling_piece = ColumnsPiece(random_color(m_rng), random_color(m_rng), random_color(m_rng));
            m_falling_piece.set_column_position(m_blocks.width() / 2);
            if (m_blocks(m_blocks.width() / 2, 0) == k_empty_block) {
                if (pop_columns_blocks(m_blocks, 3)) {
                    make_blocks_fall(m_blocks, m_fall_ef);
                }
            } else {
                make_all_blocks_fall_out(m_blocks, m_fall_ef);
            }
        }
        m_fall_offset = 0.;
    }
    check_invarients();
}
#if 0
/* private */ void ColumnsState::process_event(const sf::Event & event) {
    BoardState::process_event(event);
    switch (event.type) {
    case sf::Event::KeyPressed:
        switch (event.key.code) {
        case sf::Keyboard::Down:
            m_fall_multiplier = k_fast_fall;
            break;
        default: break;
        }
        break;
    case sf::Event::KeyReleased:
        switch (event.key.code) {
        case sf::Keyboard::Return:
            m_paused = !m_paused;
            break;
        case sf::Keyboard::Left:
            m_falling_piece.move_left(m_blocks);
            break;
        case sf::Keyboard::Right:
            m_falling_piece.move_right(m_blocks);
            break;
        case sf::Keyboard::Down:
            m_fall_multiplier = k_slow_fall;
            break;
        case sf::Keyboard::A:
            m_falling_piece.rotate_down();
            break;
        case sf::Keyboard::S:
            m_falling_piece.rotate_up();
            break;
        default: break;
        }
    default: break;
    }
}

/* private */ void ColumnsState::handle_event(PlayControlEvent event) {

}
#endif
/* private */ void ColumnsState::draw
    (sf::RenderTarget & target, sf::RenderStates) const
{
    BoardState::draw_fill_with_background(target, m_blocks.width(), m_blocks.height());
    if (m_fall_ef.has_effects()) {
        target.draw(m_fall_ef);
        return;
    }
    auto y_offset = [this]() {
        auto one_below = m_falling_piece.bottom() + VectorI(0, 1);
        if (!m_blocks.has_position(one_below)) return 0.;
        if (m_blocks(one_below) != k_empty_block) return 0.;
        return m_fall_offset*k_block_size;
    } ();
    sf::Sprite brush;
    brush.setTexture(load_builtin_block_texture());
    for (auto [pos, id] : m_falling_piece.as_blocks()) {
        brush.setPosition(float(pos.x*k_block_size),float(pos.y*k_block_size + y_offset));
        brush.setColor(base_color_for_block(id));
        brush.setTextureRect(texture_rect_for(id));
        target.draw(brush);
    }
    brush.setPosition(0.f, 0.f);
    render_blocks(m_blocks, brush, target);
}

/* private */ void ColumnsState::check_invarients() const {
    assert(m_fall_offset >= 0. && m_fall_offset <= 1.);
}

// ----------------------------------------------------------------------------

/* private */ void ColumnsSettingsDialog::setup_() {
    m_back_to_main.set_string(U"Back to Menu");
    m_unimplemented.set_string(U"This is still unimplemented.\n"
                               "For now this game uses settings from the Puyo derivative.");
    m_back_to_main.set_press_event([this]() {
        set_next_state(make_top_level_dialog(GameSelection::columns_clone));
    });
    begin_adding_widgets(get_styles())
        .add(m_unimplemented).add_line_seperator()
        .add(m_back_to_main);
}

namespace {

template <typename Cont>
int wrap_index(const Cont & cont, int i) {
    auto size = std::end(cont) - std::begin(cont);
    if (i <     0) return size - 1;
    if (i >= size) return 0;
    return i;
}

} // end of <anonymous> namespace
