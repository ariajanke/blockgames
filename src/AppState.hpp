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

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/View.hpp>

#include <asgl/Event.hpp>

#include <memory>

#include "Defs.hpp"

class Settings;

namespace sf { class Event; }
namespace asgl {
    class WidgetRenderer;
    class StyleMap;
}

class UpStackCom {
public:
    virtual ~UpStackCom() {}
    static UpStackCom & null_instance();
    virtual void listen_for_play_control_assignment(PlayControlId) = 0;
    virtual void cancel_play_control_assignment() = 0;
};

class AppState : public sf::Drawable {
public:
    void setup(const asgl::StyleMap &);

    void assign_upstack_com(UpStackCom &);

    virtual void update(double et) = 0;

    // there should be a difference between UI events and play control events

    virtual double width() const = 0;
    virtual double height() const = 0;
    virtual int scale() const = 0;
    virtual bool is_quiting_application() const { return false; }

    virtual void process_play_event(PlayControlEvent) {}

    virtual void process_ui_event(const asgl::Event &) = 0;
    virtual void draw_ui_elements(asgl::WidgetRenderer &) const {}

    sf::View window_view() const;

    sf::Vector2u window_size() const;

    std::unique_ptr<AppState> next_state()
        { return std::move(m_next_state); }

protected:
    AppState() {}

    template <typename T>
    T & make_next_state() {
        static_assert(std::is_base_of_v<AppState, T>, "");
        auto ptr = std::make_unique<T>();
        T & rv = *ptr;
        set_next_state(std::move(ptr));
        return rv;
    }

    void set_next_state(std::unique_ptr<AppState> && ptr) {
        m_next_state = std::move(ptr);
        m_next_state->m_shared_info = m_shared_info;
    }

    /** @param settings lives in a constant address */
    virtual void setup_(Settings & settings, const asgl::StyleMap &) = 0;

    void listen_for_play_control_assignment(PlayControlId);

    void cancel_play_control_assignment();

private:
    using SettingsPtr = std::unique_ptr<Settings>;
    Settings & ensure_settings();

    struct SharedInfo {
        SettingsPtr settings_ptr;
        UpStackCom * upstack = &UpStackCom::null_instance();
    };

    std::unique_ptr<AppState> m_next_state;
    std::shared_ptr<SharedInfo> m_shared_info;
};

class QuitState final : public AppState {
    void setup_(Settings &, const asgl::StyleMap &) override {}
    void update(double) override {}
    void process_ui_event(const asgl::Event &) override {}
    double width() const override { return 1.; }
    double height() const override { return 1.; }
    int scale() const override { return 1; }
    bool is_quiting_application() const override { return true; }
    void draw(sf::RenderTarget &, sf::RenderStates) const override {}
};
