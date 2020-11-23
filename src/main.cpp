#include "EffectsFull.hpp"
#include "Graphics.hpp"
#include "WakefullnessUpdater.hpp"
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

#ifdef MACRO_TEST_DRIVER_ENTRY_FUNCTION
    int MACRO_TEST_DRIVER_ENTRY_FUNCTION();
#endif

int main(int argc, char ** argv) {
    parse_options<ProgramOptions>(argc, argv, {
        { "save-builtin", 'b', parse_save_builtin_to_file_system },
        { "save-icon"   , 'i', save_icon_to_file                 }
    });

#   ifdef MACRO_TEST_DRIVER_ENTRY_FUNCTION
    assert(MACRO_TEST_DRIVER_ENTRY_FUNCTION() == 0);
#   endif

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
            WakefullnessUpdater::instance().check_for_waking_events(event);
        }
        }
        anchor.update_position(win);

        app_state->update(1. / 60.);
        WakefullnessUpdater::instance().update(1. / 60.);
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
