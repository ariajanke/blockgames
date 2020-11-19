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
