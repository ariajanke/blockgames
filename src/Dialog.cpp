#include "Dialog.hpp"

#include "AppState.hpp"
#include "BoardStates.hpp"
#include "PuyoState.hpp"
#include "PuyoDialogs.hpp"

#include <ksg/TextArea.hpp>
#include <ksg/TextButton.hpp>

#include <cassert>

namespace {

class FrameStretcher final : public ksg::Widget {
    static constexpr const float k_min_width = 600.f;

    void process_event(const sf::Event &) override {}

    void set_location(float x, float y) override { m_location = VectorF(x, y); }

    VectorF location() const override { return m_location; }

    float width() const override { return k_min_width; }

    float height() const override { return 0.f; }

    void set_style(const ksg::StyleMap &) override {}

    void draw(sf::RenderTarget &, sf::RenderStates) const override {}

    VectorF m_location;
};

class GameSelectionDialog final : public Dialog {
public:
    void setup_() override;

private:
    ksg::TextArea m_sel_notice;
    ksg::TextArea m_desc_notice;

    ksg::TextButton m_freeplay;
    ksg::TextButton m_scenario;
    ksg::TextButton m_settings;

    ksg::OptionsSlider m_game_slider;

    FrameStretcher m_stretcher;

    ksg::TextButton m_exit;
};

const char * game_name(GameSelection sel) {
    using Game = GameSelection;
    switch (sel) {
    case Game::puyo_clone    : return "Puyo Puyo Clone";
    case Game::tetris_clone  : return "Tetris Clone";
    case Game::samegame_clone: return "Same Game Clone";
    default: throw std::invalid_argument("game_name: selection is not a game");
    }
}

} // end of <anonymous> namespace

std::unique_ptr<AppState> Dialog::get_next_app_state()
    { return std::move(m_next_state); }

/* protected */ void Dialog::set_next_state(DialogPtr new_dialog_ptr) {
    new_dialog_ptr->set_styles_ptr( m_styles_ptr );

    // settings are set by "setup" call on the app state
    auto ds = std::make_unique<DialogState>();
    ds->set_dialog(std::move(new_dialog_ptr));
    m_next_state = std::move(ds);
}

/* protected */ void Dialog::set_next_state(std::unique_ptr<AppState> ptr) {
    // overload *should* capture all dialog states
    if (dynamic_cast<DialogState *>(ptr.get())) {
        throw std::invalid_argument("Dialog::set_next_state: pointer types which derive from DialogState should call the other overload.");
    }
    m_next_state = std::move(ptr);
}

/* protected */ Settings & Dialog::settings() {
    assert(m_settings);
    return *m_settings;
}

/* protected */ const Settings & Dialog::settings() const {
    assert(m_settings);
    return *m_settings;
}

void Dialog::setup(Settings & settings_) {
    m_settings = &settings_;
    if (!m_styles_ptr) {
        auto & styles = *( m_styles_ptr = std::make_shared<ksg::StyleMap>() );
        styles = ksg::styles::construct_system_styles();
        styles[ksg::styles::k_global_font] = ksg::styles::load_font("font.ttf");
    }
    setup_();
}

void Dialog::set_styles_ptr(StyleMapPtr sptr)
    { m_styles_ptr = sptr; }

/* protected */ const ksg::StyleMap & Dialog::get_styles() const {
    if (m_styles_ptr) return *m_styles_ptr;
    throw std::runtime_error("Dialog::get_styles: styles pointer has not been set yet.");
}

/* static */ DialogPtr Dialog::make_top_level_dialog()
    { return make_dialog<GameSelectionDialog>(); }

namespace {

void GameSelectionDialog::setup_() {
#   if 0
    auto styles_ = ksg::styles::construct_system_styles();
    styles_[ksg::styles::k_global_font] = ksg::styles::load_font("font.ttf");
#   endif

    m_sel_notice.set_text(U"Select a game to play.");

    m_freeplay.set_string(U"Free Play");
    m_scenario.set_string(U"Scenarios");
    m_settings.set_string(U"Settings" );
    m_exit.set_string(U"Exit Appplication");
    {
    std::vector<UString> gamenames;
    for (auto game : k_game_list) {
        gamenames.push_back(to_ustring(game_name(game)));
    }
    m_game_slider.swap_options(gamenames);
    }

    m_freeplay.set_press_event([this]() {
        set_next_state([this]() -> std::unique_ptr<AppState> {
            using Game = GameSelection;
            switch (to_game_selection(m_game_slider.selected_option_index())) {
            case Game::puyo_clone    : return std::make_unique<PuyoState >();
            case Game::samegame_clone: return std::make_unique<SameGame   >();
            case Game::tetris_clone  : return std::make_unique<TetrisState>();
            default: throw std::runtime_error("Options slider on invalid game.");
            }
        }());
    });

    m_settings.set_press_event([this]() {
        auto game = to_game_selection(m_game_slider.selected_option_index());
        if (game == GameSelection::puyo_clone) {
            set_next_state(make_dialog<PuyoDialog>());
            return;
        }
#       if 0
        set_next_state(make_dialog<BoardConfigDialog>(game));
#       endif
    });

    m_exit.set_press_event([this]()
        { set_next_state(std::make_unique<QuitState>()); });

    begin_adding_widgets(get_styles()).
        add_horizontal_spacer().add(m_sel_notice).add_horizontal_spacer().add_line_seperator().
        add_horizontal_spacer().add(m_game_slider).add_horizontal_spacer().add_line_seperator().
        add(m_desc_notice).add_line_seperator().
        add_horizontal_spacer().add(m_freeplay).add_horizontal_spacer().
            add(m_scenario).add_horizontal_spacer().add(m_settings).add_horizontal_spacer().add_line_seperator().
        add(m_stretcher).add_line_seperator().
        add_horizontal_spacer().add(m_exit).add_horizontal_spacer();

}

} // end of <anonymous> namespace
