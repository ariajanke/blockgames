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

#include "PlayControl.hpp"

#include <common/Util.hpp>

#include <cassert>

static PlayControlState update_state(PlayControlState old_state, bool is_press) {
    if (is_pressed(old_state) == is_press) {
        return is_press ? PlayControlState::still_pressed : PlayControlState::still_released;
    }
    return is_press ? PlayControlState::just_pressed : PlayControlState::just_released;
};

PlayControlEventReceiver::~PlayControlEventReceiver() {}

FallingPieceBase::~FallingPieceBase() {}

std::size_t EntryHasher::operator () (const SfEventEntry & entry) const noexcept
    { return (entry.index() << 16) ^ alt_hash(entry); }

/* private static */ std::size_t EntryHasher::alt_hash(const SfEventEntry & entry) noexcept {
    if (auto * gv = get_alternative<JoystickEntry>(entry)) {
        return std::size_t(gv->axis);
    }
    if (auto * gv = get_alternative<ButtonEntry>(entry)) {
        return std::size_t(gv->button);
    }
    if (auto * gv = get_alternative<KeyEntry>(entry)) {
        return static_cast<std::size_t>(gv->key);
    }
    return 0;
}

// ----------------------------------------------------------------------------

bool EntryEqualTo::operator ()
    (const SfEventEntry & lhs, const SfEventEntry & rhs) const noexcept
{
    if (lhs.index() != rhs.index()) return false;

    bool rv = false;
    if (handles_alternatives<JoystickEntry>(lhs, rhs,
        [&rv](const JoystickEntry & lhs, const JoystickEntry & rhs)
        { rv = (lhs.axis == rhs.axis); }))
    { return rv; }
    if (handles_alternatives<ButtonEntry>(lhs, rhs,
        [&rv](const ButtonEntry & lhs, const ButtonEntry & rhs)
        { rv = (lhs.button == rhs.button); }))
    { return rv; }
    if (handles_alternatives<KeyEntry>(lhs, rhs,
        [&rv](const KeyEntry & lhs, const KeyEntry & rhs)
        { rv = (lhs.key == rhs.key); }))
    { return rv; }

    assert(!lhs.valueless_by_exception() && !rhs.valueless_by_exception());
    return true;
}

// ----------------------------------------------------------------------------

void PlayControlEventHandler::update(const sf::Event & event) {
    switch (event.type) {
    case sf::Event::KeyPressed: case sf::Event::KeyReleased:
        return update_key(event);
    case sf::Event::JoystickButtonPressed: case sf::Event::JoystickButtonReleased:
        return update_button(event);
    case sf::Event::JoystickMoved:
        return update_axis(event);
    default: break;
    }
}

void PlayControlEventHandler::send_events(PlayControlEventReceiver & receiver) {
    send_events_(receiver);
    degrade_states();
}

/* static */ PlayControlEventHandler::PlayControlArray PlayControlEventHandler::
    make_default_play_control_array()
{
    PlayControlArray rv;
    std::fill(rv.begin(), rv.end(), PlayControlState::still_released);
    return rv;
}

/* static */ PlayControlEventHandler::PlayControlSet PlayControlEventHandler::
    make_default_play_control_set()
{
    PlayControlSet rv;
    auto add_key = [&rv](sf::Keyboard::Key k, PlayControlId id)
        { rv.insert(SfEventEntry(std::in_place_type_t<KeyEntry>(), KeyEntry(k, id))); };
    add_key(sf::Keyboard::Down  , PlayControlId::down        );
    add_key(sf::Keyboard::Left  , PlayControlId::left        );
    add_key(sf::Keyboard::Right , PlayControlId::right       );
    add_key(sf::Keyboard::A     , PlayControlId::rotate_left );
    add_key(sf::Keyboard::S     , PlayControlId::rotate_right);
    add_key(sf::Keyboard::Return, PlayControlId::pause       );
    add_key(sf::Keyboard::Up    , PlayControlId::up          );

    static auto mk_landr = [](sf::Joystick::Axis axis) {
        JoystickEntry landr;
        landr.axis = axis;
        landr.neg  = PlayControlId::left;
        landr.pos  = PlayControlId::right;
        return SfEventEntry(std::in_place_type_t<JoystickEntry>(), landr);
    };

    static auto mk_uandd = [](sf::Joystick::Axis axis) {
        JoystickEntry uandd;
        uandd.axis = axis;
        uandd.neg  = PlayControlId::down;
        uandd.pos  = PlayControlId::up;
        return SfEventEntry(std::in_place_type_t<JoystickEntry>(), uandd);
    };

    auto landr_list = { sf::Joystick::PovX, sf::Joystick::X };
    auto uandd_list = { sf::Joystick::PovY, sf::Joystick::Y };

    for (auto landr_axis : landr_list) { rv.insert(mk_landr(landr_axis)); }
    for (auto uandd_axis : uandd_list) { rv.insert(mk_uandd(uandd_axis)); }

    auto buttons = { ButtonEntry(0, PlayControlId::rotate_left ),
                     ButtonEntry(1, PlayControlId::rotate_right),
                     ButtonEntry(2, PlayControlId::rotate_left ),
                     ButtonEntry(3, PlayControlId::rotate_right) };
    for (auto button : buttons) {
        rv.insert(SfEventEntry(std::in_place_type_t<ButtonEntry>(), button));
    }

    return rv;
}

/* private */ void PlayControlEventHandler::update_key
    (const sf::Event & event)
{
    assert(   event.type == sf::Event::KeyPressed
           || event.type == sf::Event::KeyReleased);
    auto itr = m_mappings.find(SfEventEntry(std::in_place_type_t<KeyEntry>(), KeyEntry(event.key.code)));
    if (itr == m_mappings.end()) return;
    auto id = std::get<KeyEntry>(*itr).id;
    bool is_press = event.type == sf::Event::KeyPressed;
    auto & state = m_state_array.at(static_cast<int>(id));
    state = update_state(state, is_press);
}

/* private */ void PlayControlEventHandler::update_button
    (const sf::Event & event)
{
    assert(   event.type == sf::Event::JoystickButtonPressed
           || event.type == sf::Event::JoystickButtonReleased);
    auto itr = m_mappings.find(SfEventEntry(std::in_place_type_t<ButtonEntry>(), ButtonEntry(event.joystickButton.button)));
    if (itr == m_mappings.end()) return;
    auto id = std::get<ButtonEntry>(*itr).id;
    bool is_press = event.type == sf::Event::JoystickButtonPressed;
    auto & state = m_state_array.at(static_cast<int>(id));
    state = update_state(state, is_press);
}

/* private */ void PlayControlEventHandler::update_axis
    (const sf::Event & event)
{
    assert(event.type == sf::Event::JoystickMoved);
    auto itr = m_mappings.find(SfEventEntry(
        std::in_place_type_t<JoystickEntry>(), JoystickEntry(event.joystickMove.axis)));
    if (itr == m_mappings.end()) return;

    auto pos_id = std::get<JoystickEntry>(*itr).pos;
    auto neg_id = std::get<JoystickEntry>(*itr).neg;
    auto & pos_state = m_state_array.at(static_cast<int>(pos_id));
    auto & neg_state = m_state_array.at(static_cast<int>(neg_id));

    if (magnitude(event.joystickMove.position) < k_axis_activation_thershold) {
        pos_state = update_state(pos_state, false);
        neg_state = update_state(neg_state, false);
    } else {
        bool is_neg = event.joystickMove.position < 0.f;

        pos_state = update_state(pos_state, !is_neg);
        neg_state = update_state(neg_state,  is_neg);
    }

    // deactivation should be prioritized
    std::array<PlayControlEvent, 2> arr = {
        PlayControlEvent(pos_id, pos_state),
        PlayControlEvent(neg_id, neg_state)
    };
    std::sort(arr.begin(), arr.end(),
              [](const PlayControlEvent & lhs, const PlayControlEvent & rhs)
    {
        // lhs < rhs
        if (is_pressed(lhs) == is_pressed(rhs)) return false;
        if (is_pressed(lhs)) return true;
        return false;
    });
}

/* private */ void PlayControlEventHandler::degrade_states() {
    // degrade states
    for (auto & state : m_state_array) {
        using Pcs = PlayControlState;
        switch (state) {
        case Pcs::just_pressed  : state = Pcs::still_pressed ; break;
        case Pcs::just_released : state = Pcs::still_released; break;
        case Pcs::still_pressed :
        case Pcs::still_released: break;
        default: throw std::runtime_error("PlayControlEventHandler::degrade_states: invalid play control state");
        }
    }
}

/* private */ void PlayControlEventHandler::send_events_
    (PlayControlEventReceiver & receiver) const
{
    for (const auto & state : m_state_array) {
        auto idx = std::size_t(&state - &m_state_array.front());
        assert(idx < static_cast<std::size_t>(PlayControlId::count));
        if (state == PlayControlState::still_released) continue;
        receiver.handle_event(PlayControlEvent(static_cast<PlayControlId>(idx), state));
    }
}
