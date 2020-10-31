#pragma once

#include "Defs.hpp"
#include "Dialog.hpp"
#include "BoardStates.hpp"

#include <ksg/TextArea.hpp>
#include <ksg/OptionsSlider.hpp>
#include <ksg/TextButton.hpp>
#include <ksg/EditableText.hpp>

// this applies to samegame, and columns too, and maybe more
class BoardConfigDialog final : public ksg::Frame {
public:
    using BoardOptions = Settings::Board;
#   if 0
    explicit BoardConfigDialog(GameSelection);

    explicit BoardConfigDialog(BoardOptions &);
#   endif
    void setup();
    void assign_board_options(BoardOptions &);
private:
#   if 0
    void setup_() override;
#   endif

    BoardOptions & board_options();
    ksg::TextArea m_board_config_notice;

    ksg::TextArea m_width_label;
    ksg::TextArea m_height_label;
    ksg::TextArea m_num_of_colors_label;

    ksg::OptionsSlider m_width_sel;
    ksg::OptionsSlider m_height_sel;
    ksg::OptionsSlider m_number_of_colors_sel;

    BoardOptions * m_board_options = nullptr;

#   if 0
    ksg::TextButton m_back;

    GameSelection m_selection = GameSelection::count;
#   endif
};

class PuyoDialog final : public Dialog {
    void setup_() override;
    ksg::TextArea m_pop_req_notice;
    ksg::TextArea m_fall_speed_notice;

    ksg::EditableText m_fall_speed_text;
    ksg::OptionsSlider m_pop_req_slider;

    ksg::TextButton m_back;

    BoardConfigDialog m_board_config;
};
