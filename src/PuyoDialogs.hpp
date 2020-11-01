#pragma once

#include "Defs.hpp"
#include "Dialog.hpp"
#include "BoardStates.hpp"

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
