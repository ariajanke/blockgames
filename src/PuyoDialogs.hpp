#pragma once

#include "Defs.hpp"
#include "Dialog.hpp"
#include "BoardStates.hpp"
#include "PuyoState.hpp"

#include <ksg/TextArea.hpp>
#include <ksg/OptionsSlider.hpp>
#include <ksg/TextButton.hpp>
#include <ksg/EditableText.hpp>

class PuyoDialog final : public Dialog {
    void setup_() override;

    ksg::TextArea m_pop_req_notice;
    ksg::TextArea m_fall_speed_notice;

    ksg::EditableText m_fall_speed_text;
    ksg::OptionsSlider m_pop_req_slider;

    ksg::TextButton m_back;

    BoardConfigDialog m_board_config;
};

class PuyoScenarioDialog final : public Dialog {
    // there are two types of scenarios
    // - sequential (unlockables)
    // - free play scenarios
    void setup_() override;
    void flip_to_scenario();
    const Scenario & get_selected_scenario() const;
#   if 0
    ksg::TextArea m_title;
#   endif
    ksg::TextArea m_scen_select_notice;

    ksg::TextArea m_name_notice;
    ksg::TextArea m_name;
    ksg::TextArea m_desc_notice;
    ksg::TextArea m_desc;

    ksg::TextButton m_play;

    ksg::OptionsSlider m_scenario_slider;
    ksg::TextButton m_back;
};
