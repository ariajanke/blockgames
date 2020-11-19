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

#include "PuyoDialogs.hpp"
#include "PuyoScenario.hpp"

#include <common/StringUtil.hpp>

#include <iostream>

#include <cstring>
#include <cassert>

void PuyoDialog::setup_() {
    set_title(U"Puyo Clone (Free Play) Setting");
    set_drag_enabled(false);

    m_pop_req_notice.set_string(U"Pop Requirement");
    m_fall_speed_notice.set_string(U"Fall Speed\n(blocks/sec)");

    static constexpr const int k_min_pop_req = 2;
    static constexpr const int k_max_pop_req = 12;

    m_pop_req_slider.set_options(number_range_to_strings(k_min_pop_req, k_max_pop_req));
    m_pop_req_slider.select_option(settings().puyo.pop_requirement - k_min_pop_req);

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
            std::cout << "set fall speed " << out << " bps" << std::endl;
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

// ----------------------------------------------------------------------------

namespace {


using ConstScenarioCont = std::vector<ConstScenarioPtr>;
using ScenarioStrFunc = const char * (Scenario::*)() const;
using TextSize = ksg::Text::TextSize;

static constexpr const int k_wrap_limit = 80;

template <typename T, typename U>
std::vector<T> move_and_convert(std::vector<U> &&);

const sf::Font & get_global_font(const ksg::StyleMap &);

template <ScenarioStrFunc get_str>
TextSize get_max_dims(const sf::Font &, const ConstScenarioCont &, int char_size);

UString wrap_string(const UString &);

const ConstScenarioCont s_scenarios =
    move_and_convert<ConstScenarioPtr>(Scenario::make_all_scenarios());

} // end of <anonymous> namespace

/* private */ void PuyoScenarioDialog::setup_() {
    set_title(U"Scenario Select");
    set_drag_enabled(false);
#   if 0
    m_title.set_character_size(24.f);
    m_title.set_string(U"Scenario Select");
#   endif
    m_scen_select_notice.set_string(
        U"When you select a free play scanario, it will become the scenario\n"
         "you play when you click \"Free Play\" on the main menu.");

    static const constexpr int k_char_size = 18;
    const sf::Font & font = get_global_font(get_styles());

    m_name_notice.set_string(U"Name:");
    m_name_notice.set_character_size(22);
    auto sz = get_max_dims<&Scenario::name>(font, s_scenarios, k_char_size);
    m_name.set_size(sz.width, sz.height);

    m_desc_notice.set_string(U"Description:");
    m_desc_notice.set_character_size(22);
    sz = get_max_dims<&Scenario::description>(font, s_scenarios, k_char_size);
    m_desc.set_size(sz.width, sz.height);

    for (auto * ta_ptr : { &m_name_notice, &m_name, &m_desc_notice, &m_desc }) {
        ta_ptr->set_character_size(k_char_size);
    }

    m_play.set_string(U"Play");
    m_play.set_press_event([this]() {
        set_next_state(std::make_unique<PuyoState>(get_selected_scenario().clone()));
    });

    {
    std::vector<UString> scens;
    scens.reserve(s_scenarios.size());
    int i = 1;
    for (const auto & uptr : s_scenarios) {
        if (uptr->is_sequential()) {
            scens.emplace_back(U"Level " + to_ustring(std::to_string(i++)));
        } else {
            scens.emplace_back(U"Free Play Scenario");
        }
    }
    m_scenario_slider.set_options(std::move(scens));
    m_scenario_slider.set_option_change_event([this]() { flip_to_scenario(); });
    }

    m_back.set_string(U"Back to Menu");
    m_back.set_press_event([this]() {
        set_next_state(Dialog::make_top_level_dialog(GameSelection::puyo_clone));
    });

    flip_to_scenario();
    begin_adding_widgets(get_styles()).
#       if 0
        add_horizontal_spacer().add(m_title).add_horizontal_spacer().add_line_seperator().
#       endif
        add(m_scen_select_notice).add_line_seperator().
        add(m_name_notice).add_line_seperator().
        /*add_horizontal_spacer().*/add(m_name).add_line_seperator().
        add(m_desc_notice).add_line_seperator().
        /*add_horizontal_spacer().*/add(m_desc).add_line_seperator().
        add(m_scenario_slider).add_line_seperator().
        add(m_back).add_horizontal_spacer().add(m_play).add_horizontal_spacer().add_horizontal_spacer().add_horizontal_spacer();
}

/* private */ void PuyoScenarioDialog::flip_to_scenario() {
    const Scenario & scen = get_selected_scenario();
    assert(strlen(scen.name()) <= k_wrap_limit);
    m_name.set_string(            to_ustring(scen.name       ()) );
    m_desc.set_string(wrap_string(to_ustring(scen.description())));
    if (!scen.is_sequential()) {
        settings().puyo.scenario_number = int(m_scenario_slider.selected_option_index());
    }
}

/* private */ const Scenario & PuyoScenarioDialog::get_selected_scenario() const
    { return *s_scenarios.at(m_scenario_slider.selected_option_index()); }

namespace {

template <typename T, typename U>
std::vector<T> move_and_convert(std::vector<U> && cont) {
    using std::move;
    std::vector<T> rv;
    rv.reserve(cont.size());
    for (auto & scen : cont) rv.emplace_back(move(scen));
    return rv;
}


const sf::Font & get_global_font(const ksg::StyleMap & style_map) {
    static const char * const k_no_font_msg = "lmao where's the font?";
    auto itr = style_map.find(ksg::styles::k_global_font);
    if (itr == style_map.end()) {
        throw std::runtime_error(k_no_font_msg);
    }
    const sf::Font * font = nullptr;
    if (auto * f_ = itr->second.as_pointer<const sf::Font *>()) {
        font = *f_;
    } else if (const auto f_sptr = itr->second.as_pointer<std::shared_ptr<const sf::Font>>()) {
        font = f_sptr->get();
    }
    if (!font) {
        throw std::runtime_error(k_no_font_msg);
    }
    return *font;
}

template <ScenarioStrFunc get_str>
TextSize get_max_dims(const sf::Font & font, const ConstScenarioCont & cont, int char_size) {
    using UChar = UString::value_type;
    TextSize rv;
    UString temp;
    for (const auto & uptr : cont) {
        auto gv = std::invoke(get_str, uptr);
        auto gv_end = gv + strlen(gv);
        temp.clear();
        temp.reserve(gv_end - gv);
        for (auto itr = gv; itr != gv_end; ++itr) temp += UChar(*itr);
        temp = wrap_string(temp);
        auto sz = ksg::Text::measure_text(font, unsigned(char_size), temp);
        rv.width  = std::max(rv.width , sz.width );
        rv.height = std::max(rv.height, sz.height);
    }
    return rv;
}

UString wrap_string(const UString & ustr) {
    using Iter = UString::const_iterator;
    UString t;
    t.reserve(ustr.size() + (ustr.size() % k_wrap_limit) + 3);
    wrap_string_as_monowidth(ustr.begin(), ustr.end(), k_wrap_limit,
        [&t](Iter beg, Iter end)
    {
        t.insert(t.end(), beg, end);
        t += U"\n";
    });
    return t;
}

} // end of <anonymous> namespace
