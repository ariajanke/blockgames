#pragma once

#include "AppState.hpp"

#include <ksg/StyleMap.hpp>

class Dialog;
class DialogPtrPriv {
    struct Del { void operator () (Dialog *) const; };
public:
    using UPtr = std::unique_ptr<Dialog, Del>;
};

using DialogPtr = DialogPtrPriv::UPtr;

class DialogState final : public AppState {
public:
    void set_dialog(DialogPtr dialog);

private:
    void setup_(Settings &) override;
    void update(double) override {}
    void process_event(const sf::Event & event) override;
    double width() const override;
    double height() const override;
    int scale() const override { return 1; }
    void draw(sf::RenderTarget &, sf::RenderStates) const override;

    DialogPtr m_dialog;
    ksg::StyleMap m_style_map;
};
