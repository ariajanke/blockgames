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

#include <memory>

namespace sf { class Event; }

class WakefullnessUpdater {
public:
    static WakefullnessUpdater & instance();
    void update(double et);
    void check_for_waking_events(const sf::Event & event);

private:
    static constexpr const double k_delay_until_reawake = 10.;

    static bool is_waking_event(const sf::Event &);

    // <-------------------- platform specific functions --------------------->

    static void send_signal(void *);
    static void * create_impl();
    static void delete_impl(void *);

    struct Del { void operator () (void * p) const { delete_impl(p); } };

    WakefullnessUpdater();

    std::unique_ptr<void, Del> m_impl = std::unique_ptr<void, Del>(create_impl());
    double m_time_since_signal        = 0.;
    double m_time_since_wake_event    = 0.;
};
