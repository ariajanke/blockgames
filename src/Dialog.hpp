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
#include "AppState.hpp"
#include "DialogState.hpp"
#include "Settings.hpp"

#include <ksg/Frame.hpp>
#include <ksg/TextArea.hpp>
#include <ksg/OptionsSlider.hpp>
#include <ksg/TextButton.hpp>

class Dialog : public ksg::Frame {
public:
    using StyleMapPtr = std::shared_ptr<ksg::StyleMap>;
    std::unique_ptr<AppState> get_next_app_state();

    void setup(Settings &);

    void set_styles_ptr(StyleMapPtr);

    static DialogPtr make_top_level_dialog(GameSelection);

protected:
    void set_next_state(DialogPtr);
    void set_next_state(std::unique_ptr<AppState>);

    Settings & settings();
    const Settings & settings() const;

    const ksg::StyleMap & get_styles() const;

    virtual void setup_() = 0;

private:
    std::unique_ptr<AppState> m_next_state;
    Settings * m_settings = nullptr;
    StyleMapPtr m_styles_ptr = nullptr;
};

// ----------------------------------------------------------------------------

class BoardConfigDialog final : public ksg::Frame {
public:
    using BoardOptions = Settings::Board;

    void setup();
    void assign_board_options(BoardOptions &);

private:
    BoardOptions & board_options();
    ksg::TextArea m_board_config_notice;

    ksg::TextArea m_width_label;
    ksg::TextArea m_height_label;
    ksg::TextArea m_num_of_colors_label;

    ksg::OptionsSlider m_width_sel;
    ksg::OptionsSlider m_height_sel;
    ksg::OptionsSlider m_number_of_colors_sel;

    BoardOptions * m_board_options = nullptr;
};

class SameGameDialog final : public Dialog {
    void setup_() override;
    void update_button_string();

    ksg::TextArea m_about_single_block_popping;
    ksg::TextArea m_single_block_pop_notice;
    ksg::TextButton m_single_block_pop;

    ksg::TextButton m_back;

    BoardConfigDialog m_board_config;
};

std::vector<UString> number_range_to_strings(int min, int max);

template <typename T, typename ... Args>
DialogPtr make_dialog(Args ... args) {
    static_assert(std::is_base_of_v<Dialog, T>, "T must be derived from Dialog.");
    return DialogPtr(new T(std::forward<Args>(args)...));
}
