#include "PuyoDialogs.hpp"

#include <common/StringUtil.hpp>

#include <cassert>

void PuyoDialog::setup_() {
    m_pop_req_notice.set_string(U"Pop Requirement");
    m_fall_speed_notice.set_string(U"Fall Speed");

    static constexpr const int k_min_pop_req = 2;
    static constexpr const int k_max_pop_req = 12;

    m_pop_req_slider.set_options(number_range_to_strings(k_min_pop_req, k_max_pop_req));
    m_pop_req_slider.select_option(settings().puyo.pop_requirement - k_min_pop_req);
    // I should *not* have to set the slider's size
    m_pop_req_slider.set_size(120, 40);
    m_pop_req_slider.set_option_change_event([this]() {
        settings().puyo.pop_requirement = int(m_pop_req_slider.selected_option_index()) + k_min_pop_req;
    });

    m_fall_speed_text.set_width(200.f);
    auto num = to_ustring(std::to_string(settings().puyo.fall_speed));
    m_fall_speed_text.set_string(to_ustring(std::to_string(settings().puyo.fall_speed)));

    m_fall_speed_text.set_character_filter([this](const UString & ustr) {
        double out = 0.;
        if (!string_to_number(ustr, out)) return false;
        auto rv = out > 0. && out < 10.;
        if (rv) {
            settings().puyo.fall_speed = out;
        }
        return true;
    });
    m_fall_speed_text.set_text_change_event([this]() {
        string_to_number(m_fall_speed_text.string(), settings().puyo.fall_speed);
    });

    m_back.set_press_event([this]() {
        set_next_state(Dialog::make_top_level_dialog(GameSelection::puyo_clone));
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
