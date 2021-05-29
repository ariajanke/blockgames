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
#if 0
#include <ksg/Frame.hpp>
#include <ksg/TextArea.hpp>
#include <ksg/OptionsSlider.hpp>
#include <ksg/TextButton.hpp>
#endif

#include <asgl/Frame.hpp>
#include <asgl/TextArea.hpp>
#include <asgl/OptionsSlider.hpp>
#include <asgl/TextButton.hpp>

namespace asgl {

class Frame;
class TextArea;
class OptionsSlider;
class TextButton;
class Widget;
class Button;

} // end of asgl namespace

using UiFrame  = asgl::Frame;
using UiWidget = asgl::Widget;
using UiVector = asgl::Vector;
using UiSize   = asgl::Size;
using UiButton = asgl::Button;
using asgl::TextArea;
using asgl::OptionsSlider;
using asgl::TextButton;

class Dialog : public UiFrame {
public:
#   if 0
    using StyleMapPtr = std::shared_ptr<ksg::StyleMap>;
#   endif
    std::unique_ptr<AppState> get_next_app_state();

    void setup(Settings &);

    void set_styles_ptr(asgl::StyleMap);

    virtual void update(double) {}

    virtual void process_play_event(PlayControlEvent) {}

    static DialogPtr make_top_level_dialog(GameSelection);

protected:
    void set_next_state(DialogPtr);

    void set_next_state(std::unique_ptr<AppState>);

    Settings & settings();

    const Settings & settings() const;
#   if 0
    const asgl::StyleMap & get_styles() const;
#   endif
    virtual void setup_() = 0;

private:
    std::unique_ptr<AppState> m_next_state;
    Settings * m_settings = nullptr;
#   if 0
    asgl::StyleMap m_styles_ptr;
#   endif
};

// ----------------------------------------------------------------------------

class BoardConfigDialog final : public UiFrame {
public:
    using BoardOptions = Settings::Board;

    void assign_size_pointers(int * width_pointer, int * height_pointer);
    void assign_number_of_colors_pointer(int * colors_pointer);

    void setup();
    void assign_pointers_from_board_options(BoardOptions &);

    bool will_be_blank() const noexcept
        { return !m_width_ptr && !m_height_ptr && !m_colors_ptr; }

private:
    TextArea m_board_config_notice;

    TextArea m_width_label;
    TextArea m_height_label;
    TextArea m_num_of_colors_label;

    OptionsSlider m_width_sel;
    OptionsSlider m_height_sel;
    OptionsSlider m_number_of_colors_sel;

    int * m_width_ptr = nullptr;
    int * m_height_ptr = nullptr;
    int * m_colors_ptr = nullptr;
};

class SameGameDialog final : public Dialog {
    void setup_() override;
    void update_button_string();

    TextArea m_about_single_block_popping;
    TextArea m_single_block_pop_notice;
    TextButton m_single_block_pop;

    TextButton m_back;

    BoardConfigDialog m_board_config;
};

class FrameStretcher final : public UiWidget {
    static constexpr const float k_min_width = 600.f;

    void process_event(const asgl::Event &) override {}

    void set_location_(int x, int y) override { m_location = UiVector(x, y); }

    UiVector location() const override { return m_location; }

    UiSize size() const override { return UiSize(k_min_width, 0); }

    void stylize(const asgl::StyleMap &) override {}

    void update_size() final {}

    void draw(asgl::WidgetRenderer &) const final {}

    UiVector m_location;
};

class GameSelectionDialog final : public Dialog {
public:
    GameSelectionDialog(GameSelection);
private:
    void setup_() override;
    void on_game_selection_update();
#   if 0
    ksg::TextArea m_sel_notice;
#   endif
    TextArea m_desc_notice;

    TextButton m_freeplay;
    TextButton m_scenario;
    TextButton m_settings;

    TextButton m_configure_controls;

    OptionsSlider m_game_slider;

    FrameStretcher m_stretcher;

    TextButton m_exit;

    GameSelection m_starting_sel;
};

std::vector<UString> number_range_to_strings(int min, int max);

template <typename T, typename ... Args>
DialogPtr make_dialog(Args ... args) {
    static_assert(std::is_base_of_v<Dialog, T>, "T must be derived from Dialog.");
    return DialogPtr(new T(std::forward<Args>(args)...));
}
