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

#include <cstdint>
#if 1
#include <SFML/Window/Event.hpp>
#endif
#include <vector>
#include <unordered_set>
#include <variant>

#include "Defs.hpp"

#if 0
struct PlayControlEventReceiver {
    virtual ~PlayControlEventReceiver();
    virtual void handle_event(PlayControlEvent) = 0;
};
#endif
// ----------------------------------------------------------------------------

struct FallingPieceBase {
    virtual ~FallingPieceBase();

    virtual void rotate_left(const BlockGrid &) = 0;
    virtual void rotate_right(const BlockGrid &) = 0;
    virtual void move_right(const BlockGrid &) = 0;
    virtual void move_left(const BlockGrid &) = 0;
};

// ---------------------------- SFML relevant stuff ---------------------------

struct JoystickEntry {
    JoystickEntry() {}
    explicit JoystickEntry(sf::Joystick::Axis axis_): axis(axis_) {}

    JoystickEntry(sf::Joystick::Axis axis_, PlayControlId neg_, PlayControlId pos_):
        axis(axis_), neg(neg_), pos(pos_) {}

    sf::Joystick::Axis axis;
    PlayControlId neg = PlayControlId::count, pos = PlayControlId::count;
};

struct ButtonEntry {
    static constexpr const int k_no_button = -1;
    ButtonEntry() {}
    explicit ButtonEntry(/* from sfml */ unsigned button_,
                         PlayControlId id_ = PlayControlId::count):
        button(int(button_)), id(id_)
    {}
    int button = k_no_button;
    PlayControlId id = PlayControlId::count;
};

struct KeyEntry {
    KeyEntry() {}
    explicit KeyEntry(sf::Keyboard::Key key_): key(key_) {}
    KeyEntry(sf::Keyboard::Key key_, PlayControlId id_): key(key_), id(id_) {}

    sf::Keyboard::Key key;
    PlayControlId id = PlayControlId::count;
};

struct UnmappedEntry {};

using SfEventEntry = std::variant<JoystickEntry, ButtonEntry, KeyEntry, UnmappedEntry>;

void write_assignment_to(const SfEventEntry &, SfEventEntry &);

template <typename T, typename ... Types>
const T * get_alternative(const std::variant<Types...> & var) {
    if (std::holds_alternative<T>(var)) return &std::get<T>(var);
    return nullptr;
}

template <typename T, typename Func, typename ... Types>
bool handles_alternatives
    (const std::variant<Types...> & lhs, const std::variant<Types...> & rhs,
     Func && f)
{
    if (lhs.index() != rhs.index()) return false;
    if (!std::holds_alternative<T>(lhs)) return false;
    f(std::get<T>(lhs), std::get<T>(rhs));
    return true;
}

struct EntryHasher {
    std::size_t operator () (const SfEventEntry & entry) const noexcept;
private:
    static std::size_t alt_hash(const SfEventEntry & entry) noexcept;
};

struct EntryEqualTo {
    bool operator () (const SfEventEntry & lhs, const SfEventEntry & rhs) const noexcept;
};

class AppState;
#if 0
class PlayControlEventReceiver;
#endif
class PlayControlEventHandler {
public:
    using PlayControlSet = std::unordered_set<SfEventEntry, EntryHasher, EntryEqualTo>;
    using PlayControlArray = std::array<PlayControlState, k_play_control_id_count>;
    using PlayControlSetConstIter = PlayControlSet::const_iterator;

    void update(const sf::Event &);
#   if 0
    // does not send still_released events
    void send_events(PlayControlEventReceiver &);
#   endif
    void send_events(AppState &);

    void set_mappings(const PlayControlSet & playset) {
        m_mappings    = playset;
        m_state_array = make_default_play_control_array();
    }

    void set_mapping(const SfEventEntry & entry) {
        return;
        // skip assignments for now :/
        if (std::holds_alternative<UnmappedEntry>(entry)) {
            throw std::invalid_argument("");
        }
        auto itr = m_mappings.find(entry);
        if (itr == m_mappings.end()) {
            m_mappings.insert(entry);
        } else {
            // overwrites previous entry
            auto val = *itr;
            m_mappings.erase(itr);
            write_assignment_to(entry, val);
            m_mappings.insert(val);
        }
    }

    static PlayControlSet make_default_play_control_set();

private:
    static PlayControlSetConstIter find_by_event(const sf::Event &, const PlayControlSet &);

    static constexpr const float k_axis_activation_thershold = 10.f;

    static PlayControlArray make_default_play_control_array();

    void update_key   (const sf::Event &, PlayControlSetConstIter);
    void update_button(const sf::Event &, PlayControlSetConstIter);
    void update_axis  (const sf::Event &, PlayControlSetConstIter);

    void degrade_states();

    void send_events_(AppState &) const;

    PlayControlArray m_state_array = make_default_play_control_array();
    PlayControlSet   m_mappings    = make_default_play_control_set  ();
};

// this is used for control assignments
SfEventEntry convert_to_entry(const sf::Event &, PlayControlId);
