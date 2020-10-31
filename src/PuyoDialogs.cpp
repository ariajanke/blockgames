#include "PuyoDialogs.hpp"

#include <cassert>

namespace {

using BoardOptions = BoardConfigDialog::BoardOptions;
using Error        = std::runtime_error;

std::vector<UString> num_options_from_range(int min, int max);
#if 0
GameSelection verify_not_count(GameSelection);
#endif
} // end of <anonymous> namespace
#if 0
BoardConfigDialog::BoardConfigDialog(GameSelection sel):
    m_selection(verify_not_count(sel))
{}

BoardConfigDialog::BoardConfigDialog(BoardOptions & opts):
    m_board_options(&opts)
{}
#endif
void BoardConfigDialog::setup() {
#   if 0
    auto styles_ = ksg::styles::construct_system_styles();
    styles_[ksg::styles::k_global_font] = ksg::styles::load_font("font.ttf");
#   endif

    m_board_config_notice.set_text(U"Configure board width, height,\nand maximum number of colors.");

    m_width_label.set_text(U"Width");
    m_height_label.set_text(U"Height");
    m_num_of_colors_label.set_text(U"Max Colors");

    // I must be allowed to have options sliders without setting its size
    {
    auto s = num_options_from_range(k_min_board_size, k_max_board_size);
    m_width_sel.swap_options(s);
    s = num_options_from_range(k_min_board_size, k_max_board_size);
    m_height_sel.swap_options(s);
    s = num_options_from_range(k_min_colors, k_max_colors);
    m_number_of_colors_sel.swap_options(s);
    for (auto * sel : { &m_width_sel, &m_height_sel, &m_number_of_colors_sel }) {
        sel->set_size(120.f, 40.f);
    }
    }
#   if 0
    m_back.set_string(U"Return to Menu");
#   endif
    m_width_sel.select_option(std::size_t(board_options().width - k_min_board_size));
    m_width_sel.set_option_change_event([this]() {
        board_options().width = k_min_board_size + int(m_width_sel.selected_option_index());
    });
    m_height_sel.select_option(std::size_t(board_options().height - k_min_board_size));
    m_height_sel.set_option_change_event([this]() {
        board_options().height = k_min_board_size + int(m_height_sel.selected_option_index());
    });
    m_number_of_colors_sel.select_option(std::size_t(board_options().colors - k_min_colors));
    m_number_of_colors_sel.set_option_change_event([this]() {
        board_options().colors = k_min_colors + int(m_width_sel.selected_option_index());
    });
#   if 0
    m_back.set_press_event([this]() {
        set_next_state(Dialog::make_top_level_dialog());
    });
#   endif

    begin_adding_widgets(/*get_styles()*/).
        add(m_board_config_notice).add_line_seperator().
        add(m_width_label).add_horizontal_spacer().add(m_width_sel).add_line_seperator().
        add(m_height_label).add_horizontal_spacer().add(m_height_sel).add_line_seperator().
        add(m_num_of_colors_label).add_horizontal_spacer().add(m_number_of_colors_sel).add_line_seperator();
#       if 0
        add(m_back);
#       endif
}

void BoardConfigDialog::assign_board_options(BoardOptions & opts)
    { m_board_options = &opts; }

BoardOptions & BoardConfigDialog::board_options() {
    assert(m_board_options);
    return *m_board_options;
#   if 0
    using Game = GameSelection;
    switch (m_selection) {
    case Game::puyo_clone    : return settings().puyo;
    case Game::samegame_clone: return settings().samegame;
    case Game::tetris_clone  : return settings().tetris;
    default: throw Error("BoardConfigDialog::board_options: selection not a game.");

    }
#   endif
}

void PuyoDialog::setup_() {
    m_pop_req_notice.set_text(U"Pop Requirement");
    m_fall_speed_notice.set_text(U"Fall Speed");

    auto opts = num_options_from_range(2, 12);
    m_pop_req_slider.swap_options(opts);
    // I should *not* have to set the slider's size
    m_pop_req_slider.set_size(120, 40);

    m_fall_speed_text.set_width(200.f);

    m_back.set_press_event([this]() {
        set_next_state(Dialog::make_top_level_dialog());
    });
    m_back.set_string(U"Back to menu");

    m_board_config.assign_board_options(settings().puyo);
    m_board_config.setup();

    begin_adding_widgets(get_styles()).
        add(m_pop_req_notice).add_horizontal_spacer().add(m_pop_req_slider).add_line_seperator().
        add(m_fall_speed_notice).add_horizontal_spacer().add(m_fall_speed_text).add_line_seperator().
        add(m_board_config).add_line_seperator().
        add(m_back);
    m_board_config.set_padding(0.f);
}

namespace {

std::vector<UString> num_options_from_range(int min, int max) {
    assert(min <= max);
    std::vector<UString> rv;
    for (int i = min; i != max + 1; ++i) {
        rv.push_back(to_ustring(std::to_string(i)));
    }
    return rv;
}
#if 0
GameSelection verify_not_count(GameSelection sel) {
    if (sel == GameSelection::count) {
        throw std::invalid_argument("Selection is not a game.");
    }
    return sel;
}
#endif
} // end of <anonymous> namespace
