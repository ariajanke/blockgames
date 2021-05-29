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

#include "AppState.hpp"
#include "Settings.hpp"

#include <cassert>

/* static */ UpStackCom & UpStackCom::null_instance() {
    class NullInstance final : public UpStackCom {
        void listen_for_play_control_assignment(PlayControlId) final {}
        void cancel_play_control_assignment() final {}
    };
    static NullInstance inst;
    return inst;
}

// ----------------------------------------------------------------------------

void AppState::setup(const asgl::StyleMap & stylemap) {
    setup_(ensure_settings(), stylemap);
}

void AppState::assign_upstack_com(UpStackCom & upstackcom) {
    (void)ensure_settings();
    m_shared_info->upstack = &upstackcom;
}

sf::View AppState::window_view() const {
    sf::View rv;
    auto w_ = float(width ());
    auto h_ = float(height());
    rv.setSize  (w_     , h_     );
    rv.setCenter(w_*0.5f, h_*0.5f);
    return rv;
}

sf::Vector2u AppState::window_size() const {
    return sf::Vector2u(unsigned(int(window_view().getSize().x)*scale()),
                        unsigned(int(window_view().getSize().y)*scale()));
}

/* protected */ void AppState::listen_for_play_control_assignment
    (PlayControlId playconid)
{
    assert(m_shared_info);
    m_shared_info->upstack->listen_for_play_control_assignment(playconid);
}

/* protected */ void AppState::cancel_play_control_assignment() {
    assert(m_shared_info);
    m_shared_info->upstack->cancel_play_control_assignment();
}

/* private */ Settings & AppState::ensure_settings() {
    if (!m_shared_info) {
        m_shared_info = std::make_shared<SharedInfo>();
        m_shared_info->settings_ptr = std::make_unique<Settings>();
    }
    return *m_shared_info->settings_ptr;
}
