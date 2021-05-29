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

/* private */ void PuyoSettingsDialog::setup_() {
    set_title(U"Puyo Clone (Free Play) Setting");
    set_drag_enabled(false);

    m_pop_req_notice.set_string(U"Pop Requirement");
    m_fall_speed_notice.set_string(U"Fall Speed\n(blocks/sec)");

    static constexpr const int k_min_pop_req = 2;
    static constexpr const int k_max_pop_req = 12;

#   if 0
    m_pop_req_slider.select_option(puyo_settings().pop_requirement - k_min_pop_req);

    m_pop_req_slider.set_option_change_event([this]() {
        puyo_settings().pop_requirement = int(m_pop_req_slider.selected_option_index()) + k_min_pop_req;
    });
#   endif
    if (auto * pop_req = puyo_settings().pop_requirement_ptr()) {
        m_pop_req_slider.set_options(number_range_to_strings(k_min_pop_req, k_max_pop_req));
        m_pop_req_slider.select_option(*pop_req - k_min_pop_req);

        m_pop_req_slider.set_option_change_event([this, pop_req]() {
            *pop_req = int(m_pop_req_slider.selected_option_index()) + k_min_pop_req;
        });
    }
    if (auto * fall_speed = puyo_settings().fall_speed_ptr()) {
        m_fall_speed_text.set_text_width(200);
        auto num = to_ustring(std::to_string(*fall_speed));
        m_fall_speed_text.set_entered_string(to_ustring(std::to_string(*fall_speed)));

        m_fall_speed_text.set_check_string_event(
            [fall_speed](const UString & new_string, UString & display_string)
        {
            static constexpr const double k_min_change = 0.05;
            double out = 0.;
            if (!cul::string_to_number(new_string.begin(), new_string.end(), out)) {
                return false;
            }
            if (cul::magnitude(out - *fall_speed) < k_min_change) return false;
            if (out < 0. || out > 10.) return false;
            display_string = new_string;
            *fall_speed = out;
            return true;
        });
    }
#   if 0
    if (auto * fall_speed = puyo_settings().fall_speed_ptr()) {
        m_fall_speed_text.set_width(200.f);
        auto num = to_ustring(std::to_string(*fall_speed));
        m_fall_speed_text.set_string(to_ustring(std::to_string(*fall_speed)));

        m_fall_speed_text.set_character_filter([fall_speed](const UString & ustr) {
            double out = 0.;
            if (!string_to_number(ustr, out)) return false;
            if (out > 0. && out < 10.) {
                *fall_speed = out;
                std::cout << "set fall speed " << out << " bps" << std::endl;
                return true;
            }
            return false;
        });
        m_fall_speed_text.set_text_change_event([this, fall_speed]() {
            string_to_number(m_fall_speed_text.string(), *fall_speed);
        });
    }
#   endif
    m_back.set_press_event([this]() {
        set_next_state(Dialog::make_top_level_dialog(GameSelection::puyo_clone));
    });
    m_back.set_string(U"Back to menu");

#   if 0
    m_board_config.assign_board_options(puyo_settings());
#   endif
    if (puyo_settings().height_ptr()) {
        m_board_config.assign_size_pointers(puyo_settings().width_ptr(), puyo_settings().height_ptr());
    }
    m_board_config.assign_number_of_colors_pointer(puyo_settings().color_count_ptr());

    auto adder = begin_adding_widgets(/*get_styles()*/);
    if (puyo_settings().pop_requirement_ptr()) {
        adder.add(m_pop_req_notice).add_horizontal_spacer().add(m_pop_req_slider).add_line_seperator();
    }
    if (puyo_settings().fall_speed_ptr()) {
        adder.add(m_fall_speed_notice).add_horizontal_spacer().add(m_fall_speed_text).add_line_seperator();
    }
    if (!m_board_config.will_be_blank()) {
        m_board_config.setup();
        adder.add(m_board_config).add_line_seperator();
    }
    adder.add(m_back);
    m_board_config.set_padding(0.f);
}

/* private */ PuyoSettingsDialog::PuyoSettings PuyoSettingsDialog::puyo_settings() {
#   if 0
    if (m_scenario_index < 0 || m_scenario_index > settings().puyo_scenario_count()) {
        throw std::runtime_error("PuyoSettingsDialog::puyo_settings: cannot access scenario's settings (invalid index)");
    }
    return settings().get_puyo_scenario(m_scenario_index);
#   endif
    return settings().get_puyo_settings(m_name_ptr);
}

// ----------------------------------------------------------------------------

namespace {


using ConstScenarioCont = std::vector<ConstScenarioPtr>;
using ScenarioStrFunc = const char * (Scenario::*)() const;
using TextSize = asgl::Size;// ksg::Text::TextSize;

static constexpr const int k_wrap_limit = 80;

template <typename T, typename U>
std::vector<T> move_and_convert(std::vector<U> &&);
#if 0
const sf::Font & get_global_font(const ksg::StyleMap &);

template <ScenarioStrFunc get_str>
TextSize get_max_dims(const sf::Font &, const ConstScenarioCont &, int char_size);
#endif
UString wrap_string(const UString &);
#if 0
const ConstScenarioCont s_scenarios =
    move_and_convert<ConstScenarioPtr>(Scenario::make_all_scenarios());
#endif

template <ScenarioStrFunc get_str>
MaxTextSizer net_up_string_maximums(TextArea & target);

void set_to_maximum(MaxTextSizer &);

void revert_to_front(MaxTextSizer &);

} // end of <anonymous> namespace

/* private */ void PuyoScenarioDialog::setup_() {
    const auto & s_scenarios = Scenario::get_all_scenarios();
    set_title(U"Scenario Select");
    set_drag_enabled(false);
    m_scen_select_notice.set_string(
        U"When you select a free play scanario, it will become the scenario\n"
         "you play when you click \"Free Play\" on the main menu.");
#   if 0
    static const constexpr int k_char_size = 18;
    const sf::Font & font = get_global_font(get_styles());
#   endif
    m_name_notice.set_string(U"Name:");
    m_desc_notice.set_string(U"Description:");
    m_desc_notice.set_limiting_line(400);
    m_max_sizers.emplace_back(net_up_string_maximums<&Scenario::name>(m_name));
    m_max_sizers.emplace_back(net_up_string_maximums<&Scenario::description>(m_desc));
#   if 0
    m_name_notice.set_character_size(24);
    auto sz = get_max_dims<&Scenario::name>(font, s_scenarios, k_char_size);
    m_name.set_size(sz.width, sz.height);

    m_desc_notice.set_string(U"Description:");
    m_desc_notice.set_character_size(24);
    sz = get_max_dims<&Scenario::description>(font, s_scenarios, k_char_size);
    m_desc.set_size(sz.width, sz.height);

    for (auto * ta_ptr : { &m_name_notice, &m_name, &m_desc_notice, &m_desc }) {
        ta_ptr->set_character_size(k_char_size);
    }
#   endif
    m_play.set_string(U"Play");
    m_play.set_press_event([this]() {
        auto scen_clone = get_selected_scenario().clone();
        if (!scen_clone->is_sequential()) {
            settings().default_puyo_freeplay_scenario = int(m_scenario_slider.selected_option_index());
        }
        set_next_state(std::make_unique<PuyoStateN>(std::move(scen_clone)));
    });

    m_settings.set_string(U"Settings");
    m_settings.set_press_event([this]() {
        if (get_selected_scenario().is_sequential()) {
            throw std::runtime_error("Cannot set settings for a sequential scenario. Settings are only for free play.");
        }
        set_next_state(make_dialog<PuyoSettingsDialog>(Scenario::get_all_scenarios()[m_scenario_slider.selected_option_index()]->name()));
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
    begin_adding_widgets(/*get_styles()*/).
        add(m_scen_select_notice).add_line_seperator().
        add(m_name_notice).add_line_seperator().
        add(m_name).add_line_seperator().
        add(m_desc_notice).add_line_seperator().
        add(m_desc).add_line_seperator().
        add(m_scenario_slider).add_line_seperator().
        add(m_back).add_horizontal_spacer().add(m_play)
            .add_horizontal_spacer().add_horizontal_spacer().add_horizontal_spacer()
            .add(m_settings);
}

/* private */ void PuyoScenarioDialog::stylize(const asgl::StyleMap & map) {
    Dialog::stylize(map);
    // choose and keep maximum strings
    std::for_each(m_max_sizers.begin(), m_max_sizers.end(), set_to_maximum);
}

/* private */ void PuyoScenarioDialog::process_event(const asgl::Event & event) {
    Dialog::process_event(event);
    if (!m_max_sizers.empty()) {
        // revert to first string
        std::for_each(m_max_sizers.begin(), m_max_sizers.end(), revert_to_front);
        m_max_sizers.clear();
    }
}

/* private */ void PuyoScenarioDialog::flip_to_scenario() {
    const Scenario & scen = get_selected_scenario();
    assert(strlen(scen.name()) <= k_wrap_limit);
    m_name.set_string(            to_ustring(scen.name       ()) );
    m_desc.set_string(wrap_string(to_ustring(scen.description())));

    m_settings.set_visible(!scen.is_sequential());
#   if 0
    if (!scen.is_sequential()) {
        //settings().puyo.scenario_number = int(m_scenario_slider.selected_option_index());

    }
#   endif
}

/* private */ const Scenario & PuyoScenarioDialog::get_selected_scenario() const
    { return *Scenario::get_all_scenarios().at(m_scenario_slider.selected_option_index()); }

namespace {
#if 0
template <typename T, typename U>
std::vector<T> move_and_convert(std::vector<U> && cont) {
    using std::move;
    std::vector<T> rv;
    rv.reserve(cont.size());
    for (auto & scen : cont) rv.emplace_back(move(scen));
    return rv;
}
#endif
#if 0
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
#endif
UString wrap_string(const UString & ustr) {
    using Iter = UString::const_iterator;
    UString t;
    t.reserve(ustr.size() + (ustr.size() % k_wrap_limit) + 3);
    cul::wrap_string_as_monowidth(ustr.begin(), ustr.end(), k_wrap_limit,
        [&t](Iter beg, Iter end)
    {
        t.insert(t.end(), beg, end);
        t += U"\n";
    });
    return t;
}


template <ScenarioStrFunc get_str>
MaxTextSizer net_up_string_maximums(TextArea & target) {
    MaxTextSizer rv;
    rv.target = &target;
    for (auto & ptr : Scenario::get_all_scenarios()) {
        rv.strings.emplace_back(to_ustring(std::invoke(get_str, &*ptr)));
    }
    return rv;
}

void set_to_maximum(MaxTextSizer & mts) {
    assert(mts.target);
    auto & target = *mts.target;
    UiSize max_size_;
    const UString * s = nullptr;
    for (auto & ustr : mts.strings) {
        target.set_string(ustr);
        auto sz = target.size();
        if (sz.width > max_size_.width) {
            s = &ustr;
        }
        max_size_.width  = std::max(max_size_.width , sz.width );
        max_size_.height = std::max(max_size_.height, sz.height);
    }
    assert(s);
    target.set_viewport(asgl::Rectangle(VectorI(), max_size_));
    target.set_string(*s);
}

void revert_to_front(MaxTextSizer & mts) {
    assert(mts.target);
    mts.target->set_string(mts.strings.front());
}

} // end of <anonymous> namespace
