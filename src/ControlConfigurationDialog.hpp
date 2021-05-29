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

#pragma once

#include "Dialog.hpp"
#if 0
#include <ksg/ImageWidget.hpp>
#endif
#include <asgl/ImageWidget.hpp>

using asgl::ImageWidget;

class HeightStretcher final : public UiWidget {
public:
    void set_height(float h) { m_height = h; }
private:
    void process_event(const asgl::Event &) override {}

    void set_location_(int x, int y) override { m_location = UiVector(x, y); }

    UiVector location() const override { return m_location; }

    UiSize size() const override { return UiSize(0, m_height); }

    void stylize(const asgl::StyleMap &) override {}

    void draw(asgl::WidgetRenderer &) const override {}

    void update_size() override {}

    float m_height = 1.f;
    UiVector m_location;
};

class ControlConfigurationDialog final : public Dialog/*, public PlayControlEventReceiver*/ {
public:
    ControlConfigurationDialog(GameSelection sel): m_return_to_sel(sel) {}

private:
    using PlayControlSet = PlayControlEventHandler::PlayControlSet;
    using ControlMapping = Settings::ControlMapping;

    struct ControlWidgetGroup {
        static constexpr const int k_widget_count = 5;
        ControlWidgetGroup():
            widgets({ &representation, &keyboard_assignment, &gamepad_assignment, &reassign_btn, &revert_to_default })
        {}
        sf::IntRect press_rect, release_rect;

        ImageWidget representation;
        TextArea keyboard_assignment;
        TextArea gamepad_assignment;
        TextButton reassign_btn;
        TextButton revert_to_default;

        std::array<UiWidget *, k_widget_count> widgets;
    };

    static constexpr const auto k_info_string = U""
        "Configure game controls here. If you see one of the little button "
        "graphics depress in this dialog based on your key/gamepad presses, "
        "that shows which control was assigned to that key/gamepad element.";

    static constexpr const auto k_ani_string = U""
        "The animation looks like this:";

    static constexpr const auto k_reassign_string = U"Reassign";
    static constexpr const auto k_cancel_string = U"Cancel";

    void setup_() override;
    void update(double) override;
    void process_event(const asgl::Event &) override;
    void process_play_event(PlayControlEvent) override;

    void setup_for_control(PlayControlId);

    void hide_all_reasign_but(const TextButton & this_button);

    void update_for_control(PlayControlId);
    PlayControlSet make_control_set() const;

    static const std::array<PlayControlId, k_play_control_id_count> k_control_list;

    TextArea m_info;

    TextArea m_pop_ani_info;
    ImageWidget m_pop_animation;

    std::array<asgl::SimpleFrame, ControlWidgetGroup::k_widget_count> m_column_frames;
    std::array<HeightStretcher, ControlWidgetGroup::k_widget_count*k_play_control_id_count> m_height_stretchers;
    std::array<ControlWidgetGroup, k_play_control_id_count> m_control_groups;

    ControlMapping m_control_mapping;
#   if 0
    PlayControlEventHandler m_control_handler;
#   endif
    TextButton m_exit;
    PlayControlId m_currently_assigning = PlayControlId::count;

    GameSelection m_return_to_sel;

    static constexpr const double k_pop_preview_duration = 0.8;
    double m_pop_preview_timer = k_pop_preview_duration;
    bool m_pop_preview_pressed = false;
};
