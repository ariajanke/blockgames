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

#include "Defs.hpp"
#include "Dialog.hpp"
#include "BoardStates.hpp"
#include "PuyoState.hpp"
#if 0
#include <ksg/TextArea.hpp>
#include <ksg/OptionsSlider.hpp>
#include <ksg/TextButton.hpp>
#include <ksg/EditableText.hpp>
#endif

#include <asgl/EditableText.hpp>

using asgl::EditableText;

class PuyoSettingsDialog final : public Dialog {
public:
#   if 0
    explicit PuyoSettingsDialog(int scenario_index):
        m_scenario_index(scenario_index)
    {}
#   endif
    explicit PuyoSettingsDialog(const char * name_ptr):
        m_name_ptr(name_ptr)
    {}

private:
    using PuyoSettings = Settings::WritablePuyo;

    void setup_() override;

    PuyoSettings puyo_settings();

    TextArea m_pop_req_notice;
    TextArea m_fall_speed_notice;

    EditableText m_fall_speed_text;
    OptionsSlider m_pop_req_slider;

    TextButton m_back;

    BoardConfigDialog m_board_config;
    int m_scenario_index;
    const char * m_name_ptr = nullptr;
};

struct MaxTextSizer {
    // always reverts to the first string (in front)
    std::vector<UString> strings;
    TextArea * target = nullptr;
};

class PuyoScenarioDialog final : public Dialog {
    // there are two types of scenarios
    // - sequential (unlockables)
    // - free play scenarios
    void setup_() override;

    void stylize(const asgl::StyleMap &) override;

    void process_event(const asgl::Event &) override;

    void flip_to_scenario();

    const Scenario & get_selected_scenario() const;

    TextArea m_scen_select_notice;

    TextArea m_name_notice;
    TextArea m_name;
    TextArea m_desc_notice;
    TextArea m_desc;

    TextButton m_play;
    TextButton m_settings;

    OptionsSlider m_scenario_slider;
    TextButton m_back;

    std::vector<MaxTextSizer> m_max_sizers;
};
