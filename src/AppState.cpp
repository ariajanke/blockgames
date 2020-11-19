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

void AppState::setup(SettingsPtr & settings) {
    if (!settings) {
        settings = std::make_unique<Settings>();
    }
    setup_(*settings);
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
