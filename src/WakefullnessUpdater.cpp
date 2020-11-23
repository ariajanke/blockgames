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

#include "WakefullnessUpdater.hpp"

#include <SFML/Window/Event.hpp>

#include <X11/X.h>
#include <X11/Xlib.h>

/* static */ WakefullnessUpdater & WakefullnessUpdater::instance() {
    static WakefullnessUpdater inst;
    return inst;
}

void WakefullnessUpdater::update(double et) {
    m_time_since_signal     += et;
    m_time_since_wake_event += et;
    if (   m_time_since_signal     >  k_delay_until_reawake
        && m_time_since_wake_event <= m_time_since_signal  )
    {
        send_signal(m_impl.get());
        m_time_since_signal = 0.;
    }
}

void WakefullnessUpdater::check_for_waking_events(const sf::Event & event) {
    if (!is_waking_event(event)) return;
    m_time_since_wake_event = 0.;
}

/* private static */ bool WakefullnessUpdater::is_waking_event(const sf::Event & event) {
    switch (event.type) {
    case sf::Event::KeyPressed: case sf::Event::KeyReleased:
    case sf::Event::MouseButtonPressed: case sf::Event::MouseButtonReleased:
    case sf::Event::MouseMoved:
    case sf::Event::JoystickButtonPressed: case sf::Event::JoystickButtonReleased:
    case sf::Event::JoystickMoved:
        return true;
    default: return false;
    }
}

/* private */ WakefullnessUpdater::WakefullnessUpdater() {}

// ---------------------------- X11 implementation ----------------------------

struct X11Back {
    Display * display = XOpenDisplay(nullptr);
};

/* private static */ void WakefullnessUpdater::send_signal(void * ptr) {
    X11Back & impl = *reinterpret_cast<X11Back *>(ptr);
    XResetScreenSaver(impl.display);
    int dummy = 0;
    // this call seems to be needed... I guess as long as it works
    XGetScreenSaver(impl.display, &dummy, &dummy, &dummy, &dummy);
    XForceScreenSaver(impl.display, ScreenSaverReset);
}

/* private static */ void * WakefullnessUpdater::create_impl() {
    return new X11Back();
}

/* private static */ void WakefullnessUpdater::delete_impl(void * ptr) {
    auto * impl = reinterpret_cast<X11Back *>(ptr);
    XCloseDisplay(impl->display);
    delete impl;
}
