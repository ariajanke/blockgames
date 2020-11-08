#include "Dialog.hpp"

#include "AppState.hpp"
#include "BoardStates.hpp"
#include "PuyoState.hpp"
#include "PuyoDialogs.hpp"
#include "TetrisDialogs.hpp"

#include <ksg/TextArea.hpp>
#include <ksg/TextButton.hpp>

#include <cassert>

namespace {

using BoardOptions = BoardConfigDialog::BoardOptions;

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
    GameSelectionDialog(GameSelection);
private:
    void setup_() override;
    void on_game_selection_update();
#   if 0
    ksg::TextArea m_sel_notice;
#   endif
    ksg::TextArea m_desc_notice;

    ksg::TextButton m_freeplay;
    ksg::TextButton m_scenario;
    ksg::TextButton m_settings;

    ksg::OptionsSlider m_game_slider;

    FrameStretcher m_stretcher;

    ksg::TextButton m_exit;

    GameSelection m_starting_sel;
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

/* static */ DialogPtr Dialog::make_top_level_dialog(GameSelection gamesel)
    { return make_dialog<GameSelectionDialog>(gamesel); }

// ----------------------------------------------------------------------------

void BoardConfigDialog::setup() {
    m_board_config_notice.set_string(U"Configure board width, height,\nand maximum number of colors.");

    m_width_label.set_string(U"Width");
    m_height_label.set_string(U"Height");
    m_num_of_colors_label.set_string(U"Max Colors");

    m_width_sel.set_options(number_range_to_strings(k_min_board_size, k_max_board_size));
    m_height_sel.set_options(number_range_to_strings(k_min_board_size, k_max_board_size));
    m_number_of_colors_sel.set_options(number_range_to_strings(k_min_colors, k_max_colors));

    m_width_sel.select_option(std::size_t(board_options().width - k_min_board_size));
    m_width_sel.set_option_change_event([this]() {
        board_options().width = k_min_board_size + int(m_width_sel.selected_option_index());
    });
    m_height_sel.select_option(std::size_t(board_options().height - k_min_board_size));
    m_height_sel.set_option_change_event([this]() {
        board_options().height = k_min_board_size + int(m_height_sel.selected_option_index());
    });
    m_number_of_colors_sel.select_option(std::size_t(board_options().colors - k_min_colors));
    m_number_of_colors_sel.set_option_change_event([this]() {
        board_options().colors = k_min_colors + int(m_number_of_colors_sel.selected_option_index());
    });

    set_frame_border_size(0.f);

    begin_adding_widgets().
        add(m_board_config_notice).add_line_seperator().
        add(m_width_label).add_horizontal_spacer().add(m_width_sel).add_line_seperator().
        add(m_height_label).add_horizontal_spacer().add(m_height_sel).add_line_seperator().
        add(m_num_of_colors_label).add_horizontal_spacer().add(m_number_of_colors_sel).add_line_seperator();
}

void BoardConfigDialog::assign_board_options(BoardOptions & opts)
    { m_board_options = &opts; }

/* private */ BoardOptions & BoardConfigDialog::board_options() {
    assert(m_board_options);
    return *m_board_options;
}

// ----------------------------------------------------------------------------

/* private */ void SameGameDialog::setup_() {
    set_title(U"Same Game Settings");
    set_drag_enabled(false);

    m_about_single_block_popping.set_string(
        U"Enabling single block popping will cause the game to not\n"
         "end when only single blocks remain. The downside is that\n"
         "this comes with a score penalty");
    m_single_block_pop_notice.set_string(U"Enable/Disable single block popping:");
    update_button_string();
    m_back.set_string(U"Back to Menu");

    m_single_block_pop.set_press_event([this]() {
        auto & b = settings().samegame.gameover_on_singles;
        b = !b;
        update_button_string();
    });

    m_board_config.assign_board_options(settings().samegame);
    m_board_config.setup();

    m_back.set_press_event([this]() {
        set_next_state(Dialog::make_top_level_dialog(GameSelection::samegame_clone));
    });

    begin_adding_widgets(get_styles()).
        add(m_about_single_block_popping).add_line_seperator().
        add(m_single_block_pop_notice).add_horizontal_spacer().add(m_single_block_pop).add_horizontal_spacer().add_line_seperator().
        add_horizontal_spacer().add(m_board_config).add_horizontal_spacer().add_line_seperator().
        add(m_back);
}

/* private */ void SameGameDialog::update_button_string() {
    m_single_block_pop.set_string(settings().samegame.gameover_on_singles ? U"Disabled" : U"Enabled");
}

std::vector<UString> number_range_to_strings(int min, int max) {
    assert(min <= max);
    std::vector<UString> rv;
    for (int i = min; i != max + 1; ++i) {
        rv.push_back(to_ustring(std::to_string(i)));
    }
    return rv;
}

namespace {

GameSelectionDialog::GameSelectionDialog(GameSelection sel):
    m_starting_sel(sel)
{}

void GameSelectionDialog::setup_() {
    using Game = GameSelection;
    set_title(U"Game Selection");
    set_drag_enabled(false);

    m_freeplay.set_string(U"Free Play");
    m_scenario.set_string(U"Scenarios");
    m_settings.set_string(U"Settings" );
    m_exit.set_string(U"Exit Appplication");
    {
    std::vector<UString> gamenames;
    for (auto game : k_game_list) {
        gamenames.push_back(to_ustring(game_name(game)));
    }
    m_game_slider.set_options(std::move(gamenames));
    }
    if (m_starting_sel != GameSelection::count) {
        m_game_slider.select_option(static_cast<std::size_t>(m_starting_sel));
    }
    m_game_slider.set_option_change_event([this]() {
        on_game_selection_update();
    });
    on_game_selection_update();

    m_freeplay.set_press_event([this]() {
        set_next_state([this]() -> std::unique_ptr<AppState> {
            switch (to_game_selection(m_game_slider.selected_option_index())) {
            case Game::puyo_clone    : return std::make_unique<PuyoState >();
            case Game::samegame_clone: return std::make_unique<SameGame   >();
            case Game::tetris_clone  : return std::make_unique<TetrisState>();
            default: throw std::runtime_error("Options slider on invalid game.");
            }
        }());
    });

    m_scenario.set_press_event([this]() {
        auto sel = to_game_selection(m_game_slider.selected_option_index());
        if (sel == Game::puyo_clone) {
            set_next_state(make_dialog<PuyoScenarioDialog>());
        }
    });

    m_settings.set_press_event([this]() {        
        switch (to_game_selection(m_game_slider.selected_option_index())) {
        case Game::puyo_clone  : set_next_state(make_dialog<PuyoDialog>()); break;
        case Game::tetris_clone: set_next_state(make_dialog<PolyominoSelectDialog>()); break;
        case Game::samegame_clone: set_next_state(make_dialog<SameGameDialog>()); break;
        default: break;
        }
    });

    m_exit.set_press_event([this]()
        { set_next_state(std::make_unique<QuitState>()); });

    begin_adding_widgets(get_styles()).
        add_horizontal_spacer().add(m_game_slider).add_horizontal_spacer().add_line_seperator().
        add(m_desc_notice).add_line_seperator().
        add_horizontal_spacer().add(m_freeplay).add_horizontal_spacer().
            add(m_scenario).add_horizontal_spacer().add(m_settings).add_horizontal_spacer().add_line_seperator().
        add(m_stretcher).add_line_seperator().
        add_horizontal_spacer().add(m_exit).add_horizontal_spacer();

}

void GameSelectionDialog::on_game_selection_update() {
    using Game = GameSelection;
    switch (to_game_selection(m_game_slider.selected_option_index())) {
    case Game::puyo_clone:
        m_scenario.set_visible(true);
        break;
    default:
        m_scenario.set_visible(false);
        break;
    }
}

} // end of <anonymous> namespace
