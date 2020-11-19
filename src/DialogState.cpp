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

#include "DialogState.hpp"

#include "Dialog.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

void DialogPtrPriv::Del::operator () (Dialog * ptr) const { delete ptr; }

// ----------------------------------------------------------------------------

void DialogState::set_dialog(DialogPtr dialog) {
    m_dialog = std::move(dialog);
}

/* private */ void DialogState::setup_(Settings & settings) {
    if (!m_dialog) {
        m_dialog = Dialog::make_top_level_dialog(GameSelection::count);
    }
    m_dialog->setup(settings);
}

/* private */ void DialogState::process_event(const sf::Event & event) {
    if (event.type == sf::Event::KeyReleased) {
        if (event.key.code == sf::Keyboard::Escape) {
            set_next_state(std::make_unique<QuitState>());
            return;
        }
    }
    m_dialog->process_event(event);
    set_next_state(m_dialog->get_next_app_state());
}

/* private */ double DialogState::width() const
    { return m_dialog->width(); }

/* private */ double DialogState::height() const
    { return m_dialog->height(); }

/* private */ void DialogState::draw(sf::RenderTarget & target, sf::RenderStates states) const {
    target.draw(*m_dialog, states);
}
