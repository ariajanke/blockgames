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
