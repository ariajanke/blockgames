#pragma once

#include "Defs.hpp"
#include "AppState.hpp"
#include "DialogState.hpp"
#include "Settings.hpp"

#include <ksg/Frame.hpp>

class Dialog : public ksg::Frame {
public:
    using StyleMapPtr = std::shared_ptr<ksg::StyleMap>;
    std::unique_ptr<AppState> get_next_app_state();

    void setup(Settings &);

    void set_styles_ptr(StyleMapPtr);

    static DialogPtr make_top_level_dialog();

protected:
    void set_next_state(DialogPtr);
    void set_next_state(std::unique_ptr<AppState>);

    Settings & settings();
    const Settings & settings() const;

    const ksg::StyleMap & get_styles() const;

    virtual void setup_() = 0;

private:
    std::unique_ptr<AppState> m_next_state;
    Settings * m_settings = nullptr;
    StyleMapPtr m_styles_ptr = nullptr;
};

template <typename T, typename ... Args>
DialogPtr make_dialog(Args ... args) {
    static_assert(std::is_base_of_v<Dialog, T>, "T must be derived from Dialog.");
    return DialogPtr(new T(std::forward<Args>(args)...));
}
