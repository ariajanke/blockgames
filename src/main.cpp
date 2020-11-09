#include "EffectsFull.hpp"
#include "Graphics.hpp"

#include "DialogState.hpp"
#include "Settings.hpp"
// #include "discord.h"
// test edit for wip

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include <common/ParseOptions.hpp>

#include <thread>

#include <cassert>

namespace {

using SettingsPtr = AppState::SettingsPtr;

void do_tests();

inline void assert_(bool b) { assert(b); }

sf::Vector2i center_screen(const AppState &);

class WindowAnchor {
public:
    WindowAnchor(const sf::RenderWindow &, const AppState &);
    void update_position(const sf::RenderWindow &);
    VectorI adjusted_position_for(const AppState &) const;
private:
    VectorI m_anchor;
    //VectorI m_old_pos;
};

struct ProgramOptions {
};

void parse_save_builtin_to_file_system(ProgramOptions &, char ** beg, char ** end);
void save_icon_to_file(ProgramOptions &, char ** beg, char ** end);

} // end of <anonymous> namespace

int main(int argc, char ** argv) {
    parse_options<ProgramOptions>(argc, argv, {
        { "save-builtin", 'b', parse_save_builtin_to_file_system },
        { "save-icon"   , 'i', save_icon_to_file                 }
    });
    FallEffectsFull::do_tests();
    do_tests();

    std::unique_ptr<AppState> app_state = std::make_unique<DialogState>();
    SettingsPtr settings_ptr;
    app_state->setup(settings_ptr);

    sf::RenderWindow win;

    win.create(sf::VideoMode(1, 1), "Block Games", sf::Style::Default);
    win.setPosition(center_screen(*app_state));

    std::this_thread::sleep_for(std::chrono::microseconds(250000));
    win.setSize(sf::Vector2u(app_state->window_size().x, app_state->window_size().y));
    auto sz = win.getPosition();
    win.setPosition(sz);
    auto sz2 = win.getPosition();

    win.setFramerateLimit(60u);
    win.setView(app_state->window_view());
    win.setIcon(unsigned(k_icon_size), unsigned(k_icon_size), get_icon_image());
    sz = win.getPosition();

    WindowAnchor anchor(win, *app_state);
    while (win.isOpen()) {
        {
        sf::Event event;
        while (win.pollEvent(event)) {
            switch (event.type) {
            case sf::Event::Closed: win.close();
                break;
            case sf::Event::KeyReleased:
                break;
            default: break;
            }
            app_state->process_event(event);
        }
        }
        anchor.update_position(win);

        app_state->update(1. / 60.);
        if (auto new_state = app_state->next_state()) {
            new_state.swap(app_state);
            if (app_state->is_quiting_application())
                return 0;
            app_state->setup(settings_ptr);
            win.setSize(app_state->window_size());
#           if 0
            win.setPosition(anchor.adjusted_position_for(*app_state));
#           endif
            win.setView(app_state->window_view());
        }

        win.clear();
        win.draw(*app_state);
        win.display();
        std::this_thread::sleep_for(std::chrono::microseconds(16667));
    }
    return 0;
}

namespace {

void do_tests() {
    assert(GetEdgeValue<>::k_value == TileEdges().to_ulong());
    assert(GetEdgeValue<k_left_edge>::k_value == TileEdges().set(k_left_edge).to_ulong());
    assert(GetEdgeValue<k_right_edge>::k_value == TileEdges().set(k_right_edge).to_ulong());
    assert(GetEdgeValue<k_bottom_edge>::k_value == TileEdges().set(k_bottom_edge).to_ulong());
    assert(GetEdgeValue<k_top_edge>::k_value == TileEdges().set(k_top_edge).to_ulong());
    assert_(GetEdgeValue<k_right_edge, k_left_edge>::k_value == TileEdges().set(k_right_edge).set(k_left_edge).to_ulong());

    assert_(GetEdgeValue<k_right_edge, k_bottom_edge>::k_value == TileEdges().set(k_right_edge).set(k_bottom_edge).to_ulong());
    assert_(GetEdgeValue<k_right_edge, k_top_edge>::k_value == TileEdges().set(k_right_edge).set(k_top_edge).to_ulong());
    assert_(GetEdgeValue<k_left_edge, k_top_edge>::k_value == TileEdges().set(k_left_edge).set(k_top_edge).to_ulong());
    assert_(GetEdgeValue<k_left_edge, k_bottom_edge>::k_value == TileEdges().set(k_left_edge).set(k_bottom_edge).to_ulong());
    assert_(GetEdgeValue<k_top_edge, k_bottom_edge>::k_value == TileEdges().set(k_top_edge).set(k_bottom_edge).to_ulong());


    assert_(GetEdgeValue<k_top_edge, k_bottom_edge, k_left_edge>::k_value == TileEdges().set(k_top_edge).set(k_bottom_edge).set(k_left_edge).to_ulong());
    assert_(GetEdgeValue<k_top_edge, k_bottom_edge, k_right_edge>::k_value == TileEdges().set(k_top_edge).set(k_bottom_edge).set(k_right_edge).to_ulong());
    assert_(GetEdgeValue<k_bottom_edge, k_right_edge, k_left_edge>::k_value == TileEdges().set(k_bottom_edge).set(k_right_edge).set(k_left_edge).to_ulong());
    assert_(GetEdgeValue<k_top_edge, k_right_edge, k_left_edge>::k_value == TileEdges().set(k_top_edge).set(k_right_edge).set(k_left_edge).to_ulong());

    assert(GetEdgeValue<k_left_edge>::k_value == TileEdges().set(k_left_edge).to_ulong());

    {
    auto g = make_grid({
       { 0, 1, 2 },
       { 3, 4, 5 },
       { 6, 7, 8 },
    });
    assert(g(0, 0) == 0);
    assert(g(1, 0) == 1);
    assert(g(2, 0) == 2);

    assert(g(0, 1) == 3);
    assert(g(1, 1) == 4);
    assert(g(2, 1) == 5);

    assert(g(0, 2) == 6);
    assert(g(1, 2) == 7);
    assert(g(2, 2) == 8);
    }
    {
    auto g = make_grid({
        { 0, 1 },
        { 2, 3 },
        { 4, 5 }
    });

    assert(g(0, 0) == 0);
    assert(g(1, 0) == 1);

    assert(g(0, 1) == 2);
    assert(g(1, 1) == 3);

    assert(g(0, 2) == 4);
    assert(g(1, 2) == 5);
    }
    {
    auto g = make_grid({
       { 0, 1, 0 },
       { 1, 1, 1 },
       { 0, 1, 0 }
    });
    Grid<bool> explored;
    explored.set_size(g.width(), g.height());
    std::vector<VectorI> selections = { VectorI(1, 1) };
    select_connected_blocks(g, selections, explored);
    assert(selections.size() == 5);
    }
    {
    auto g = make_grid({
       { 1, 0, 1 },
       { 1, 1, 1 },
       { 1, 0, 1 }
    });
    Grid<bool> explored;
    explored.set_size(g.width(), g.height());
    std::vector<VectorI> selections = { VectorI(0, 0) };
    select_connected_blocks(g, selections, explored);
    assert(selections.size() == 7);
    }
    {
    auto g = make_grid({
       { 0, 0, 2, 0 },
       { 0, 1, 1, 0 },
       { 0, 1, 1, 2 },
       { 0, 0, 0, 0 }
    });
    Grid<bool> explored;
    explored.set_size(g.width(), g.height());
    std::vector<VectorI> selections = { VectorI(1, 1) };
    select_connected_blocks(g, selections, explored);
    assert(selections.size() == 4);
    }
    {
    auto g = make_grid({
        { 0 },
        { 1 },
        { 0 },
        { 1 },
        { 1 },
    });
    make_blocks_fall(g);
    assert(g(0, 0) == 0);
    assert(g(0, 1) == 0);
    assert(g(0, 2) == 1);
    assert(g(0, 3) == 1);
    assert(g(0, 4) == 1);
    }
}

sf::Vector2i center_screen(const AppState & app_state) {
    auto screen_nfo = sf::VideoMode::getDesktopMode();
    return sf::Vector2i(
        (int(screen_nfo.width ) - app_state.window_size().x) / 2,
        (int(screen_nfo.height) - app_state.window_size().y) / 2);

}

WindowAnchor::WindowAnchor
    (const sf::RenderWindow & win, const AppState &)
    //m_anchor(center_screen(app_state))//, m_old_pos(win.getPosition())
{
    //auto screen_nfo = sf::VideoMode::getDesktopMode();
    m_anchor = win.getPosition();//VectorI(int(screen_nfo.width / 2), int(screen_nfo.height / 2));
}

void WindowAnchor::update_position(const sf::RenderWindow & win) {
    return;
    auto new_anchor = win.getPosition();
    auto win_size = win.getSize();
#   if 0
    new_anchor.x += int(win.getSize().x) / 2;
    new_anchor.y += int(win.getSize().y) / 2;
#   endif
    m_anchor = new_anchor;
}

VectorI WindowAnchor::adjusted_position_for(const AppState & app_state) const {
    //return m_anchor;
#   if 1
    return VectorI(m_anchor.x - int(app_state.window_size().x / 2u),
                   m_anchor.y - int(app_state.window_size().y / 2u));
#   endif
}

void parse_save_builtin_to_file_system
    (ProgramOptions &, char ** beg, char ** end)
{
    if (end == beg) return;
    load_builtin_block_texture().copyToImage().saveToFile(*beg);
}

void save_icon_to_file(ProgramOptions &, char ** beg, char ** end) {
    if (end == beg) return;

    sf::Image img;
    img.create(k_icon_size, k_icon_size);
    auto * data     = reinterpret_cast<const sf::Color *>(get_icon_image());
    auto * data_end = data + k_icon_size*k_icon_size;
    VectorI r;
    for (auto itr = data; itr != data_end; ++itr) {
        img.setPixel(unsigned(r.x), unsigned(r.y), *itr);
        ++r.x;
        if (r.x == k_icon_size) {
            ++r.y;
            r.x = 0;
        }
    }
    img.saveToFile(*beg);
}

} // end of <anonymous> namespace
