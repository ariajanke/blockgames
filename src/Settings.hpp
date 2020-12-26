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

#include "Defs.hpp"
#include "Polyomino.hpp"

#include <map>

constexpr const char * const k_settings_filename = "blockgamesconf.bin";

PolyominoEnabledSet enable_tetromino_only();

// program wide settings for various games
class Settings {
public:
    friend void save_settings(const Settings &) noexcept;

    static constexpr const int    k_unused_i = -1;
    static constexpr const double k_unused_d = 0.;

    struct MappingEntry {
        SfEventEntry keyboard = UnmappedEntry();
        SfEventEntry gamepad  = UnmappedEntry();
    };
    using ControlMapping = std::array<MappingEntry, k_play_control_id_count>;

    Settings();
    Settings(const Settings &) = delete;
    Settings(Settings &&) = delete;
    ~Settings();

    Settings & operator = (const Settings &) = delete;
    Settings & operator = (Settings &&) = delete;

    struct Board {
        Board() {}
        Board(int width_, int height_, int colors_):
            width(width_), height(height_), colors(colors_)
        {}
        int width  = k_min_board_size;
        int height = k_min_board_size;
        int colors = k_min_colors;
    };

    struct Puyo : public Board {
        Puyo();
        int pop_requirement = 4;
        double fall_speed = 1.5;
    };

    class WritablePuyo {
    public:
        explicit WritablePuyo(Puyo & p_): m_puyo(p_) {}
        int * height_ptr()
            { return m_puyo.height == k_unused_i ? nullptr : &m_puyo.height; }

        int * width_ptr()
            { return m_puyo.width == k_unused_i ? nullptr : &m_puyo.width; }

        int * color_count_ptr()
            { return m_puyo.colors == k_unused_i ? nullptr : &m_puyo.colors; }

        int * pop_requirement_ptr()
            { return m_puyo.pop_requirement == k_unused_i ? nullptr : &m_puyo.pop_requirement; }

        double * fall_speed_ptr()
            { return std::equal_to<double>()(m_puyo.fall_speed, k_unused_d) ? nullptr : &m_puyo.fall_speed; }

    private:
        Puyo & m_puyo;
    };

    struct Tetris : public Board {
        Tetris();
        PolyominoEnabledSet enabled_polyominos = enable_tetromino_only();
        double fall_speed = 1.5;
    };

    struct SameGame : public Board {
        SameGame();
        bool gameover_on_singles = false;
    };
#   if 0
    Puyo & get_puyo_scenario(int);

    const Puyo & get_puyo_scenario(int) const;
#   endif
    /** @param name_ptr Settings are not saught based on the string contents
     *                  but rather by their address. One of the scenario
     *                  guarantees is that each name has a unique address.
     */
    const Puyo & get_puyo_settings(const char * name_ptr) const;

    WritablePuyo get_puyo_settings(const char * name_ptr);

#   if 0
    int puyo_scenario_count() const;
#   endif
    int default_puyo_freeplay_scenario = 0;

    Tetris tetris;
    SameGame samegame;

private:
#   if 0
    std::vector<Puyo> m_puyo_scenarios;
#   endif
    std::map<const char *, Puyo> m_puyo_settings;
};
#if 0
template <bool k_is_const_t>
class PuyoSettingsView {
public:
    using ParentRef = std::conditional_t<k_is_const_t, const Settings &, Settings &>;
    using Iter = std::conditional_t<k_is_const_t, const Settings::Puyo *, Settings::Puyo *>;

    explicit PuyoSettingsView(ParentRef parent) {
        m_beg = &parent.get_puyo_scenario(0);
        m_end = m_beg + parent.puyo_scenario_count();
    }

    PuyoSettingsView(Iter beg_, Iter end_): m_beg(beg_), m_end(end_) {}

    Iter begin() { return m_beg; }
    Iter end  () { return m_end; }
private:
    Iter m_beg, m_end;
};
#if 0
template <bool k_is_const_t>
PuyoSettingsView<k_is_const_t> make_puyo_settings_view
    (typename PuyoSettingsView<k_is_const_t>::ParentRef settings)
{ return PuyoSettingsView<k_is_const_t>(settings); }
#endif
#if 1
inline PuyoSettingsView<true> make_puyo_settings_view(const Settings & settings)
    { return PuyoSettingsView<true>(settings); }

inline PuyoSettingsView<false> make_puyo_settings_view(Settings & settings)
    { return PuyoSettingsView<false>(settings); }
#endif
#endif
void save_settings(const Settings &) noexcept;
