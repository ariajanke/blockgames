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
        m_dialog = Dialog::make_top_level_dialog();
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
