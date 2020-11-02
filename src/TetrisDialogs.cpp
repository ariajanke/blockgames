#include "TetrisDialogs.hpp"
#include "Graphics.hpp"
#include "DialogState.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

#include <cassert>

namespace {

using PolyominoItr = PolyominoDialogPage::PolyominoItr;

} // end of <anonymous> namespace

void PolyominoButton::set_polyomino(const Polyomino & poly) {
    assert(poly.block_count() > 1);
    m_polyomino = poly;
    static constexpr const int k_ext_low  = std::numeric_limits<int>::min();
    static constexpr const int k_ext_high = std::numeric_limits<int>::max();
    VectorI low(k_ext_high, k_ext_high);
    VectorI high(k_ext_low, k_ext_low);
    for (int i = 0; i != poly.block_count(); ++i) {
        auto r = poly.block_location(i);
        low.x  = std::min(low .x, r.x);
        low.y  = std::min(low .y, r.y);
        high.x = std::max(high.x, r.x);
        high.y = std::max(high.y, r.y);
    }
    auto block_height = (high.y - low.y) + 1;
    auto block_width  = (high.x - low.x) + 1;
    m_scale = (block_height > k_use_small_thershold ||
               block_width  > k_use_small_thershold) ? k_small_scale : k_big_scale;
    m_polyomino_offset.x = float(((low.x + high.x) / 2)*k_block_size);
    m_polyomino_offset.y = float(((low.y + high.y) / 2)*k_block_size);
}

void PolyominoButton::draw(sf::RenderTarget & target, sf::RenderStates states) const {
    sf::Sprite brush;
    auto start_offset = m_polyomino_offset + location()
        + VectorF(1.f, 1.f)*padding()
        + VectorF(1.f, 1.f)*float(k_pixels_for_blocks)*0.5f
        - VectorF(1.f, 1.f)*float(k_block_size)*0.5f;
    brush.setTexture(load_builtin_block_texture());
    brush.setTextureRect(texture_rect_for(1));
    brush.setColor(m_on ? sf::Color::White : sf::Color(200, 200, 200));
    brush.scale(float(m_scale), float(m_scale));
    sf::FloatRect bounds(location(), sf::Vector2f(width(), height()));
    for (int i = 0; i != m_polyomino.block_count(); ++i) {
        auto loc = start_offset + VectorF(m_polyomino.block_location(i)*k_block_size*m_scale);
        brush.setPosition(loc);
        target.draw(brush, states);
    }
}

void PolyominoButton::issue_auto_resize() {
    set_size(padding()*2.f + k_pixels_for_blocks,
             padding()*2.f + k_pixels_for_blocks);
    set_button_frame_size(k_pixels_for_blocks, k_pixels_for_blocks);
}

// ----------------------------------------------------------------------------

PolyominoSetSelectPage::PolyominoSetSelectPage(BoardOptions & board_opts) {
    m_board_config.assign_board_options(board_opts);
}

PolyominoItr PolyominoSetSelectPage::set
    (PolyominoItr cont_beg, PolyominoItr, PolyominoItr,
     EnabledPolyominoBits & enabledbits)
{
    m_enabled_polyominos = &enabledbits;
    setup();
    return cont_beg;
}

void PolyominoSetSelectPage::update_selections() {
    std::size_t idx = 0;
    bool all_off = true;
    for (auto set : k_sets) {
        m_set_notices[idx++].set_string(display_string_for_set(set));
        auto range = range_for_set(set);
        for (auto i = range.first; i != range.second; ++i) {
            if (m_enabled_polyominos->test(i)) all_off = false;
        }
    }
    m_all_off_notice.set_visible(all_off);
}

/* private */ void PolyominoSetSelectPage::setup() {
    m_set_notices.resize(static_cast<std::size_t>(PolyominoSet::count));
    m_set_endis_buttons.resize(m_set_notices.size());

    std::size_t idx = 0;
    auto adder = begin_adding_widgets();
    m_board_config.setup();
    m_poly_set_nfo.set_string(U"Enable/Disable sets of polyominos at a time.");
    adder.add(m_board_config).add_line_seperator().add(m_poly_set_nfo).add_line_seperator();

    for (auto set : k_sets) {
        m_set_endis_buttons[idx].set_string(UString(U"Toggle ") + to_ustring(to_string(set)) + U"s");
        m_set_endis_buttons[idx].set_press_event([this, set] () {
            bool new_value = amount_set_for(set) == k_none;
            auto range = range_for_set(set);
            for (auto i = range.first; i != range.second; ++i)
                m_enabled_polyominos->set(i, new_value);
            update_selections();
        });
        m_set_notices[idx].set_string(display_string_for_set(set));
        adder.add(m_set_endis_buttons[idx]).add(m_set_notices[idx]).add_line_seperator();
        ++idx;
        assert(idx <= m_set_notices.size());
    }

    m_all_off_notice.set_string(U"Note: if all polyominos are set to \"off\", then the program will force dominos to spawn during gameplay.");
    m_all_off_notice.set_width(500.f);
    adder.add(m_all_off_notice);
    update_selections();
}

/* private */ std::pair<std::size_t ,std::size_t> PolyominoSetSelectPage::
    range_for_set(PolyominoSet set) const
{
    using Ps = PolyominoSet;
    using std::make_pair;
    std::size_t start = 0;
    switch (set) {
    case Ps::domino   :
        return make_pair(0, Polyomino::k_domino_count);
    case Ps::tromino  :
        start = Polyomino::k_domino_count;
        return make_pair(start, start + Polyomino::k_tromino_count);
    case Ps::tetromino:
        start = Polyomino::k_domino_count + Polyomino::k_tromino_count;
        return make_pair(start, start + Polyomino::k_tetromino_count);
    case Ps::pentomino:
        start = Polyomino::k_domino_count + Polyomino::k_tromino_count + Polyomino::k_tetromino_count;
        return make_pair(start, start + Polyomino::k_pentomino_count);
    default: throw std::runtime_error("");
    }
}

/* private */ PolyominoSetSelectPage::AmtSet PolyominoSetSelectPage::amount_set_for
    (PolyominoSet set) const
{
    auto [start, end] = range_for_set(set);
    assert(m_enabled_polyominos);
    std::size_t numset = 0;
    for (auto i = start; i != end; ++i) {
        if (m_enabled_polyominos->test(i)) ++numset;
    }
    if (numset == (end - start)) {
        return k_all;
    } else if (numset) {
        return k_some;
    } else {
        return k_none;
    }
}

/* private */ const UString & PolyominoSetSelectPage::display_string_for_set
    (PolyominoSet set) const
{
    static const UString k_all_set  = U"All set" ;
    static const UString k_some_set = U"Some set";
    static const UString k_none_set = U"None set";
    switch (amount_set_for(set)) {
    case k_all : return k_all_set ;
    case k_some: return k_some_set;
    case k_none: return k_none_set;
    default: throw std::runtime_error("");
    }
}

// ----------------------------------------------------------------------------

PolyominoItr PolyominoIndividualSelectPage::set
    (PolyominoItr cont_beg, PolyominoItr beg, PolyominoItr end,
     EnabledPolyominoBits & enabledbits)
{
    assert(cont_beg <= beg);
    assert(beg <= end);
    auto rv = beg + std::min(int(end - beg), k_max_polyominos_per_page);
    m_start_index = beg - cont_beg;
    m_enabled_polyominos = &enabledbits;

    m_buttons        .resize(rv - beg);
    m_poly_enabled_ta.resize(rv - beg);

    m_activate_polyonmino_notice.set_string(
        U"You can turn off or on specific polyonimos here.\n"
         "Polyonimos set off will not appear in free play.");

    auto adder = begin_adding_widgets();
    adder.add(m_activate_polyonmino_notice).add_line_seperator();

    auto ta_itr = m_poly_enabled_ta.begin();
    auto add_row_of_tas = [&adder, &ta_itr, this](int sofar) {
        adder.add_horizontal_spacer().add_line_seperator();
        while (sofar--) {
            assert(ta_itr != m_poly_enabled_ta.end());
            adder.add_horizontal_spacer().add(*ta_itr++);
        }
        adder.add_horizontal_spacer().add_line_seperator();
    };
    int sofar = 0;
    for (auto itr = beg; itr != rv; ++itr) {
        std::size_t idx = itr - beg;
        auto & button = m_buttons[idx];
        button.set_polyomino(*itr);
        button.set_press_event([this, idx]() {
            auto idx_ = m_start_index + idx;
            auto new_val = !m_enabled_polyominos->test(idx_);
            m_enabled_polyominos->set(idx_, new_val);
            m_poly_enabled_ta[idx].set_string(new_val ? k_on_string : k_off_string);
            if (new_val) m_buttons[idx].set_on();
            else m_buttons[idx].set_off();
        });
        adder.add_horizontal_spacer().add(button);
        if (++sofar == k_max_polyominos_per_row) {
            add_row_of_tas(sofar);
            sofar = 0;
        }
    }

    add_row_of_tas(sofar);
    for (auto & ta : m_poly_enabled_ta) {
        ta.set_width(100);
    }

    update_selections();
    check_invarients();
    return rv;
}

void PolyominoIndividualSelectPage::update_selections() {
    for (std::size_t i = 0; i != m_buttons.size(); ++i) {
        if (m_enabled_polyominos->test(i + m_start_index)) {
            m_buttons[i].set_on();
            m_poly_enabled_ta[i].set_string(k_on_string);
        } else {
            m_buttons[i].set_off();
            m_poly_enabled_ta[i].set_string(k_off_string);
        }
    }
    check_invarients();
}

/* private */ void PolyominoIndividualSelectPage::check_invarients() const {
    assert(m_buttons.size() == m_poly_enabled_ta.size());
}

// ----------------------------------------------------------------------------

/* private */ void PolyominoSelectDialog::setup_() {
    const auto & all_p = Polyomino::all_polyminos();
    m_pages.emplace_back(std::make_unique<PolyominoSetSelectPage>(settings().tetris));
    for (auto itr = all_p.begin(); true;) {
        itr = m_pages.back()->set
                (all_p.begin(), itr, all_p.end(), settings().tetris.enabled_polyominos);
        if (itr == all_p.end()) break;
        m_pages.emplace_back(std::make_unique<PolyominoIndividualSelectPage>());
    }
    for (auto & page_ptr : m_pages) {
        page_ptr->set_size(500, 500);
    }

    m_back_to_menu.set_string(U"Back to menu");
    {
    std::vector<UString> opts;
    for (std::size_t i = 0; i != m_pages.size(); ++i) {
        opts.push_back(U"Page " + to_ustring(std::to_string(i + 1)));
    }
    m_page_slider.swap_options(opts);
    }

    m_page_slider.set_option_change_event([this]() {
        auto & page = *m_pages.at(m_page_slider.selected_option_index());
        page.update_selections();
        flip_to_page(page);

    });
    m_back_to_menu.set_press_event([this]() {
        set_next_state(Dialog::make_top_level_dialog(GameSelection::tetris_clone));
    });
    flip_to_page(*m_pages.front());
}

/* private */ void PolyominoSelectDialog::flip_to_page(PolyominoDialogPage & page) {
    begin_adding_widgets(get_styles()).
        add(page).add_line_seperator().
        add(m_page_slider).add_line_seperator().
        add(m_back_to_menu);
}
