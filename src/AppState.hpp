#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/View.hpp>

#include <memory>

struct Settings;

namespace sf { class Event; }

class AppState : public sf::Drawable {
public:
    using SettingsPtr = std::unique_ptr<Settings>;
    void setup(SettingsPtr &);

    virtual void update(double et) = 0;
    virtual void process_event(const sf::Event &) = 0;
    virtual double width() const = 0;
    virtual double height() const = 0;
    virtual int scale() const = 0;
    virtual bool is_quiting_application() const { return false; }

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
        m_next_state = std::move(ptr);
        return rv;
    }

    void set_next_state(std::unique_ptr<AppState> && ptr)
        { m_next_state = std::move(ptr); }

    /** @param settings lives in a constant address */
    virtual void setup_(Settings & settings) = 0;

private:
    std::unique_ptr<AppState> m_next_state;
};

class QuitState final : public AppState {
    void setup_(Settings &) override {}
    void update(double) override {}
    void process_event(const sf::Event &) override {}
    double width() const override { return 1.; }
    double height() const override { return 1.; }
    int scale() const override { return 1; }
    bool is_quiting_application() const override { return true; }
    void draw(sf::RenderTarget &, sf::RenderStates) const override {}
};
