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

void DialogState::set_dialog(DialogPtr dialog)
    { m_dialog = std::move(dialog); }

/* private */ void DialogState::setup_(Settings & settings, const asgl::StyleMap & stylemap) {
    if (!m_dialog) {
        m_dialog = Dialog::make_top_level_dialog(GameSelection::count);
    }
    m_dialog->setup(settings);
    m_dialog->stylize(stylemap);
    m_dialog->check_for_geometry_updates();
}

/* private */ void DialogState::update(double et) {
    m_dialog->update(et);
    m_dialog->check_for_geometry_updates();
}

/* private */ void DialogState::process_ui_event(const asgl::Event & event) {
    if (auto * key = event.as_pointer<asgl::KeyRelease>()) {
        if (key->key == asgl::keys::k_escape) {
            set_next_state(std::make_unique<QuitState>());
        }
    }
#   if 0
    if (event.type == sf::Event::KeyReleased) {
        if (event.key.code == sf::Keyboard::Escape) {
            set_next_state(std::make_unique<QuitState>());
            return;
        }
    }
#   endif
    m_dialog->process_event(event);
    if (auto uptr = m_dialog->get_next_app_state())
        set_next_state(std::move(uptr));
}

/* private */ void DialogState::process_play_event(PlayControlEvent event)
    { m_dialog->process_play_event(event); }

/* private */ double DialogState::width() const
    { return m_dialog->width(); }

/* private */ double DialogState::height() const
    { return m_dialog->height(); }

/* private */ void DialogState::draw(sf::RenderTarget &, sf::RenderStates) const
    {/* target.draw(*m_dialog, states); */}

/* private */ void DialogState::draw_ui_elements(asgl::WidgetRenderer & target) const {
    m_dialog->draw(target);
}
