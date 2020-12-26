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

#include "ControlConfigurationDialog.hpp"

#include "Graphics.hpp"

#include <common/StringUtil.hpp>

#include <cassert>

/* static */ const std::array<PlayControlId, k_play_control_id_count>
    ControlConfigurationDialog::k_control_list = []()
{
    using Pid = PlayControlId;
    return decltype (ControlConfigurationDialog::k_control_list) {
        Pid::left, Pid::right, Pid::down, Pid::up,
        Pid::rotate_left, Pid::rotate_right, Pid::pause
    };
} ();

/* private */ void ControlConfigurationDialog::setup_() {
    for (auto id : k_control_list) {
        setup_for_control(id);
        update_for_control(id);
    }

    UString info_string;
    using UChar = UString::value_type;
    wrap_string_as_monowidth(k_info_string, find_string_end(k_info_string), 60,
                             [&info_string](const UChar * beg, const UChar * end)
    {
        info_string.append(beg, end);
        info_string += U"\n";
    });
    m_info.set_string(info_string);
    m_pop_ani_info.set_string(k_ani_string);
    m_pop_animation.set_texture(load_builtin_block_texture(),
                                texture_rect_for_control(PlayControlId::down, false));
    m_pop_animation.set_size(float(k_block_size*3), float(k_block_size*3));

    {
    static constexpr const float k_height = 56.f;
    for (auto & hstretcher : m_height_stretchers) {
        hstretcher.set_height(k_height);
    }
    }
    {
    auto itr = m_height_stretchers.begin();
    for (int i = 0; i != ControlWidgetGroup::k_widget_count; ++i) {
        m_column_frames[i].set_frame_border_size(0.f);
        auto adder = m_column_frames[i].begin_adding_widgets();
        for (auto & groups : m_control_groups) {
            assert(itr != m_height_stretchers.end());
            adder.add(*itr++).add(*groups.widgets[i]).add_line_seperator();
        }
    }
    assert(itr == m_height_stretchers.end());
    }
    m_exit.set_string(U"Back");
    m_exit.set_press_event([this]() {
        set_next_state(make_dialog<GameSelectionDialog>(m_return_to_sel));
    });

    auto adder = begin_adding_widgets(get_styles());
    adder.add_horizontal_spacer().add(m_info).add_horizontal_spacer().add_line_seperator()
        .add_horizontal_spacer().add(m_pop_ani_info).add(m_pop_animation).add_horizontal_spacer().add_line_seperator();
    for (auto & frame : m_column_frames) {
        adder.add(frame);
    }
    adder.add_line_seperator()
         .add_horizontal_spacer().add(m_exit);
}

/* private */ void ControlConfigurationDialog::update(double et) {
    m_control_handler.send_events(*this);
    if ((m_pop_preview_timer -= et) < 0.) {
        m_pop_preview_timer = k_pop_preview_duration;
        m_pop_preview_pressed = !m_pop_preview_pressed;
        m_pop_animation.reset_texture_rectangle(texture_rect_for_control(PlayControlId::down, m_pop_preview_pressed));
    }
}

/* private */ void ControlConfigurationDialog::process_event(const sf::Event & event) {
    Dialog::process_event(event);
    if (m_currently_assigning == PlayControlId::count) {
        m_control_handler.update(event);
        return;
    }
    auto update_controls = [this]() {
        for (auto id : k_control_list) {
            update_for_control(id);
        }
        if (m_currently_assigning == PlayControlId::count) {
            for (auto & group : m_control_groups) {
                auto & btn = group.reassign_btn;
                btn.set_visible(true);
                btn.set_string(k_reassign_string);
            }
            m_control_handler.set_mappings(make_control_set());
        }
    };
    if (event.type == sf::Event::KeyReleased) {
        if (event.key.code == sf::Keyboard::Escape) {
            m_currently_assigning = PlayControlId::count;
            update_controls();
            return;
        }
    }

    auto eventry = convert_to_entry(event, m_currently_assigning);
    if (std::get_if<UnmappedEntry>(&eventry)) return;
    auto & entry = m_control_mapping[static_cast<std::size_t>(m_currently_assigning)];
    if (std::get_if<KeyEntry>(&eventry)) {
        entry.keyboard = eventry;
    } else if (std::get_if<ButtonEntry>(&eventry)) {
        entry.gamepad = eventry;
    } else if (std::get_if<JoystickEntry>(&eventry)) {
        entry.gamepad = eventry;
    }
    m_currently_assigning = PlayControlId::count;
    update_controls();
}

/* private */ void ControlConfigurationDialog::handle_event(PlayControlEvent pce) {
    auto handle_flip = [this](PlayControlId id, bool is_pressed) {
        assert(id != PlayControlId::count);
        m_control_groups[ static_cast<std::size_t>(id) ].representation
            .reset_texture_rectangle(texture_rect_for_control(id, is_pressed));
    };
    switch (pce.state) {
    case PlayControlState::just_pressed : handle_flip(pce.id, true ); break;
    case PlayControlState::just_released: handle_flip(pce.id, false); break;
    default: return;
    }
}

/* private */ void ControlConfigurationDialog::setup_for_control(PlayControlId id) {
    auto idx = static_cast<std::size_t>(id);
    auto & group = m_control_groups[idx];
    group.press_rect   = texture_rect_for_control(id, true );
    group.release_rect = texture_rect_for_control(id, false);

    auto & repre = group.representation;
    repre.set_texture(load_builtin_block_texture(), group.release_rect);
    repre.set_size(float(group.release_rect.width )*3.f,
                   float(group.release_rect.height)*3.f);
    {
    auto & btn = group.reassign_btn;
    btn.set_press_event([this, id, &btn]() {
        if (m_currently_assigning == PlayControlId::count) {
            m_currently_assigning = id;
            // adjustments for assignment mode
            // hide the other buttons
            btn.set_string(k_cancel_string);
            hide_all_reasign_but(btn);
        } else if (m_currently_assigning == id) {
            // act as cancel
            m_currently_assigning = PlayControlId::count;
            btn.set_string(k_reassign_string);
            for (auto & group : m_control_groups) {
                group.reassign_btn.set_visible(true);
            }
        }
    });
    btn.set_string(k_reassign_string);
    }
    {
    auto & btn = group.revert_to_default;
    btn.set_string(U"Revert to Default");
    }

}

/* private */ void ControlConfigurationDialog::hide_all_reasign_but(const ksg::TextButton & this_button) {
    for (auto & group : m_control_groups) {
        if (&group.reassign_btn == &this_button) continue;
        group.reassign_btn.set_visible(false);
    }
}

/* private */ void ControlConfigurationDialog::update_for_control(PlayControlId id) {
    static constexpr const auto k_unassigned = U"default";
    auto idx = static_cast<std::size_t>(id);
    auto & group = m_control_groups[idx];
    //auto & assigned = revmap[idx];
    auto & assigned = m_control_mapping[idx];

    if (auto * key = std::get_if<KeyEntry>(&assigned.keyboard)) {
        group.keyboard_assignment.set_string(U"Key " + to_ustring(std::to_string(int(key->key))));
    } else {
        group.keyboard_assignment.set_string(k_unassigned);
    }
    if (std::get_if<JoystickEntry>(&assigned.gamepad)) {
        group.gamepad_assignment.set_string(U"Some Axis");
    } else if (std::get_if<ButtonEntry>(&assigned.gamepad)) {
        group.gamepad_assignment.set_string(U"Some Button");
    } else {
        group.gamepad_assignment.set_string(k_unassigned);
    }

}

/* private */ ControlConfigurationDialog::PlayControlSet ControlConfigurationDialog::make_control_set() const {
    PlayControlSet rv;
    rv.reserve(k_play_control_id_count*2);
    for (const auto & pid_pair : m_control_mapping) {
        if (!std::get_if<UnmappedEntry>(&pid_pair.gamepad)) {
            rv.insert(pid_pair.gamepad);
        }
        if (!std::get_if<UnmappedEntry>(&pid_pair.keyboard)) {
            rv.insert(pid_pair.keyboard);
        }
    }
    return rv;
}
