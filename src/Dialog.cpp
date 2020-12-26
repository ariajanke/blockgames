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

#include "Dialog.hpp"

#include "ControlConfigurationDialog.hpp"
#include "AppState.hpp"
#include "BoardStates.hpp"
#include "PuyoState.hpp"
#include "PuyoDialogs.hpp"
#include "TetrisDialogs.hpp"
#include "ColumnsClone.hpp"

#include "PlayControl.hpp"

#include <ksg/TextArea.hpp>
#include <ksg/TextButton.hpp>
#include <ksg/ImageWidget.hpp>

#include <common/StringUtil.hpp>

#include <iostream>

#include <cassert>

namespace {

using BoardOptions = BoardConfigDialog::BoardOptions;

template <typename ... VarTypes>
class VariantVisitorMaker {
public:
    using VariantType = std::variant<VarTypes...>;

    template <typename ... FuncTypes>
    class Visitor;

private:
    template <typename ... FuncTypes>
    class VisitorBase {
    public:
        template <typename ... FullFuncTypes>
        using VisitFp = void (*)(const Visitor<FullFuncTypes...> &, const VariantType &);

    protected:
        template <typename ... FullFuncTypes>
        constexpr static void add_visit_fp(VisitFp<FullFuncTypes...> *,
                                           VisitFp<FullFuncTypes...> *)
        {}
    };

    template <typename HeadFunc, typename ... FuncTypes>
    class VisitorBase<HeadFunc, FuncTypes...> : public VisitorBase<FuncTypes...>, public HeadFunc {
    public:
        template <typename ... FullFuncTypes>
        using VisitFp = typename VisitorBase<FuncTypes...>::template VisitFp<FullFuncTypes...>;

    protected:
        constexpr VisitorBase(HeadFunc && head, FuncTypes && ... args):
            VisitorBase<FuncTypes...>(std::forward<FuncTypes>(args)...),
            HeadFunc(std::move(head))
        {}

        template <typename ... FullFuncTypes>
        constexpr static void add_visit_fp(VisitFp<FullFuncTypes...> * fp,
                                 VisitFp<FullFuncTypes...> * end_ptr)
        {
#           if 0
            if (fp == end_ptr) {
                throw std::runtime_error("Cannot construct function pointer table.");
            }
#           endif
            *fp = visit<FullFuncTypes...>;
            VisitorBase<FuncTypes...>::add_visit_fp(fp + 1, end_ptr);
        }

    private:
        template <typename ... FullFuncTypes>
        static void visit(const Visitor<FullFuncTypes...> & inst, const VariantType & var_) {
            static constexpr const std::size_t k_idx = (sizeof...(VarTypes) - 1) - sizeof...(FuncTypes);
            const auto & head = static_cast<const HeadFunc &>(inst);
            head( std::get<k_idx>(var_) );
        }
    };

public:
    template <typename ... FuncTypes>
    class Visitor final : public VisitorBase<FuncTypes...> {
    public:
        static_assert(sizeof...(VarTypes) == sizeof...(FuncTypes), "Number of functors must be equal to number of variant types.");
        constexpr explicit Visitor(FuncTypes && ... args):
            VisitorBase<FuncTypes...>(std::forward<FuncTypes>(args)...)
        {
            VisitorBase<FuncTypes...>::add_visit_fp
                (m_fp_array.data(), m_fp_array.data() + m_fp_array.size());
        }

        void operator() (const VariantType & var) const {
            if (var.valueless_by_exception()) return;
            m_fp_array[var.index()](*this, var);
        }

    private:
        using FullFp = void (*)(const Visitor &, const VariantType &);
        std::array<FullFp, sizeof...(FuncTypes)> m_fp_array;
    };

    template <typename ... FuncTypes>
    auto make_visitor(FuncTypes && ... args) const {
        return Visitor<FuncTypes...>(std::forward<FuncTypes>(args)...);
    }
};

template <typename ... VarTypes>
auto from_variant(TypeTag<std::variant<VarTypes...>>) {
    return VariantVisitorMaker<VarTypes...>();
}

int initme = []() {
    using MyVar = std::variant<int, double, char>;
    MyVar a, b, c;
    a = MyVar(std::in_place_type_t<int>(), 1);
    b = MyVar(std::in_place_type_t<double>(), 40.);
    c = MyVar(std::in_place_type_t<char>(), 'A');

    auto visitor = from_variant(TypeTag<MyVar>()).make_visitor(
        [](int i) {
        std::cout << "integer " << i << std::endl;
    }, [](double d) {
        std::cout << "double " << d << std::endl;
    }, [](char c) {
        std::cout << "character " << c << std::endl;
    });
    for (const auto * var : { &a, &b, &c }) {
        visitor(*var);
    }
    return 1;
} ();

const char * game_name(GameSelection sel) {
    using Game = GameSelection;
    switch (sel) {
    case Game::puyo_clone    : return "Puyo Puyo Clone";
    case Game::tetris_clone  : return "Tetris Clone";
    case Game::samegame_clone: return "Same Game Clone";
    case Game::columns_clone : return "Columns Clone";
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

void BoardConfigDialog::assign_size_pointers(int * width_pointer, int * height_pointer) {
    assert(width_pointer != height_pointer);
    assert(width_pointer && height_pointer);
    m_height_ptr = height_pointer;
    m_width_ptr = width_pointer;
}

void BoardConfigDialog::assign_number_of_colors_pointer(int * colors_pointer) {
    assert(colors_pointer);
    m_colors_ptr = colors_pointer;
}

void BoardConfigDialog::setup() {
    m_board_config_notice.set_string(U"Configure board width, height,\nand maximum number of colors.");

    m_width_label.set_string(U"Width");
    m_height_label.set_string(U"Height");
    m_num_of_colors_label.set_string(U"Max Colors");

    m_width_sel.set_options(number_range_to_strings(k_min_board_size, k_max_board_size));
    m_height_sel.set_options(number_range_to_strings(k_min_board_size, k_max_board_size));
    m_number_of_colors_sel.set_options(number_range_to_strings(k_min_colors, k_max_colors));

    if (!m_width_ptr && !m_height_ptr && !m_colors_ptr) {
        throw std::runtime_error("No option pointers have been assigned, this would mean a blank frame.");
    }
    if (m_width_ptr) {
        assert(m_height_ptr);
        m_width_sel.select_option(std::size_t(*m_width_ptr - k_min_board_size));
        m_width_sel.set_option_change_event([this]() {
            *m_width_ptr = k_min_board_size + int(m_width_sel.selected_option_index());
        });
        m_height_sel.select_option(std::size_t(*m_height_ptr - k_min_board_size));
        m_height_sel.set_option_change_event([this]() {
            *m_height_ptr = k_min_board_size + int(m_height_sel.selected_option_index());
        });
    }
    if (m_colors_ptr) {
        m_number_of_colors_sel.select_option(std::size_t(*m_colors_ptr - k_min_colors));
        m_number_of_colors_sel.set_option_change_event([this]() {
            *m_colors_ptr = k_min_colors + int(m_number_of_colors_sel.selected_option_index());
        });
    }

    set_frame_border_size(0.f);

    auto adder = begin_adding_widgets();
    adder.add(m_board_config_notice).add_line_seperator();
    if (m_width_ptr) {
        adder.
            add(m_width_label).add_horizontal_spacer().add(m_width_sel).add_line_seperator().
            add(m_height_label).add_horizontal_spacer().add(m_height_sel).add_line_seperator();
    }
    if (m_colors_ptr) {
        adder.add(m_num_of_colors_label).add_horizontal_spacer().add(m_number_of_colors_sel).add_line_seperator();
    }
}

void BoardConfigDialog::assign_pointers_from_board_options(BoardOptions & options) {
    m_width_ptr  = &options.width ;
    m_height_ptr = &options.height;
    m_colors_ptr = &options.colors;
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
#   if 0
    m_board_config.assign_board_options(settings().samegame);
#   endif
    m_board_config.assign_pointers_from_board_options(settings().samegame);
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
#           if 0
            case Game::puyo_clone    : return std::make_unique<PuyoStateN  >(settings().default_puyo_freeplay_scenario);
#           endif
            case Game::puyo_clone    : return std::make_unique<PuyoStateVS >();
            case Game::samegame_clone: return std::make_unique<SameGame    >();
            case Game::tetris_clone  : return std::make_unique<TetrisState >();
            case Game::columns_clone : return std::make_unique<ColumnsState>();
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
#   if 0
    m_settings.set_press_event([this]() {        
        switch (to_game_selection(m_game_slider.selected_option_index())) {
        case Game::puyo_clone    : set_next_state(make_dialog<PuyoSettingsDialog           >()); break;
        case Game::tetris_clone  : set_next_state(make_dialog<PolyominoSelectDialog>()); break;
        case Game::samegame_clone: set_next_state(make_dialog<SameGameDialog       >()); break;
        case Game::columns_clone : set_next_state(make_dialog<ColumnsSettingsDialog>()); break;
        default: break;
        }
    });
#   endif

    m_configure_controls.set_press_event([this]() {
        auto sel = to_game_selection(m_game_slider.selected_option_index());
        set_next_state(make_dialog<ControlConfigurationDialog>(sel));
    });
    m_configure_controls.set_string(U"Configure Controls");

    m_exit.set_press_event([this]()
        { set_next_state(std::make_unique<QuitState>()); });

    begin_adding_widgets(get_styles()).
        add_horizontal_spacer().add(m_game_slider).add_horizontal_spacer().add_line_seperator().
        add(m_desc_notice).add_line_seperator().
        add_horizontal_spacer().add(m_freeplay).add_horizontal_spacer().
            add(m_scenario).add_horizontal_spacer().add(m_settings).add_horizontal_spacer().add_line_seperator().
        add(m_stretcher).add_line_seperator().
        add(m_configure_controls).add_line_seperator().
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
